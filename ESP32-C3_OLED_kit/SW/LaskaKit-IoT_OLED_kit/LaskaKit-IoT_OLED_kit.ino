/*
* Example code for ESP32-C3_OLED_kit 
*
* ESP32-C3-LPKit reads data from:
* * SCD41:  It shows the CO2, temperature and humidity
* or
* * BME280: It shows the pressure, temperature and humidity
* or
* * SHT40:  It shows the temperature and humidity
* on OLED display
*
* Hardware:
* LaskaKit ESP32-C3-LPKit                                   - https://www.laskakit.cz/laskkit-esp-12-board/
* LaskaKit SCD41 Senzor CO2, teploty a vlhkosti vzduchu     - https://www.laskakit.cz/laskakit-scd41-senzor-co2--teploty-a-vlhkosti-vzduchu/
* LaskaKit BME280 Senzor tlaku, teploty a vlhkosti vzduchu  - https://www.laskakit.cz/arduino-senzor-tlaku--teploty-a-vlhkosti-bme280/
* LaskaKit SHT40 Senzor teploty a vlhkosti vzduchu          - https://www.laskakit.cz/laskakit-sht40-senzor-teploty-a-vlhkosti-vzduchu/
* LaskaKit OLED displej 128x64 1.3" I²C                     - https://www.laskakit.cz/laskakit-oled-displej-128x64-1-3--i2c/
*
* Library:
* https://github.com/sparkfun/SparkFun_SCD4x_Arduino_Library
* https://github.com/adafruit/Adafruit_BME280_Library
* https://github.com/adafruit/Adafruit_SHT4X
* https://github.com/adafruit/Adafruit_SH110x
*
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Math.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <PubSubClient.h>

// Choose your version | Vyber svou verze kitu
#define SCD41
// #define BME280
// #define SHT4x

// OLED 
// uncomment only one of them, check the solderbridge on LaskaKit OLED
#define OLED_Address 0x3c //initialize with the I2C addr 0x3C
//#define OLED_Address 0x3d  //initialize with the I2C addr 0x3D
#define POWER 4               // Power pin 

#ifdef SCD41
  #include <SparkFun_SCD4x_Arduino_Library.h>
  SCD4x SSCD41;
#endif
#ifdef BME280
  #include <Adafruit_BME280.h> 
  Adafruit_BME280 bme;
#endif
#ifdef SHT4x
  #include <Adafruit_SHT4x.h>
  Adafruit_SHT4x sht4 = Adafruit_SHT4x();
#endif

// fonts
#include "DSEG14_Classic_Bold_32px.h"
#include "DSEG14_Classic_Bold_12px.h"

//mqtt helper
#include "my_mqtt.h"

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

int co2 = 0;
float temperature = 0.0;
float temperature2 = 0.0;
int pressure = 0;
int humidity = 0;
int humidity2 = 0.0;

float temperature_correction = 0;
float temperature2_correction = 0;

bool show_display = true;

// WiFi
const char *ssid = "Vodafone-E39C"; // Enter your WiFi name
const char *password = "TrakturekPtacek40111";  // Enter WiFi password
bool debug_measurement = true;
bool debug_detailed = false;


WiFiClient espClient;
PubSubClient client(espClient);

// MQTT Broker
const char *mqtt_broker = "192.168.0.115";
const char *mqtt_username = "broker";
const char *mqtt_password = "brokersecret";
const int mqtt_port = 1883;
const char *thingid = "alej1";

// MQTT Publish Topics Array
const char *publish_topics[] = {
  "contimeter/alej1",             // Main topic for publishing data
  "contimeter/alej1/co2",         // CO2 readings
  "contimeter/alej1/temperature", // Temperature readings
  "contimeter/alej1/humidity",    // Humidity readings
  "contimeter/alej1/status"       // Status messages
};
const int NUM_PUBLISH_TOPICS = sizeof(publish_topics) / sizeof(publish_topics[0]);

// MQTT Subscribe Topics Array
const char *subscribe_topics[] = {
  "contimeter/alej1/display",     // Display control commands
  "contimeter/alej1/config",      // Configuration updates
  "contimeter/alej1/calibration",   // Calibration commands
  "contimeter/alej1/reset"        // Reset commands
};
const int NUM_SUBSCRIBE_TOPICS = sizeof(subscribe_topics) / sizeof(subscribe_topics[0]);

// Legacy topic variables (for backward compatibility)
const char *topic = publish_topics[0];
const char *subscribe_topic = subscribe_topics[0];

int counter = 0;
String mqtt_message = "1";
String display_message = "1";

void setup() {
  // Speed of Serial
  Serial.begin(115200);
  pinMode(POWER, OUTPUT); 
  digitalWrite(POWER, HIGH); // enable power supply for uSup
  delay(500);   

  Serial.println("Setup start");
  // set dedicated I2C pins 8 - SDA, 10 SCL for ESP32-C3-LPKit
  Wire.begin(8, 10);

  /*----- OLED sequence ------*/
  delay(250);                        // wait for the OLED to power up
  display.begin(OLED_Address, true);
  //display.setContrast (0); // dim display

  // Clear the buffer.
  display.clearDisplay();

  #ifdef SCD41
    /*----- SCD41 sequence ------*/
    //             begin, autoCalibrate
    //               |      |
    if (SSCD41.begin(false, true) == false) {
      Serial.println("SCD41 was not set correctly. Check the connection.");
      display.clearDisplay();
      displayMessage("SCD41", "NOK, was it correct defined?");
      while (1) ;
    }
    displayMessage("SCD41", "OK, waiting for measurement");
    if (SSCD41.startLowPowerPeriodicMeasurement() == true) {
      Serial.println("Low power mode enabled.");
    }
  #endif
  #ifdef BME280
    unsigned status;
    
    // default settings
    status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }
  #endif
  #ifdef SHT4x
    /*----- SHT4x seqence ------*/
    if (! sht4.begin()) {
      Serial.println("SHT4x not found");
      display.clearDisplay();
      displayMessage("SHT4x", "NOK, was it defined correct?");
      while (1) delay(1);
    }
    displayMessage("SHT4x", "OK, waiting for measurement");
    sht4.setPrecision(SHT4X_HIGH_PRECISION); // highest resolution
    sht4.setHeater(SHT4X_NO_HEATER); // no heater
  #else
    Serial.print("Error, no sensor defined!");
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 40);
    display.println("Error, no sensor defined!");
  #endif


  setup_wifi(ssid, password);
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  //reconnect_broker(client, subscribe_topics, NUM_SUBSCRIBE_TOPICS);
  Serial.println("Setup done");
}


