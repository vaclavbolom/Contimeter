import json
from datetime import datetime, timezone

from DataPipeline import subscriber


def test_send_data_executes_query(monkeypatch):
    payload = {
        "thingid": "pve-radim",
        "created": "2026-06-15T13:44:05.399812+02:00",
        "vals": {
            "solar_energy": "3.1",
            "load_consumption": "0.1",
            "export_to_grid": "3",
            "import_from_grid": "0"
        }
    }

    # Fix current time to make assertion deterministic
    fixed_now = datetime(2026, 6, 15, 11, 44, 5, 399812, tzinfo=timezone.utc)

    class DummyDate:
        @staticmethod
        def now(tz):
            return fixed_now

    monkeypatch.setattr(subscriber, "datetime", DummyDate)

    class FakeCursor:
        def __init__(self):
            self.closed = False
            self.exec_args = None

        def execute(self, query, params):
            self.exec_args = (query, params)

        def close(self):
            self.closed = True

    class FakeConn:
        def __init__(self, cur):
            self._cur = cur
            self.committed = False

        def cursor(self):
            return self._cur

        def commit(self):
            self.committed = True

    cur = FakeCursor()
    conn = FakeConn(cur)

    subscriber.send_data(conn, json.dumps(payload))

    assert cur.exec_args is not None, "cursor.execute was not called"
    query, params = cur.exec_args
    # query string may contain whitespace differences; compare stripped
    assert query.strip() == subscriber.STORE_MEASUREMENT_QUERY.strip()
    # first param is the timestamp we fixed
    assert params[0] == fixed_now
    # thing id should be passed as second param
    assert params[1] == "pve-radim"

    # the 3rd and 4th params are JSON strings representing the remaining payload
    p2 = json.loads(params[2])
    p3 = json.loads(params[3])
    expected = payload.copy()
    expected.pop("thingid")
    assert p2 == expected
    assert p3 == expected

    assert conn.committed is True
    assert cur.closed is True
