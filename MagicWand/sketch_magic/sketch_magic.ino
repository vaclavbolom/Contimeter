// ESP32-C3 Super Mini
// Board: "ESP32C3 Dev Module", USB CDC On Boot: "Enabled"

const int UV_PIN = 8;  // Onboard LED on Super Mini (active LOW)
const int BUTTON_PIN = 4;  // Example button pin
const int R_PIN = 2;
const int G_PIN = 3;
const int B_PIN = 4;
const int TEST_PIN = 0;

const int UV_DELAY_ON = 2000;
const int BLINK_INTERVAL = 500;

volatile unsigned long counter = 0;
volatile bool read_press = false;
volatile bool uv_on = false;
volatile unsigned long current = 0;
volatile unsigned long previous = 0;
int uv_led_on = LOW;


void start_pressed(){
  
  if (uv_on){
    uv_on = false;
    digitalWrite(UV_PIN, LOW);
    read_press = false;
  }else{
    counter = millis();
    read_press = true;
  }
}

void start_released(){
  read_press = false;
}

void control_uv(){
  read_press &= ((digitalRead(BUTTON_PIN) == LOW) && (digitalRead(UV_PIN) == LOW));
  if (read_press){    
    unsigned long actual_counter = millis();
    unsigned long delay = actual_counter - counter;

    if (delay >= UV_DELAY_ON){
      Serial.println("UV on");
      uv_on = true;
      digitalWrite(UV_PIN, HIGH);
    }
  }
}

void blink_led(int pin){
  current = millis();
  
  if  ((current - previous) >= BLINK_INTERVAL){
    previous = current;
    if (uv_led_on == LOW){
      uv_led_on = HIGH;      
    }
    else{
      uv_led_on = LOW;
    }
    digitalWrite(pin, uv_led_on);

  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Activating magic wand");
  pinMode(UV_PIN, OUTPUT);
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(TEST_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), start_pressed, FALLING);
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), start_released, FALLING);
  Serial.println("Magic wand activated");
  digitalWrite(TEST_PIN, HIGH);

}

void loop() {
  blink_led(TEST_PIN);
  control_uv();

}
