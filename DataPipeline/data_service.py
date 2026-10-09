"""Read runtime measurements from TimescaleDB into a pandas DataFrame."""

from contextlib import closing
from datetime import datetime
import json
from typing import Any, Mapping, Sequence

import pandas as pd
import psycopg2


class DataService:
	"""Query measurements in ``ml.runtime``.

	``db_parameters`` should contain the keyword arguments accepted by
	``psycopg2.connect``, such as ``host``, ``database``, ``user``, and
	``password``.
	"""

	def __init__(self, db_parameters: Mapping[str, Any]) -> None:
		self._db_parameters = dict(db_parameters)

	def get_data(
		self,
		from_datetime: datetime,
		to_datetime: datetime,
		thingid: str,
		variables: Sequence[str],
	) -> pd.DataFrame:
		"""Return measurements in the inclusive time range.

		The result has ``created`` and ``thingid`` columns followed by one
		column per requested key in the ``vals`` JSON object. Missing JSON
		keys are returned as ``None`` (and represented by pandas as nulls).
		"""
		if from_datetime > to_datetime:
			raise ValueError("from_datetime must be less than or equal to to_datetime")

		variable_names = list(variables)
		if any(not isinstance(name, str) for name in variable_names):
			raise TypeError("variables must contain only strings")

		query = """
			SELECT created, thingid, vals
			FROM ml.runtime
			WHERE thingid = %s AND created >= %s AND created <= %s
			ORDER BY created, thingid
		"""

		with closing(psycopg2.connect(**self._db_parameters)) as connection:
			with connection.cursor() as cursor:
				cursor.execute(query, (thingid, from_datetime, to_datetime))
				rows = cursor.fetchall()

		records: list[dict[str, Any]] = []
		for created, thingid, values in rows:
			if isinstance(values, str):
				values = json.loads(values)
			if values is None:
				values = {}

			record = {"created": created, "thingid": thingid}
			record.update({name: values.get(name) for name in variable_names})
			records.append(record)

		return pd.DataFrame(
			records,
			columns=["created", "thingid", *variable_names],
		)
