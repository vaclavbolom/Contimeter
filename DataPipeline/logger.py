import logging
import os
from logging.handlers import RotatingFileHandler

# Ensure log directory exists next to this file
LOG_DIR = os.path.join(os.path.dirname(__file__), "logs")
os.makedirs(LOG_DIR, exist_ok=True)
LOG_FILE = os.path.join(LOG_DIR, "subscriber.log")

def setup_logging(level=logging.INFO, logfile=LOG_FILE):
    root = logging.getLogger()
    if root.handlers:
        return
    root.setLevel(level)
    fmt = logging.Formatter("%(asctime)s %(levelname)s [%(name)s] %(message)s")

    # File handler with rotation
    fh = RotatingFileHandler(logfile, maxBytes=5 * 1024 * 1024, backupCount=5)
    fh.setFormatter(fmt)
    root.addHandler(fh)

    # Console handler
    ch = logging.StreamHandler()
    ch.setFormatter(fmt)
    root.addHandler(ch)

    # Reduce verbosity of some noisy libraries
    for noisy in ("paho", "psycopg2", "urllib3"):
        logging.getLogger(noisy).setLevel(logging.WARNING)
