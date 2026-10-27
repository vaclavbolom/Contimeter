import json
import logging
from typing import Annotated

import psycopg2
from fastapi import APIRouter, Depends, HTTPException, Query
from fastapi.responses import JSONResponse
from pydantic import AwareDatetime

from ..DataAccess.data_service import DataService
from ..dependencies import get_data_service

logger = logging.getLogger("contimeter.api")
router = APIRouter(tags=["data"])


@router.get("/data", response_class=JSONResponse)
def get_data(
    from_datetime: AwareDatetime,
    to_datetime: AwareDatetime,
    thingid: Annotated[str, Query(min_length=1)],
    variables: Annotated[list[str], Query(min_length=1)],
    service: Annotated[DataService, Depends(get_data_service)],
) -> JSONResponse:
    if from_datetime > to_datetime:
        raise HTTPException(422, "from_datetime must be less than or equal to to_datetime")
    if any(not name.strip() or name in {"created", "thingid"} for name in variables):
        raise HTTPException(422, "variables must be nonempty and cannot be created or thingid")
    if len(set(variables)) != len(variables):
        raise HTTPException(422, "variables must be unique")

    try:
        measurements = service.get_data(from_datetime, to_datetime, thingid, variables)
    except psycopg2.Error as exc:
        logger.error(f"Database query failed: {type(exc).__name__}")
        raise HTTPException(503, "Database is unavailable") from exc

    return JSONResponse(json.loads(measurements.to_json(orient="records", date_format="iso")))