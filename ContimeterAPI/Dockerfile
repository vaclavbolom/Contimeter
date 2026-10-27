FROM ghcr.io/astral-sh/uv:latest AS uv

FROM python:3.13-slim AS builder

COPY --from=uv /uv /usr/local/bin/uv

ENV UV_COMPILE_BYTECODE=1 \
    UV_LINK_MODE=copy \
    UV_PYTHON_DOWNLOADS=never

WORKDIR /app/ContimeterAPI
COPY pyproject.toml uv.lock ./
RUN uv sync --locked --no-dev --no-install-project

FROM python:3.13-slim AS runtime

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    PATH="/app/ContimeterAPI/.venv/bin:$PATH"

WORKDIR /app

RUN groupadd --system api \
    && useradd --system --gid api --no-create-home api \
    && mkdir -p /app/ContimeterAPI/logs \
    && chown api:api /app/ContimeterAPI/logs

COPY --from=builder /app/ContimeterAPI/.venv /app/ContimeterAPI/.venv
COPY app /app/ContimeterAPI/app

USER api

EXPOSE 8000

CMD ["python", "-m", "uvicorn", "ContimeterAPI.app.main:app", "--host", "0.0.0.0", "--port", "8000", "--workers", "1"]