#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#include <AceButton.h>
using namespace ace_button;

#include "DHTesp.h"
DHTesp dhtToilet;
DHTesp dhtOutside;
BlynkTimer timer;
//SimpleTimer timer;




#define WIFI_SSID "xx"
#define WIFI_PASSWD "xxx"
#define Blynk_KEY "xxxx"
#define LINE_TOKEN  "xxxx"
#define OTA_PASS "admin"

#include "credentials.h"  // Remove this line.


const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;


// Blynk
char auth[] = Blynk_KEY;



// Virtual Pin Blynk
#define vButtonFanToilet V9
#define vButtonFanBedroom V10
#define vAutoFan V4   // Auto check Temp & Humidi control Fan.

#define vToiletTemperature V20
#define vToiletHumidity V21
#define vOutsideTemperature V22
#define vOutsideHumidity V23



// PINS
#define RelayFanToilet 13 //15
#define RelayFanBedroom 12 //2

#define ButtonFanToilet 4
#define ButtonFanBedroom 5

#define DHT11Toilet 2 //12
#define DHT11Outside 16 //13


#define TempUnder 29
#define TemperatureDiff 2.0
#define HumidityDiff 20.0

#define OpenFanMinus 30
#define CheckTempHumiSec 10   // Second


int AutoFan=0, timerId_fanTL, timerId_fanBR; 
int AutoTuneOffTL=1, AutoTuneOffBR=1;
float hToilet=0, hOutside=0, tToilet=0, tOutside=0;

AceButton btToilet(ButtonFanToilet,HIGH,1);
AceButton btBedroom(ButtonFanBedroom,HIGH,2);


BLYNK_WRITE(vAutoFan){
  AutoFan = param.asInt();
}
BLYNK_WRITE(vButtonFanToilet){
  int vRead = param.asInt();
  digitalWrite(RelayFanToilet, vRead);
}
BLYNK_WRITE(vButtonFanBedroom){
  int vRead = param.asInt();
  digitalWrite(RelayFanBedroom, vRead);
}


void FanToiletTuneOffEvent() {
  if (AutoTuneOffTL==1) FanToiletTuneOff();
}
void FanBedroomTuneOffEvent() {
  if (AutoTuneOffBR==1) FanBedroomTuneOff();
}

void FanToiletTuneOff() {
Serial.println("Toilet fan tune off");
  digitalWrite(RelayFanToilet, HIGH);
  Blynk.virtualWrite(vButtonFanToilet, HIGH);
  timer.disable(timerId_fanTL);
  AutoTuneOffTL=1;
}
void FanBedroomTuneOff() {
Serial.println("Bedroom fan tune off");
  digitalWrite(RelayFanBedroom, HIGH);
  Blynk.virtualWrite(vButtonFanBedroom, HIGH);
  timer.disable(timerId_fanBR);
  AutoTuneOffBR=1;
}


void FanToiletTuneOn() {
Serial.println("Toilet fan tune on");
  digitalWrite(RelayFanToilet, LOW);
  Blynk.virtualWrite(vButtonFanToilet, LOW);
  timer.restartTimer(timerId_fanTL);
  timer.enable(timerId_fanTL);
}
void FanBedroomTuneOn() {
Serial.println("Bedroom fan tune on");
  digitalWrite(RelayFanBedroom, LOW);
  Blynk.virtualWrite(vButtonFanBedroom, LOW);
  timer.restartTimer(timerId_fanBR);
  timer.enable(timerId_fanBR);
}

void checkFan() {
  if (AutoFan==0) return;
  
  if (hToilet==0 || hOutside==0) {
    return; // Sensor DHT error not check.
  }
  if (tToilet <= TempUnder) return;
  if (((tToilet-tOutside) >= TemperatureDiff) || ((hToilet-hOutside) >= HumidityDiff)) {
    FanToiletTuneOn();
  } else {
    FanToiletTuneOff();
  }
}

