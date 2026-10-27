import logging
from datetime import datetime, timezone
from logging.handlers import RotatingFileHandler
from unittest.mock import Mock

import pandas as pd
import psycopg2
import pytest
from fastapi.testclient import TestClient

from ContimeterAPI.app.dependencies import get_data_service
from ContimeterAPI.app.logging_config import setup_logging
from ContimeterAPI.app.main import app


@pytest.fixture
def api():
    service = Mock()
    app.dependency_overrides[get_data_service] = lambda: service
    with TestClient(app) as client:
        yield client, service
    app.dependency_overrides.clear()


def query(**overrides):
    parameters = {
        "from_datetime": "2026-06-15T12:00:00Z",
        "to_datetime": "2026-06-15T13:00:00Z",
        "thingid": "device-1",
        "variables": ["temperature", "humidity"],
    }
    parameters.update(overrides)
    return parameters


def test_data_serializes_measurements_and_missing_values(api):
    client, service = api
    created = datetime(2026, 6, 15, 12, tzinfo=timezone.utc)
    service.get_data.return_value = pd.DataFrame([
        {"created": created, "thingid": "device-1", "temperature": 21.5, "humidity": 48},
        {"created": created, "thingid": "device-1", "temperature": float("nan"), "humidity": None},
    ])

    response = client.get("/data", params=query())

    assert response.status_code == 200
    assert response.json() == [
        {"created": "2026-06-15T12:00:00.000Z", "thingid": "device-1", "temperature": 21.5, "humidity": 48},
        {"created": "2026-06-15T12:00:00.000Z", "thingid": "device-1", "temperature": None, "humidity": None},
    ]
    service.get_data.assert_called_once_with(
        created, datetime(2026, 6, 15, 13, tzinfo=timezone.utc),
        "device-1", ["temperature", "humidity"],
    )


def test_data_returns_empty_list(api):
    client, service = api
    service.get_data.return_value = pd.DataFrame(columns=["created", "thingid", "temperature"])

    response = client.get("/data", params=query())

    assert response.status_code == 200
    assert response.json() == []


@pytest.mark.parametrize("overrides", [
    {"from_datetime": "2026-06-16T12:00:00Z"},
    {"from_datetime": "not-a-date"},
    {"from_datetime": "2026-06-15T12:00:00"},
    {"thingid": ""},
    {"variables": []},
    {"variables": [""]},
    {"variables": ["created"]},
    {"variables": ["thingid"]},
    {"variables": ["temperature", "temperature"]},
])
def test_invalid_query_does_not_read_database(api, overrides):
    client, service = api

    response = client.get("/data", params=query(**overrides))

    assert response.status_code == 422
    service.get_data.assert_not_called()


def test_database_failure_does_not_expose_connection_details(api):
    client, service = api
    service.get_data.side_effect = psycopg2.OperationalError("private connection details")

    response = client.get("/data", params=query())

    assert response.status_code == 503
    assert response.json() == {"detail": "Database is unavailable"}


def test_missing_parameters_are_rejected(api):
    client, service = api

    assert client.get("/data").status_code == 422
    service.get_data.assert_not_called()


def test_logging_creates_directory_and_rotates_files_without_duplicate_handlers(tmp_path):
    log_directory = tmp_path / "logs"
    logger = setup_logging(log_directory)
    handler = next(
        handler for handler in logger.handlers
        if isinstance(handler, RotatingFileHandler)
        and handler.baseFilename == str((log_directory / "api.log").resolve())
    )
    try:
        handler_count = len(logger.handlers)
        assert setup_logging(log_directory) is logger
        assert len(logger.handlers) == handler_count
        assert handler.maxBytes == 5 * 1024 * 1024
        assert handler.backupCount == 5
        handler.maxBytes = 200
        for index in range(10):
            logger.info(f"Rotation test record {index} {'payload' * 10}")
        assert (log_directory / "api.log.1").exists()
        assert (log_directory / "api.log.5").exists()
        assert not (log_directory / "api.log.6").exists()
        content = (log_directory / "api.log").read_text(encoding="utf-8")
        assert "INFO [contimeter.api] Rotation test record 9" in content
    finally:
        logger.removeHandler(handler)
        handler.close()


def test_request_logging_includes_status_and_duration_without_query_values(api, caplog):
    client, service = api
    service.get_data.return_value = pd.DataFrame()

    with caplog.at_level(logging.INFO, logger="contimeter.api"):
        response = client.get("/data", params=query())

    assert response.status_code == 200
    assert "GET /data status=200 duration_ms=" in caplog.text
    assert "device-1" not in caplog.text


def test_database_failure_is_logged_without_private_details(api, caplog):
    client, service = api
    service.get_data.side_effect = psycopg2.OperationalError("private connection details")

    with caplog.at_level(logging.INFO, logger="contimeter.api"):
        response = client.get("/data", params=query())

    assert response.status_code == 503
    assert "Database query failed: OperationalError" in caplog.text
    assert "GET /data status=503" in caplog.text
    assert "private connection details" not in caplog.text


def test_unexpected_error_is_logged_and_reraised(api, caplog):
    client, service = api
    service.get_data.side_effect = RuntimeError("private error details")

    with caplog.at_level(logging.INFO, logger="contimeter.api"):
        with pytest.raises(RuntimeError, match="private error details"):
            client.get("/data", params=query())

    assert "GET /data status=500 duration_ms=" in caplog.text
    assert "error=RuntimeError" in caplog.text
    assert "private error details" not in caplog.text