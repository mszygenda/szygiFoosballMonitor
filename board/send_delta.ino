/* Interface Accelerometer with NodeMCU
 * By TheCircuit
 */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "YOUR_SSID"
#define STAPSK  "YOUR_PASSWORD"
#define BASE_URL "http://YOUR_HOST:3000"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;
const int xPin = A0;    //x-axis of the Accelerometer 

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

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

  Serial.print("[HTTP] Sending position..." + value);

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

int read_average_delta(int n) {
  int sum = 0;

  for (int i = 0; i < n; i++) {
    int x = read_delta();
    
    sum += x;
  }

  return sum / n;
}

int read_max_delta(int n) {
  int max = read_delta();

  for (int i = 0; i < n; i++) {
    int x = read_delta();
    if (x > max) {
      max = x;
    }
  }

  return max;
}

int read_delta() {
  int first_value = analogRead(xPin);

  delay(100);
  
  int second_value = analogRead(xPin);

  return abs(second_value - first_value);
}

void loop() {
  send_location(
    read_max_delta(10)
  );
}
