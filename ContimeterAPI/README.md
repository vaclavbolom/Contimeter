# Contimeter API

FastAPI access to TimescaleDB measurements through `app.DataAccess.data_service.DataService`.
Python 3.13 or newer is required.

## Run locally

From the repository root, using [uv](https://docs.astral.sh/uv/):

```powershell
uv sync --project ContimeterAPI --group dev
$env:DB_PASSWORD = "your-local-database-password"
uv run --project ContimeterAPI uvicorn ContimeterAPI.app.main:app --host 127.0.0.1 --port 8000
```

Configuration is read from environment variables:

| Variable | Default |
| --- | --- |
| `DB_HOST` | `localhost` |
| `DB_PORT` | `5432` |
| `DB_NAME` | `sensordata` |
| `DB_USER` | `postgres` |
| `DB_PASSWORD` | empty |
| `DB_SSLMODE` | `prefer` |

Database connections are opened only when querying data, with a five-second connection timeout.
Use a read-only database account. This API has no authentication; keep it local or behind an
authenticated gateway before exposing it to a network.

## Docker

From the repository root:

```powershell
docker build -t contimeter-api ./ContimeterAPI
$env:DB_PASSWORD = "your-local-database-password"
docker run --rm --name contimeter-api -p 127.0.0.1:8001:8000 --env DB_PASSWORD --env DB_HOST=host.docker.internal --mount source=contimeter-api-logs,target=/app/ContimeterAPI/logs contimeter-api
```

Open http://127.0.0.1:8001/docs. Port 8001 on the host avoids the local development
server's port 8000. `host.docker.internal` accesses a database running on the host
with Docker Desktop; use the database's hostname when it runs elsewhere. On Linux
Docker Engine, add `--add-host=host.docker.internal:host-gateway` for host access.
Pass other `DB_*` settings with `--env` as needed; credentials are not baked into the image.

The image installs locked runtime dependencies, runs as a non-root user, and uses
one Uvicorn worker for file rotation. The named volume preserves logs across container
removal. Local environments, tests, logs, and environment files are excluded from the build.

## Query measurements

Open http://127.0.0.1:8000/docs for interactive API documentation.

```text
GET /data?from_datetime=2026-06-15T12:00:00Z&to_datetime=2026-06-15T13:00:00Z&thingid=device-1&variables=temperature&variables=humidity
```

Both timestamps must include a timezone. The range is inclusive. Repeat `variables`
for each requested measurement key; names must be nonempty, unique, and cannot be
`created` or `thingid`. Results are a JSON array containing `created`, `thingid`, and
the requested variables. Missing values become JSON `null`; no matches return `[]`.
Invalid queries return HTTP 422; database failures return HTTP 503.
Queries currently return all matching rows, so use bounded time ranges for large datasets.

## Routers

`app/main.py` configures logging and middleware and registers routers with
`app.include_router(...)`. The `/data` endpoint lives in `app/routers/data.py`.
Shared dependencies, including `get_data_service`, live in `app/dependencies.py`.
To add endpoints for another feature, create an `APIRouter` in `app/routers/` and
register it in `app/main.py`.

## Logging

The API automatically creates `ContimeterAPI/logs/api.log`, independent of the working
directory. A `RotatingFileHandler` rotates the file at 5 MB and retains five backups
(`api.log.1` through `api.log.5`). Records include timestamps, severity, and logger name.
Requests log their method, path, status, and duration at INFO level. Database and
unexpected failures log their exception type at ERROR level without exception messages,
query strings, request bodies, or database credentials. Uvicorn's own logs remain on the
console. Use one server worker with this handler; multiple processes need a centralized
logging solution to coordinate rotation.

## Tests

From the repository root:

```powershell
uv run --project ContimeterAPI python -m pytest ContimeterAPI/tests -q
```

API tests replace the data service and do not require a live database.