import paho.mqtt.client as mqtt
import time
import psycopg2
import json
import ssl
from datetime import UTC, datetime, timezone
import logging
from logger import setup_logging

# List of topics to subscribe to. Add or remove topics as needed.
TOPICS = [
    "contimeter/alej1",
    "contimeter/pve-radim",
    "boiler/events/rpc"
]

STORE_MEASUREMENT_QUERY = """
    INSERT INTO ml.runtime(created, thingid, vals) VALUES(%s,%s,%s) 
    ON CONFLICT ON CONSTRAINT runtime_pkey
    DO UPDATE SET vals = ml.runtime.vals::jsonb || %s
    """

setup_logging()
logger = logging.getLogger(__name__)

def on_connect(client, userdata, flags, return_code, properties):
    try:
        if return_code == 0:
            logger.debug(f"connected to {client._host}:{client._port}")
            # subscribe to all configured topics
            for t in TOPICS:
                client.subscribe(t)
                logger.info(f"subscribed to {t}")
        else:
            logger.error("could not connect, return code: %s", return_code)
            client.failed_connect = True
    except Exception as e:
        logger.exception("Error: ", e)



def on_message(client, userdata, message):
    logger.info("Received message: %s", str(message.payload.decode("utf-8")))
    try:
        payload = message.payload.decode("utf-8")
        logger.debug("Received message: %s", payload)
        decoded_message = json.loads(payload)
        logger.debug("decoded message: %s", decoded_message)
        if ("src" in decoded_message):
            decoded_message = extract(decoded_message)
            if (decoded_message == {}):
                logger.warning("No relevant data extracted from message, skipping database insert.")
                return

        connection = connect_db(parameters)
        send_data(connection, json.dumps(decoded_message))
    except Exception:
        logger.exception("Error sending data to database")


def connect_db(db_parameters):    
    connection = psycopg2.connect(**db_parameters)
    return connection


def send_data(connection, data):
    logger.info("data to send: %s", data)
    data_dict = json.loads(data)
    thingid = data_dict['thingid']
    data_dict.pop('thingid')
    timestamp = data_dict.get('created', datetime.now(timezone.utc).isoformat())
    data_to_send = data_dict.get('vals', data_dict)
    data_array = [timestamp, thingid, json.dumps(data_to_send), json.dumps(data_to_send)]
    cursor = connection.cursor()
    cursor.execute(STORE_MEASUREMENT_QUERY, data_array)
    connection.commit()
    cursor.close()


def reconnect(client):
    logger.debug(f"reconnect, connected:{client.is_connected()}")
    while not client.is_connected():
        logger.debug("Attempting to reconnect to MQTT broker...")
        client.connect(broker_hostname, port) 
        logger.debug("Reconnected to MQTT broker.")
        client.loop_start()
        logger.debug("reconnecting client: %s:%s...", broker_hostname, port)
        time.sleep(1)

def extract(sensor_data: dict, switch: str = "switch:0") -> dict:
    params = sensor_data["params"]
    values = params[switch]

    vals = {}
    for key in values:
        if key in ["apower", "current", "voltage", "freq"]:
            vals[key] = values[key]
                
    if vals == {}:
        logger.info("No relevant values found in sensor data: %s", sensor_data)
        return {}
    timestamp = datetime.fromtimestamp(params["ts"], tz=UTC)

    message = {
        "created": timestamp.strftime("%Y-%m-%dT%H:%M:%S.%fZ"),
        "thingid": sensor_data["src"],
        "vals": vals
    }
    
    logger.debug("extracted message: %s", message)
    return message


broker_hostname ="127.0.0.1"
# broker_hostname = "mq.contimeter.eu"
port = 1883
parameters = {
        "host": broker_hostname,
        "port": 5432,
        "user": "postgres",
        "password": "docker",
        "sslmode": "disable",
        "database": "sensordata"
    }


def main():
    try:
        logger.debug("Starting MQTT subscriber...")
        # client = mqtt.Client(transport="websockets")
        # client.tls_set(cert_reqs=ssl.CERT_NONE, tls_version=ssl.PROTOCOL_TLS_CLIENT) # uncomment if you use TLS
        # client.tls_insecure_set(True) # uncomment if you use TLS and want to skip certificate verification
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        client.username_pw_set(username="broker", password="brokersecret") # uncomment if you use password auth

        client.on_connect = on_connect
        client.on_message = on_message
        client.failed_connect = False
        logger.debug("Connecting to MQTT broker...")

        reconnect(client)
        logger.debug("Client connected to MQTT broker, starting loop...")
    except Exception as ex:
        logger.error(f"Error connecting to MQTT broker: {ex.message}")
        return

    while True:
        try:
            if not client.is_connected():
                reconnect(client)
            time.sleep(1)
        except KeyboardInterrupt:
            logger.info("Exiting...")
            break
        except Exception:
            logger.exception("Error in main loop")
        finally:
            try:
                client.disconnect()
                client.loop_stop()
            except Exception:
                logger.exception("Error during shutdown")


if __name__ == "__main__":
    main()