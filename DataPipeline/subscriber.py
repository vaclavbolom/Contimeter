import paho.mqtt.client as mqtt
import time
import psycopg2
import json
from datetime import datetime, timezone

# List of topics to subscribe to. Add or remove topics as needed.
TOPICS = [
    "contimeter/alej1",
    "contimeter/pve-radim"
]

STORE_MEASUREMENT_QUERY = """
    INSERT INTO ml.runtime(created, thingid, vals) VALUES(%s,%s,%s) 
    ON CONFLICT ON CONSTRAINT runtime_pkey
    DO UPDATE SET vals = ml.runtime.vals::jsonb || %s
    """

def on_connect(client, userdata, flags, return_code, properties):
    try:
        if return_code == 0:
            print(f"connected to {client._host}:{client._port}")
            # subscribe to all configured topics
            for t in TOPICS:
                client.subscribe(t)
                print(f"subscribed to {t}")
        else:
            print("could not connect, return code:", return_code)
            client.failed_connect = True
    except Exception as e:
        print("Error: ", e)


def on_message(client, userdata, message):
    print("Received message: ", str(message.payload.decode("utf-8")))
    try:
        decoded_message = json.loads(message.payload.decode("utf-8"))
        print(f"decoded message: {decoded_message}")
        connection = connect_db(parameters)
        send_data(connection, json.dumps(decoded_message))
    except Exception as e:
        print("Error sending data to database: ", e)


def connect_db(db_parameters):    
    connection = psycopg2.connect(**db_parameters)
    return connection


def send_data(connection, data):
    print(f"data to send: {data}")
    data_dict = json.loads(data)
    thingid = data_dict['thingid']
    data_dict.pop('thingid')
    data_array = [datetime.now(timezone.utc), thingid, json.dumps(data_dict), json.dumps(data_dict)]
    cursor = connection.cursor()
    cursor.execute(STORE_MEASUREMENT_QUERY, data_array)
    connection.commit()
    cursor.close()

def reconnect(client):
    while(not client.is_connected()):
        client.connect(broker_hostname, port) 
        client.loop_start()
        print(f"reconnecting client:  {broker_hostname}:{port}...")
        time.sleep(1)


broker_hostname ="127.0.0.1"
port = 1883 
parameters = {
        "host": broker_hostname,
        "port": 5432,
        "user": "postgres",
        "password": "docker",
        "sslmode": "disable",
        "database": "sensordata"
    }


client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
# client.username_pw_set(username="broker", password="brokersecret") # uncomment if you use password auth
client.on_connect = on_connect
client.on_message = on_message
client.failed_connect = False

reconnect(client)

while True:
    try:
        if not client.is_connected():
            reconnect(client)
        time.sleep(1)
    except KeyboardInterrupt:
        print("Exiting...")
    except Exception as e:
        print("Error: ", e)             
    finally:
        client.disconnect()
        client.loop_stop()