void loop() {
  if (!client.connected())
  {
    Serial.println("client not connected...");
    reconnect_broker(client, subscribe_topics, NUM_SUBSCRIBE_TOPICS);
  }

  client.loop();

#ifdef SCD41
  // if(!SSCD41.readMeasurement()) { // wait for a new data (approx 30s)
  //   // Serial.println("SCD41 not ready");
  //   delay(1000);
  //   return;
  // }

  if (SSCD41.readMeasurement())
  {
    Serial.println("SCD41 read mesurement");
    co2 = SSCD41.getCO2();
    temperature2 = SSCD41.getTemperature();
    humidity2 = SSCD41.getHumidity();

    if (debug_measurement){
      Serial.println();
      Serial.print("CO2(ppm):\t");
      Serial.println(co2);
      Serial.print("Temperature(C):\t");
      Serial.print(temperature2);
      Serial.print(" / ");
      Serial.print(temperature2 + temperature2_correction);
      Serial.println(" °C");
      Serial.print("Humidity(%RH):\t");
      Serial.println(humidity2);
    }
  }
#endif

#ifdef BME280
  
  Serial.println("BME280 read mesurement");
  temperature = bme.readTemperature();
  humidity    = bme.readHumidity();
  pressure    = bme.readPressure() / 100.0F;  

  if (debug_measurement){
    Serial.println();
    Serial.print("Pressure:\t");
    Serial.print(pressure);
    Serial.println(" kPa");
    Serial.print("Temperature:\t");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity:\t");
    Serial.print(humidity);
    Serial.println("% rH");
  }
#endif

#ifdef SHT4x
  sensors_event_t hum, temp; // temperature and humidity variables
 
  sht4.getEvent(&hum, &temp);
  temperature = temp.temperature;
  humidity    = hum.relative_humidity;

  if (debug_measurement){
    Serial.println();
    Serial.println("SHT4x measurement");
    Serial.print("Temperature:\t");
    Serial.print(temperature);
    Serial.print(" / ");
    Serial.print(temperature + temperature_correction);
    Serial.println(" °C");
    Serial.print("Humidity:\t");
    Serial.print(humidity);
    Serial.println("% rH");

    Serial.println("SCD measurement");
    Serial.print("Temperature:\t");
    Serial.print(temperature2);
    Serial.print(" / ");
    Serial.print(temperature2 + temperature2_correction);
    Serial.println(" °C");
    Serial.print("Humidity:\t");
    Serial.print(humidity2);
    Serial.println("% rH");
  }
#endif

  if (!(counter % 60))
  {
    publish_temp_hum(temperature + temperature_correction, humidity, temperature2 + temperature2_correction, humidity2, co2, client, topic, thingid);

  }


  display.clearDisplay();
  show_display = (display_message == "1");
  if (show_display)
  {
    display.setContrast(255);
    // CO2
    display.setFont(&DSEG14_Classic_Bold_32);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(10, 40);
    display.println(co2);
    // Temperature
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(20, 60);
    display.print((round((temperature2 + temperature2_correction) * 10) / 10.0), 1);
    display.println("C");
    // Humidity
    display.setFont(&DSEG14_Classic_Bold_12);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(80, 60);
    display.print(humidity2);
    display.println("%");
    // update display
    display.display();
  }
  else{
    display.setContrast(0);    
    clearDisplay();
  }
  
  delay(1000);
  counter++;
  // show_display = !show_display;
  if (debug_detailed){
    Serial.println("End of loop");
    Serial.print("show display: ");
    Serial.println(show_display);
  }
}

