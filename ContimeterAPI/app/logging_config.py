import logging
from logging.handlers import RotatingFileHandler
from pathlib import Path

LOG_DIR = Path(__file__).resolve().parents[1] / "logs"


def setup_logging(log_directory: Path = LOG_DIR) -> logging.Logger:
    logger = logging.getLogger("contimeter.api")
    logger.setLevel(logging.INFO)
    log_directory.mkdir(parents=True, exist_ok=True)
    log_file = str((log_directory / "api.log").resolve())
    if not any(
        isinstance(handler, RotatingFileHandler) and handler.baseFilename == log_file
        for handler in logger.handlers
    ):
        handler = RotatingFileHandler(
            log_file, maxBytes=5 * 1024 * 1024, backupCount=5, encoding="utf-8"
        )
        handler.setFormatter(logging.Formatter(
            "%(asctime)s %(levelname)s [%(name)s] %(message)s"
        ))
        logger.addHandler(handler)
    return logger