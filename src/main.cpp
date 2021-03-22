/*
  Read temperature, humudity and MQ-2 sensor values and upload them OKOS iot platform.
*/

//Libaries
#include <SHT3x.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SERVER_ADDRESS "http://data.okosengineering.com/api/data/"
#define API_KEY "72909f1df88fa5e9250f03efa243b181c3a59e84"

#define STASSID "Wi-Waere"
#define STAPSK  "Danke678"
#define LOOP_SLEEP_TIME 3 * 60 * 1000

//Global declaration
SHT3x sht30(0x44); //adress of SHT30
const int analogInPin = A0;  //ADC-pin of AZ-Envy for the gas sensor
const int integrated_led = 2; //integrated led is connected to D2

float voltage, current, power, energy = 0;


void calibrateSTH30() {

  SHT3x::CalibrationPoints HumidityReference; //Points from reference sensor
  SHT3x::CalibrationPoints HumiditySHT; //Points from your SHT3x sensor
  HumidityReference.First = 0.;
  HumidityReference.Second = 100.;
  HumiditySHT.First = 5.3;
  HumiditySHT.Second = 97.;
  sht30.SetRelHumidityCalibrationPoints(HumiditySHT, HumidityReference);

  SHT3x::CalibrationFactors TemperatureCalibration;
  TemperatureCalibration.Factor = 0.9333; 
  TemperatureCalibration.Shift  = 0.2733;
  sht30.SetTemperatureCalibrationFactors(TemperatureCalibration);

}


void setupWiFi() {
  Serial.println("-------------WiFi--------------");
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("-------------------------------");
}


void postData(
                float temperature,
                float humidity,
                float gas_sensor,
                float voltage,
                float current,
                float power,
                float energy
) {
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, SERVER_ADDRESS); //HTTP
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Device", API_KEY);

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    char payload[500];
    sprintf(
        payload,
        "{\"weather_meter\":{\"temperature\": %f, \"humidity\": %f, \"gas_sensor\": %f}, \"load_meter\":{\"voltage\": %f, \"current\": %f, \"power\": %f, \"energy\": %f}}",
        temperature, humidity, gas_sensor, voltage, current, power, energy
    );

    Serial.println("payload is:\n<<");
    Serial.println(payload);

    int httpCode = http.POST(payload);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      const String& payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    setupWiFi();
  }
}


void setup() {

  Serial.begin(115200); //starting the serial communication with a baud rate of 115200 bps
  Serial.println("------------------------------"); //print to serial monitor
  Serial.println("-----------AZ-Envy------------"); //print to serial monitor
  Serial.println("------by Niklas Heinzel-------"); //print to serial monitor
  Serial.println("------------------------------"); //print to serial monitor

  pinMode(analogInPin, INPUT); //set ADC-pin as a input
  pinMode(integrated_led, OUTPUT); //set the integrated led as a output

//  Serial.println("------Calibrating Sensor------"); //print to serial 
//  calibrateSTH30();
//  Serial.println("------------------------------"); //print to serial monitor

  setupWiFi();
}

void loop() {

  sht30.UpdateData();
  //variables to work with
  float temperature = sht30.GetTemperature(SHT3x::Cel); //read the temperature from SHT30
  float humid = sht30.GetRelHumidity(); //read the humidity from SHT30
  int sensorValue = analogRead(analogInPin); //read the ADC-pin → connected to MQ-2

  //calibrate your temperature values - due to heat reasons from the MQ-2 (normally up to 4°C)
  float temperature_deviation = 0.0; //enter the deviation from the mq2 due to the resulting heat in order to calibrate the temperature value 
  float temperature_calibrated = temperature - temperature_deviation; //final value
  
  //-SHT30-//
  if(sht30.GetError() == 0){
    Serial.print("Temperature in Celsius: ");
    Serial.println(temperature_calibrated);
    Serial.print("Relative Humidity: ");
    Serial.println(humid);
  }
  else //if useless values are measured
  {
    Serial.println("Error, please check hardware or code!");
  }

  //-MQ-2-//
  Serial.println("----------------------------------------------"); //print to serial monitor
  Serial.print("MQ2-value: "); //print to serial monitor
  Serial.println(sensorValue); //print data to serial monitor
  Serial.println("----------------------------------------------"); //print to serial monitor


  voltage = random(220, 240) * 1.0;
  current = random(0, 100) * 1.0;
  power = voltage * current;
  energy += power * (LOOP_SLEEP_TIME / 1000) / 3600000;

  
  postData(
    temperature,
    humid,
    sensorValue * 1.0,
    voltage,
    current,
    power,
    energy
  );
 
  digitalWrite(integrated_led, HIGH);//turn the integrated led on
  
  delay(LOOP_SLEEP_TIME);

  digitalWrite(integrated_led, LOW);//turn the integrated led off
}