void displayMessage(char* sensor, char* message){
    
    display.setFont(NULL);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(5, 5);
    display.print("Senzor ");
    display.print(sensor);
    display.print(" is ");
    display.setCursor(5, 20);
    display.println(message);
      // update display
    display.display();
}

void clearDisplay(){
  display.setFont(NULL);
    display.setTextColor(SH110X_BLACK);
    display.setCursor(5, 5);
    display.print("       ");
    display.setCursor(5, 20);
    display.println("       ");
      // update display
    display.display();
}

float get_calibration_temperature(String message)
{
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return 0;
  }
  float calibration_temp = doc["temperature"];
  Serial.println(message);
  Serial.println(calibration_temp);
  return calibration_temp;
}

void callback(char* topic, byte* payload, unsigned int length)
{
  String topic_message = String((char*)topic);
  mqtt_message = "";
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char) payload[i]);
    mqtt_message += (char)payload[i];
  }
  Serial.println("");

  if (topic_message == "contimeter/alej1/display"){
    display_message = mqtt_message;
    Serial.print("Display message set to: ");
    Serial.println(display_message);
  }
  else if (topic_message == "contimeter/alej1/calibration"){
    Serial.println("Calibration command received");
    float calibration_temperature = get_calibration_temperature(mqtt_message);
    Serial.print("Calibration temperature: ");
    Serial.println(calibration_temperature);
    temperature_correction = calibration_temperature - temperature;
    temperature2_correction = calibration_temperature - temperature2;
    Serial.print(temperature);
    Serial.print(" / ");
    Serial.print(temperature2);
    Serial.println(" deg C");
  }
  Serial.println();
}