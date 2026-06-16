import json
import types

import pytest
from DataPipeline import subscriber


def test_on_message_calls_send_data(monkeypatch):
    # prevent real DB connection
    monkeypatch.setattr(subscriber, "connect_db", lambda params: None)

    called = {}

    def fake_send_data(connection, data):
        called['args'] = (connection, data)

    monkeypatch.setattr(subscriber, "send_data", fake_send_data)

    # create a dummy MQTT message object
    msg = types.SimpleNamespace()
    msg.payload = b'{"thingid": "device123", "temp": 22.5, "status": "ok"}'

    # call the handler
    subscriber.on_message(None, None, msg)

    assert 'args' in called, "send_data was not called"
    conn, data = called['args']
    # ensure connection returned by connect_db was passed (we returned None)
    assert conn is None
    decoded = json.loads(data)
    assert decoded['thingid'] == 'device123'
    assert decoded['temp'] == 22.5
    assert decoded['status'] == 'ok'