void sendSensor()
{
  Serial.println("Sensor check.");
  hToilet = dhtToilet.getHumidity();    //v 20 21
  tToilet = dhtToilet.getTemperature(); // or dhtToilet.readTemperature(true) for Fahrenheit
  if (isnan(hToilet) || isnan(tToilet)) {
    Serial.println("Failed to read from DHT Toilet sensor!");
    hToilet=0;
    tToilet=0;
  } else {
    Blynk.virtualWrite(vToiletTemperature, tToilet);
    Blynk.virtualWrite(vToiletHumidity, hToilet);
  }

  hOutside = dhtOutside.getHumidity();
  tOutside = dhtOutside.getTemperature();
  if (isnan(hOutside) || isnan(tOutside)) {
    Serial.println("Failed to read from DHT Outside sensor!");
    hOutside=0;
    tOutside=0;
  } else {
    Blynk.virtualWrite(vOutsideTemperature, tOutside);
    Blynk.virtualWrite(vOutsideHumidity, hOutside);
  }

  checkFan();
}


void handleButtonEvent(AceButton* button, uint8_t eventType,
    uint8_t /* buttonState */) {

  uint8_t b_id = button->getId();
  int d;

  switch (eventType) {
    case AceButton::kEventLongPressed:
Serial.println("Long Pressed");    
      switch (b_id) {
        case 1:
          AutoTuneOffTL=0;
          break;
        case 2:
          AutoTuneOffBR=0;
          break;
      }
      //break;    
    case AceButton::kEventClicked:
      switch (b_id) { 
        case 1:  // Toilet
          d = digitalRead(RelayFanToilet);
          if (d==LOW) {  // Relay Runnig
            FanToiletTuneOff();
          } else {
            FanToiletTuneOn();
          }
          Blynk.virtualWrite(vButtonFanToilet, d);
          break;
        case 2:  // Bedroom
          d = digitalRead(RelayFanBedroom);
          if (d==LOW) {  // Relay Runnig
            FanBedroomTuneOff();
          } else {
            FanBedroomTuneOn();
          }
          Blynk.virtualWrite(vButtonFanBedroom, d);
          break;
      }
      break;
  }
}




void SetupOTA() {
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  //while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //  Serial.println("Connection Failed! Rebooting...");
  //  delay(5000);
  //  ESP.restart();
  //}
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("ESP8266-Fan");

  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASS);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setup() {
  int timer_id;

  Serial.begin(115200);
  Serial.println("Booting");
  SetupOTA();


  Blynk.begin(auth, ssid, password);
  //Blynk.begin(auth, ssid, password, "chng.ddns.net", 9443);
  
  Serial.println("Blynk Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  dhtToilet.setup(DHT11Toilet, DHTesp::DHT11);
  dhtOutside.setup(DHT11Outside, DHTesp::DHT11);

  timer_id = timer.setInterval(CheckTempHumiSec*1000L, sendSensor);

  timerId_fanTL = timer.setInterval(OpenFanMinus*60000, FanToiletTuneOffEvent);
  timer.disable(timerId_fanTL);

  timerId_fanBR = timer.setInterval(OpenFanMinus*60000, FanBedroomTuneOffEvent);
  timer.disable(timerId_fanBR);

  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token Line Notify
  //LINE.notify("bbbbbbbbbbbbbbb");     // ตัวอย่างส่งข้อความ

  pinMode(RelayFanToilet,OUTPUT);
  digitalWrite(RelayFanToilet,HIGH);
  pinMode(RelayFanBedroom,OUTPUT);
  digitalWrite(RelayFanBedroom,HIGH);

  pinMode(ButtonFanToilet, INPUT_PULLUP);
  pinMode(ButtonFanBedroom, INPUT_PULLUP);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleButtonEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);

}

void loop() {
  ArduinoOTA.handle();
  Blynk.run();
  timer.run();
  btToilet.check();
  btBedroom.check();
}
