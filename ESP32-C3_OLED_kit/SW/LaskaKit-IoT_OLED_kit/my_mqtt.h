void reconnect_broker(PubSubClient& client, const char* subscribe_topics[], int num_topics)
{
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    // if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    if (client.connect(client_id.c_str())) {
        Serial.println("Public EMQX MQTT broker connected");
        
        // Subscribe to all topics in the array
        for (int i = 0; i < num_topics; i++) {
          Serial.print("Topic ");
          Serial.print(subscribe_topics[i]);
          Serial.println(" subscribed.");
          client.subscribe(subscribe_topics[i]);
        }
    } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }  
  return;
}

void setup_wifi(const char* ssid, const char* password)
{
  // Connecting to a Wi-Fi network
  Serial.println("Start to connect WiFi.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi.");
  Serial.println(WiFi.localIP());
}

void publish_temp_hum(float temp, int humidity, float temp2, int humidity2, int co2, PubSubClient& client, const char* topic, const char* thingid )
{
    char buffer[130];
    // temp = round(temp * 10)/10.0;
    // temp2 = round(temp2 * 10)/10.0;
    sprintf(buffer, "{\"humidity\": %d, \"temperature\": %f, \"humidity2\": %d, \"temperature2\": %f, \"co2\": %d, \"thingid\": \"%s\"}", humidity, temp, humidity2, temp2, co2, thingid);

    Serial.println(buffer);
    client.publish(topic, buffer);
    Serial.println("measurements published");
}

