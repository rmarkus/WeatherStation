/**
 * I am using https://github.com/finitespace/BME280 for BME280
 * MQTT 2.5.1 https://github.com/256dpi/arduino-mqtt
 *
*/ 

#include <WiFi.h>
#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <MQTT.h>


// communication via WLAN
#include "WifiSecrets.h"
MQTTClient client;
WiFiClient net;
int status = WL_IDLE_STATUS;

bool bme280_present = false;
// BME280
// Assumed environmental values:
//float referencePressure = 1022.9;  // hPa local QFF (official meteor-station reading)
//float outdoorTemp = 4;           // °C  measured local outdoor temp.
float barometerAltitude = 180.0;  // meters ... map readings + barometer position for my home location.


BME280I2C::Settings settings(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_16,
  BME280::SpiEnable_False,
  BME280I2C::I2CAddr_0x76);

BME280I2C bme(settings);

float temp = NAN;
float hum = NAN;
float pres = NAN;
float pressureSealevel = NAN;
float dewPoint = NAN;

BME280::TempUnit tempUnit = BME280::TempUnit_Celsius;
BME280::PresUnit presUnit = BME280::PresUnit_hPa;
EnvironmentCalculations::AltitudeUnit envAltUnit = EnvironmentCalculations::AltitudeUnit_Meters;
EnvironmentCalculations::TempUnit envTempUnit = EnvironmentCalculations::TempUnit_Celsius;


// the internal LED for debugging
#define ledPin 2
// the pin for the rain sensor (will interrupt)
#define sensorPin 27
// Analog-Digital input for the voltage measurement GPIO 36
#define voltagePin 36

// rain counter
int rain = 0;
unsigned long lastRainEventMillies = 0;
#define rainEventMinTimeMillies 10;

// doBlink: only give optical signals to LED when true. False is "darkmode" :)
bool doBlink = true;
// debug = true: output on serial. Note: time consuming!
bool debug = false;

// some flags to control the flow
bool isConnected = false;
bool hadEvent = false;



// positions from 0 - 4096
int voltRaw;
float volt;
// this has been done empirically by comparing some measurements with external voltmeter and doing linear.
// is ~0.01 V precise around 4V, which is enough for me to check the battery
#define V_FACT 0.00162363169530825
#define V_OFFS 0.311428014418271


// timer settings for deepsleep
#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 600         /* Time ESP32 will go to sleep (in seconds) */



void blink() {
  if (doBlink) {
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(100);
  }
}


void flash() {
  if (doBlink) {
    digitalWrite(ledPin, HIGH);
    delay(25);
    digitalWrite(ledPin, LOW);
  }
}




void readVoltage() {
  // check battery
  voltRaw = analogRead(voltagePin);
  volt = V_FACT * voltRaw + V_OFFS;
}

void initBME() {
  if (bme280_present) {
    // now initialize the BME280
    Wire.begin();

    // TBD: this is still not nice. If for some reason we cannot
    // get the BME sensor, we might want to do something else then looping forever.
    while (!bme.begin()) {
      delay(1000);
    }
  }
}

void readBMEData() {
  if (bme280_present) {
    bme.read(pres, temp, hum, tempUnit, presUnit);
    dewPoint = EnvironmentCalculations::DewPoint(temp, hum, envTempUnit);
    /// To get correct seaLevel pressure (QNH, QFF)
    ///    the altitude value should be independent on measured pressure.
    /// It is necessary to use fixed altitude point e.g. the altitude of barometer read in a map
    pressureSealevel = EnvironmentCalculations::EquivalentSeaLevelPressure(barometerAltitude, temp, pres, envAltUnit, envTempUnit);
  }
}

bool wokeUpTimer() {
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    return true;
  } else {
    return false;
  }
}

// this is called by the interrupt when the magnet sensor is closed.
void IRAM_ATTR sensorEvent() {
  bool minTimePassed = (millis() - lastRainEventMillies) > rainEventMinTimeMillies;
  if (minTimePassed) {
    lastRainEventMillies = millis();
    rain++;
  }
}

void mqttConnect() {
  client.begin(MQTTSERVER, net);
  int count = 0;
  while (!client.connect("Weatherstation2") && count < 10) {
    delay(100);
    count++;
  }
}

void mqttSend() {
  client.publish("wetter2/temperatur2", String(temp, 1));
  delay(100);
  client.publish("wetter2/luftfeucht2", String(hum, 1));
  delay(100);
  client.publish("wetter2/pressure2", String(pressureSealevel, 1));
  delay(100);
  client.publish("wetter2/battery_voltage2", String(volt, 2));
  delay(100);
  client.publish("wetter2/rain2", String(rain));
  delay(100);
  client.loop();
}

void mqttDisconnect() {
  client.disconnect();
}



void setup() {
  // setup the sensor for the rain detector
  pinMode(sensorPin, INPUT_PULLUP);
  //attachInterrupt(sensorPin, sensorEvent, FALLING);

  // connect the LED
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  blink();

  //
  initBME();
  // now read out the sensors
  readVoltage();
  readBMEData();

  delay(100);

  // connect to Wifi and send everything to the server
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("Weatherstation-Test");
  int cnt = 0;
  while (status != WL_CONNECTED and cnt < 10) {
    cnt++;
    WiFi.begin(SSID, PSK);
    status = WiFi.waitForConnectResult();
    // wait 10 seconds if not yet connected.
    if (status != WL_CONNECTED) {
      delay(1000);
    }
  }
  blink();

  // we only send if we have WLAN
  // otherwise we just go to sleep again and wait for better times.
  if (status == WL_CONNECTED) {
    mqttConnect();
    mqttSend();
    mqttDisconnect();
    WiFi.disconnect();
  }

  // enable wakeup reason:
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_27, 0);
  // goto sleep
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}





// main loop
void loop() {
  //   nothing to be done here - everything takes place in setup()
}
