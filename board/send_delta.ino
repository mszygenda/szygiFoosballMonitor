/* Interface Accelerometer with NodeMCU
 * By TheCircuit
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <TroykaAccelerometer.h>
#ifndef STASSID
#define STASSID "WIFI_NAME"
#define STAPSK  "WIFI_PASS"
#define BASE_URL "http://172.16.2.90:3000"
#define LED D4

#define THRESHOLD 6
#define POSITIONS_ITERATIONS 30

#define VIBRATION_ITERATIONS 30

#define INIT_MODE 1
#define LOW_POWER_MODE 2
#define SEND_POSITIONS_MODE 3

#define ACCELEROMETER_SENSOR 1
#define VIBRATION_SENSOR 2

#define MEASURING_LOOP_COUNT 50

#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const int sensor_type = VIBRATION_SENSOR;
const int xPin = A0;    //x-axis of the Accelerometer 
const int vibrationPin = D0;

int mode = INIT_MODE;

void setup() {
  switch(sensor_type) {
    case ACCELEROMETER_SENSOR:
      accel__setup();
      break;
    case VIBRATION_SENSOR:
      vibration__setup();
      break;
  }
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);  
}

void accel__setup() {
  
}

void vibration__setup() {
  pinMode(vibrationPin, INPUT);
}

void enable_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void send_location(int value) {
  HTTPClient http;
  WiFiClient client;

  Serial.println("[HTTP] Sending position..." + String(value));

  String body = String("{\"value\":" + String(value) + "}");

  if (http.begin(client, String(BASE_URL) + "/position")) {  // HTTP
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(body);

    // httpCode will be negative on errorx
    if (httpCode <= 0) {
      Serial.printf("[HTTP] POST failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP] Unable to connect\n");
  }
}

int read_max_delta2(int n) {
  int value = analogRead(xPin);
  int max = value, min = value;

  for (int i = 0; i < n; i++) {
    delay(50);

    int value = analogRead(xPin);

    if (value < min) {
      min = value;
    }

    if (value > max) {
      max = value;
    }
  }

  return abs(max - min);
}

bool low_power_loop() {
//   ESP.deepSleep(3e6);
   delay(30000);
   Serial.println("Checking for vibrations");

   if (should_wake_up()) {
      Serial.println("Vibrations detected. Waking up");
      switch_mode(SEND_POSITIONS_MODE);
   } else {
      Serial.println("No vibrations. Continuuing deepSleep");
   }
}

bool should_wake_up() {
  switch(sensor_type) {
    case ACCELEROMETER_SENSOR:
      return accel__should_wake_up();
    case VIBRATION_SENSOR:
      return vibration__should_wake_up();
  }
}

bool vibration__should_wake_up() {
  return true;
}

bool accel__should_wake_up() {
  int max = read_max_delta2(10);
   
   for (int i = 0; i < 10; i++) {
    int value = read_max_delta2(20);
    if (value > max) {
      max = value;
    }
   }

   return max > THRESHOLD;
}

void switch_mode(int newMode) {
  mode = newMode;

  switch(newMode) {
    case SEND_POSITIONS_MODE:
      Serial.println("Switching to SEND_POSITIONS_MODE");
      enable_wifi();
      break;
    case LOW_POWER_MODE:
      Serial.println("Switching to LOW_POWER_MODE");
      disable_wifi();
    default:
      disable_wifi();
      break;
  }
}

void disable_wifi() {
  Serial.println("Disabling wifi");
  WiFi.mode(WIFI_OFF);
}

void send_positions_loop() {
  switch(sensor_type) {
    case VIBRATION_SENSOR:
      return vibration__send_positions_loop();
    case ACCELEROMETER_SENSOR:
      return accel__send_positions_loop();
  }
}

void accel__send_positions_loop() {
  int sum = 0;

  for (int i = 0; i < VIBRATION_ITERATIONS; i++) {
    int value = read_max_delta2(20);

    sum += value;
    send_location(value);
  }

  int avg = sum / VIBRATION_ITERATIONS;

  if (avg < THRESHOLD) {
    //switch_mode(LOW_POWER_MODE);
  }
}

void vibration__send_positions_loop() {
  int sum = 0;

  for (int i = 0; i < MEASURING_LOOP_COUNT; i++) {
    if (is_vibrating()) {
      sum++;
      send_location(10);
    }
  }

  double avg = sum / (double)MEASURING_LOOP_COUNT;

  if (avg < 0.1) {
    Serial.println("Should switch to low power mode" + String(avg));
    switch_mode(LOW_POWER_MODE);
  }
}


boolean is_vibrating() {
  int vibrating_samples = 0;
  
  for (int i = 0; i < VIBRATION_ITERATIONS; i++) {
    if(digitalRead(vibrationPin) == 1) {
      vibrating_samples++;
    }
    
    delay(50);
  }

  return vibrating_samples > 0;
}

void loop() {  
  switch(mode) {
    case INIT_MODE:
      delay(5000);
      switch_mode(SEND_POSITIONS_MODE);
      break;
    case LOW_POWER_MODE:
      low_power_loop();
      break;
    case SEND_POSITIONS_MODE:
      send_positions_loop();
      break;
  }
}
