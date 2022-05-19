/*
  Read temperature, humudity and MQ-2 sensor values and upload them OKOS iot platform.
*/

//Libaries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SERVER_ADDRESS "http://data.okosengineering.com/api/data/"
#define API_KEY "72909f1df88fa5e9250f03efa243b181c3a59e84"

#define DEVICE_TYPE "IOT-GW-V1"
#define SERIAL_BUFFER_SIZE 1024
// #define ENABLE_SERIAL_DEBUG
#define STASSID ""
#define STAPSK  ""
#define LOOP_SLEEP_TIME 3 * 60

char payload[1500];
char serial_data[1000];
char temp_buffer[100];
long time_counter = LOOP_SLEEP_TIME;
bool serialDataValid = false;

void serial_print(char *message) {
#ifdef ENABLE_SERIAL_DEBUG
serial_print(message);
#endif
}

String sample_data() {
  String data = "#mSerial {\"Device\":{\"rUptime_s\":7217,\"rErrorFlags\":0},\"Battery\":{\"rMeas_V\":13.43,\"rMeas_A\":-0.03,\"rSOC_pct\":90,\"pEstUsable_Ah\":0.0,\"pChgDay_Wh\":26.97,\"pDisDay_Wh\":0.21,\"pDeepDisCount\":0,\"pDis_Ah\":-0},\"Charger\":{\"rState\":0,\"rDCDCState\":1},\"Solar\":{\"rMeas_V\":22.20,\"rMeas_A\":-0.03,\"pInDay_Wh\":29.01},\"Load\":{\"rMeas_A\":0.08,\"rState\":1,\"pOutDay_Wh\":2.24},\"USB\":{\"rState\":1}}";
  return data;
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
      Serial.print(payload);
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
  Serial.setTimeout(10);
  Serial.setRxBufferSize(SERIAL_BUFFER_SIZE);
  setupWiFi();
}

void loop() {

  String dataOriginal,  data;
  int startIdx = 0, endIdx = 0;
  bool dataValid = false;
  time_counter += 1;
  delay(900);

  if (Serial.available() > 50) {
    // data = Serial.readStringUntil('\n');
    dataValid = false;
    dataOriginal = Serial.readString();
    startIdx = dataOriginal.indexOf("#mSerial ");
    // data = sample_data();

    if (startIdx >= 0) {
      data = dataOriginal.substring(startIdx + 9);
      endIdx = data.indexOf("}}");
      
      if (endIdx > 0) {
        dataValid = true;
        data = data.substring(0, endIdx + 2);
      }
    }

    if (dataValid)
    {
      data.toCharArray(serial_data, 1000);
      serialDataValid = true;
      // Serial.print("Valid data: ");
      // Serial.println(serial_data);
    } else {
      Serial.print("Invalid data: ");
      // Serial.println(dataOriginal);
      data = "{}";
    }

    // Serial.println(serial_data);
  }

  if (time_counter >= LOOP_SLEEP_TIME) {
    time_counter = 0;
    
    if (!serialDataValid) {
      data = "{}";
      data.toCharArray(serial_data, 1000);
    }
    sprintf(
        payload,
        "{\"config\": {\"devType\": \"%s\",\"device\": %d,\"mac\": \"%s\"},\"data\": %s}",
        DEVICE_TYPE,
        0,
        temp_buffer,
        serial_data
    );

    postData(payload);
    serialDataValid = false;
    data.toCharArray(serial_data, 1000);
  }
}
