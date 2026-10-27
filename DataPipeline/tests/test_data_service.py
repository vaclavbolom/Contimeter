import json
import os
from datetime import datetime, UTC, timedelta

import pandas as pd
import psycopg2
import pytest

from ContimeterAPI.app.DataAccess import data_service


@pytest.mark.integration
def test_get_data_reads_real_database_rows():
    db_params = {
        "host": os.getenv("DB_HOST", "localhost"),
        "port": int(os.getenv("DB_PORT", "5432")),
        "database": os.getenv("DB_NAME", "sensordata"),
        "user": os.getenv("DB_USER", "postgres"),
        "password": os.getenv("DB_PASSWORD", "docker"),
        "sslmode": os.getenv("DB_SSLMODE", "disable"),
    }

    try:
        connection = psycopg2.connect(**db_params)
    except Exception as exc:  # pragma: no cover - depends on local Postgres availability
        pytest.skip(f"PostgreSQL integration test skipped because no database is available: {exc}")

    try:


        thingid = "shelly1pmg4-98a31676e760"
        end = datetime.now(tz=UTC)
        start = end - timedelta(minutes=10)
        

        service = data_service.DataService(db_params)
        df = service.get_data(start, end, thingid, ["apower", "voltage", "freq"])

        assert not df.empty
        assert list(df.columns) == ["created", "thingid", "apower", "voltage", "freq"]
        assert df["thingid"].iat[0] == thingid
        
    except Exception as ex:
        pytest.fail(f"Unexpected exception during test: {ex}")
    finally:
        connection.close()


def test_get_data_returns_dataframe_for_selected_variables(monkeypatch):
    rows = [
        (
            datetime(2026, 6, 15, 12, 0, 0),
            "device-1",
            {"temperature": 21.5, "humidity": 48},
        ),
        (
            datetime(2026, 6, 15, 12, 5, 0),
            "device-1",
            json.dumps({"temperature": 22.3}),
        ),
        (
            datetime(2026, 6, 15, 12, 10, 0),
            "device-1",
            None,
        ),
    ]

    class FakeCursor:
        def __init__(self):
            self.executed = None

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc, tb):
            return False

        def execute(self, query, params):
            self.executed = (query, params)

        def fetchall(self):
            return rows

    class FakeConnection:
        def __init__(self):
            self.cursor_obj = FakeCursor()

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc, tb):
            return False

        def cursor(self):
            return self.cursor_obj

        def close(self):
            return None

    fake_connection = FakeConnection()

    def fake_connect(**kwargs):
        return fake_connection

    monkeypatch.setattr(data_service.psycopg2, "connect", fake_connect)

    service = data_service.DataService({"host": "localhost", "database": "testdb"})
    start = datetime(2026, 6, 15, 11, 0, 0)
    end = datetime(2026, 6, 15, 12, 15, 0)

    result = service.get_data(start, end, "device-1", ["temperature", "humidity", "pressure"])

    assert list(result.columns) == ["created", "thingid", "temperature", "humidity", "pressure"]
    assert result["temperature"].tolist()[:2] == [21.5, 22.3]
    assert pd.isna(result["temperature"].tolist()[2])
    assert result["humidity"].tolist()[:1] == [48]
    assert pd.isna(result["humidity"].tolist()[1])
    assert pd.isna(result["humidity"].tolist()[2])
    assert pd.isna(result["pressure"].tolist()[0])
    assert pd.isna(result["pressure"].tolist()[1])
    assert pd.isna(result["pressure"].tolist()[2])
    assert result["thingid"].tolist() == ["device-1", "device-1", "device-1"]
    assert fake_connection.cursor_obj.executed[0].strip().startswith("SELECT created, thingid, vals")


def test_get_data_validates_input_ranges_and_variable_types():
    service = data_service.DataService({"host": "localhost"})
    start = datetime(2026, 6, 15, 12, 0, 0)
    end = datetime(2026, 6, 15, 11, 0, 0)

    with pytest.raises(ValueError, match="from_datetime must be less than or equal to to_datetime"):
        service.get_data(start, end, "device-1", ["temperature"])

    with pytest.raises(TypeError, match="variables must contain only strings"):
        service.get_data(end, start, "device-1", ["temperature", 42])


def test_get_data_uses_query_for_requested_thing_and_interval(monkeypatch):
    captured = {}

    class FakeCursor:
        def __init__(self):
            self.executed = None

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc, tb):
            return False

        def execute(self, query, params):
            self.executed = (query, params)

        def fetchall(self):
            return []

    class FakeConnection:
        def __init__(self):
            self.cursor_obj = FakeCursor()

        def __enter__(self):
            return self

        def __exit__(self, exc_type, exc, tb):
            return False

        def cursor(self):
            return self.cursor_obj

        def close(self):
            return None

    fake_connection = FakeConnection()

    def fake_connect(**kwargs):
        captured["connection"] = fake_connection
        return fake_connection

    monkeypatch.setattr(data_service.psycopg2, "connect", fake_connect)

    service = data_service.DataService({"host": "localhost"})
    from_dt = datetime(2026, 1, 1, 0, 0, 0)
    to_dt = datetime(2026, 1, 2, 0, 0, 0)

    result = service.get_data(from_dt, to_dt, "thing-9", ["temperature"])

    assert isinstance(result, pd.DataFrame)
    assert list(result.columns) == ["created", "thingid", "temperature"]
    assert captured["connection"].cursor_obj.executed[0].strip().startswith("SELECT created, thingid, vals")
    assert captured["connection"].cursor_obj.executed[1] == ("thing-9", from_dt, to_dt)
