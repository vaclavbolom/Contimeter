from time import perf_counter

from fastapi import FastAPI, Request
from starlette.responses import Response

from .logging_config import setup_logging
from .routers import data

logger = setup_logging()
app = FastAPI(title="Contimeter API")
app.include_router(data.router)


@app.middleware("http")
async def log_request(request: Request, call_next) -> Response:
    started = perf_counter()
    try:
        response = await call_next(request)
    except Exception as exc:
        logger.error(
            f"{request.method} {request.url.path} status=500 "
            f"duration_ms={(perf_counter() - started) * 1000:.2f} "
            f"error={type(exc).__name__}"
        )
        raise
    logger.info(
        f"{request.method} {request.url.path} status={response.status_code} "
        f"duration_ms={(perf_counter() - started) * 1000:.2f}"
    )
    return response
