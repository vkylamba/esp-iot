/*
  Read temperature, humudity and MQ-2 sensor values and upload them OKOS iot platform.
*/

//Libaries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SERVER_ADDRESS "http://data.okosengineering.com/api/data/"
#define API_KEY "72909f1df88fa5e9250f03efa243b181c3a59e84"

#define DEVICE_TYPE "IOT-GW-V1"
#define ENABLE_SERIAL_DEBUG
#define STASSID "Vodafone-983C"
#define STAPSK  "M@yo678!"
#define LOOP_SLEEP_TIME 3 * 60

char payload[1500];
char serial_data[1000];
char temp_buffer[100];
long time_counter = LOOP_SLEEP_TIME;


void serial_print(char *message) {
#ifdef ENABLE_SERIAL_DEBUG
serial_print(message);
#endif
}

char* sample_data() {
  return "";
}

void setupWiFi() {
  // serial_print("-------------WiFi--------------");
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    serial_print(".");
  }

  WiFi.macAddress().toCharArray(temp_buffer, 100);
  // serial_print("Connected! IP address: ");
  // serial_print(WiFi.localIP());
  // serial_print("-------------------------------");
}

void postData(char *payload) {
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    serial_print("[HTTP] begin...\n");
    // configure server and url
    http.begin(client, SERVER_ADDRESS); //HTTP
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device", API_KEY);

    serial_print("[HTTP] POST...\n");
    // start connection and send HTTP header and body

    serial_print("payload is:\n<<");
    serial_print(payload);

    int httpCode = http.POST(payload);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      // serial_print("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      const String& payload = http.getString();
      // serial_print("received payload:\n<<");
      // serial_print(payload);
      // serial_print(">>");
    } else {
      // serial_print("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    setupWiFi();
  }
}


void setup() {

  Serial.begin(115200); //starting the serial communication with a baud rate of 115200 bps
  setupWiFi();
}

void loop() {

  time_counter += 1;
  delay(1000);

  if (Serial.available() > 0) {
    String data = Serial.readString();
    data.toCharArray(serial_data, 1000);
  }

  if (time_counter >= LOOP_SLEEP_TIME) {
    time_counter = 0;

    sprintf(
        payload,
        "{\"config\": {\"devType\": \"%s\",\"device\": %d,\"mac\": \"%s\"},\"data\":\"%s\"}",
        DEVICE_TYPE,
        0,
        temp_buffer,
        serial_data
    );

    postData(payload);
  }
}
