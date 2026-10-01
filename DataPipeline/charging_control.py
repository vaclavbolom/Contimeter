from sqlalchemy import create_engine, text
from sqlalchemy.orm import Session
import pandas as pd
from datetime import datetime, timedelta, timezone
from typing import Optional


class TimescaleDBClient:
    """Client for querying TimescaleDB ml.runtime table"""
    
    def __init__(
        self,
        host: str,
        database: str,
        user: str,
        password: str,
        port: int = 5432,
    ):
        """
        Initialize TimescaleDB client with connection parameters
        
        Args:
            host: Database host
            database: Database name
            user: Username
            password: Password
            port: Port (default: 5432)
        """
        connection_string = f"postgresql+psycopg://{user}:{password}@{host}:{port}/{database}"
        self.engine = create_engine(connection_string)
    
    def get_runtime_data(
        self,
        thingid: Optional[str] = None,
        start_time: Optional[datetime] = None,
        end_time: Optional[datetime] = None,
        limit: int = 1000,
    ) -> pd.DataFrame:
        """
        Query ml.runtime table with optional filters
        
        Args:
            thingid: Filter by specific thing ID
            start_time: Filter data from this timestamp
            end_time: Filter data until this timestamp
            limit: Maximum number of rows to return
            
        Returns:
            DataFrame with columns: created, thingid, vals
        """
        query = "SELECT created, thingid, vals FROM ml.runtime WHERE 1=1"
        params = {}
        
        if thingid:
            query += " AND thingid = :thingid"
            params["thingid"] = thingid
        
        if start_time:
            query += " AND created >= :start_time"
            params["start_time"] = start_time
        
        if end_time:
            query += " AND created <= :end_time"
            params["end_time"] = end_time
        
        query += f" ORDER BY created DESC LIMIT {limit}"
        
        with self.engine.connect() as connection:
            df = pd.read_sql(text(query), connection, params=params)

        
        
        # Expand vals JSONB column into individual columns
        if not df.empty:
            df["created"] = df["created"].dt.floor("s")
            vals_df = pd.json_normalize(df["vals"].apply(lambda x: x if isinstance(x, dict) else {}))
            df = pd.concat([df.drop(columns=["vals"]), vals_df], axis=1)
        
        return df
    
    def get_last_readings(self, minutes: int = 60, thingid: Optional[str] = None) -> pd.DataFrame:
        """Get readings from the last N minutes"""
        start_time = datetime.now(tz=timezone.utc) - timedelta(minutes=minutes)
        return self.get_runtime_data(start_time=start_time, thingid=thingid)
    
    def get_thing_latest(self, thingid: str) -> pd.DataFrame:
        """Get latest reading for a specific thing"""
        return self.get_runtime_data(thingid=thingid, limit=1)


# Example usage
if __name__ == "__main__":
    # Set up connection with parameters
    client = TimescaleDBClient(
        host="localhost",
        database="sensordata",
        user="postgres",
        password="docker",
        port=5432,
    )
    
    thingid = 'pve-radim'
    # Get last hour of data
    df = client.get_last_readings(minutes=60, thingid=thingid)
    print(df.head())
    
    # Access JSONB data
    data_columns = df.columns.to_list()[2:]
    for idx, row in df.iterrows():
        print(f"Thing {row['thingid']} - {row['created']}")
        print(f"  Values: {row[data_columns].to_dict()}")
        # Access specific properties: row['vals']['property_a']
