/* Interface Accelerometer with NodeMCU
 * By TheCircuit
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
#define BASE_URL "http://172.16.2.90:3000"
#define LED D4

#define INIT_MODE 1
#define LOW_POWER_MODE 2
#define SEND_POSITIONS_MODE 3

#define VIBRATION_SENSOR 1

#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const int sensor_type = VIBRATION_SENSOR;
const int vibrationPin = D8;

int mode = INIT_MODE;

int get_and_increment_boot_count() {
  int boot_count = rtcmem.read32(0);

  rtcmem.write32(0, boot_count + 1);

  return boot_count;
}

void setup() {
  int bootCount = get_and_increment_boot_count();
  
  if (bootCount == 0) {
    Serial.println("Initial boot. Sleeping for 10 seconds");
    delay(10000);
  }
  
  bootCount++;

  // initialize serial communications at 9600 bps:
  Serial.begin(9600);  
  
  switch(sensor_type) {
    case VIBRATION_SENSOR:
      vibration__setup();
      break;
  }
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

bool low_power_loop() {
   Serial.println("Going in deepSleep");
   ESP.deepSleep(30 * 1000000);
}

void init_loop() {
   if (should_wake_up()) {
      Serial.println("Vibrations detected. Waking up");
      switch_mode(SEND_POSITIONS_MODE);
   } else {
      switch_mode(LOW_POWER_MODE);
   }
}

bool should_wake_up() {
  switch(sensor_type) {
    case VIBRATION_SENSOR:
      return vibration__should_wake_up();
  }
}

bool vibration__should_wake_up() {
   return is_vibrating(6000);
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
      break;
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
  }
}

void vibration__send_positions_loop() {
  send_location(10);
  switch_mode(LOW_POWER_MODE);
}


/**
 * Checks if there's any vibration over period of 1 second
 */
boolean is_vibrating(int samplingPeriodMs) {
  int samplingDelay = 50;
  int iterations = samplingPeriodMs / samplingDelay;
  
  for (int i = 0; i < iterations; i++) {
    if(digitalRead(vibrationPin) == 1) {
      return true;
    }
    
    delay(samplingDelay);
  }
}

void loop() {
  switch(mode) {
    case INIT_MODE:
      init_loop();
      break;
    case LOW_POWER_MODE:
      low_power_loop();
      break;
    case SEND_POSITIONS_MODE:
      send_positions_loop();
      break;
  }
}
