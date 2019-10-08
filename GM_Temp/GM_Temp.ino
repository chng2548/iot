#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>
#include <NTPClient.h>
//#include <ESP8266WiFi.h>
//#include <WiFiUdp.h>
#include "DHTesp.h"



#define WIFI_SSID "xx"
#define WIFI_PASSWD "xx"
#define Blynk_KEY "xxx"
#define LINE_TOKEN  "xxxx"
#define OTA_PASS "admin"

#include "credentials.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;


// Blynk GM Project
char auth[] = Blynk_KEY;

// Pin
#define DHT_PIN 5  // GPIO5
const short int BUILTIN_LED1 = 2; //GPIO2

//
unsigned long LongNotify = 3600000*4; // Interval Notify 4house
unsigned long NextNotify=0, NextDHTErrNotify=0;


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200);  // +7

DHTesp dht;
BlynkTimer timer;

void SetupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("ESP8266-GMTemperature");

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


void CheckTemp() {
  float h = dht.getHumidity();
  float t = dht.getTemperature(); // or dht.readTemperature(true) for Fahrenheit
  long timecal;

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    if (millis() > NextDHTErrNotify) {
      LINE.notify("อ่านค่าอุหภูมิเกิดความผิดผลาด");
      NextDHTErrNotify = NextDHTErrNotify + 3000000*3;  // 50 min * 3
    }
    ledblink();
    return;
  }
  h = h + 20;
  t = t + 3;
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  if (timeClient.getDay() != 0) { // not Sunday
    timecal = timeClient.getHours() * 60 + timeClient.getMinutes();
//7:20 - 18:50
    if (timecal > (7*60+20) && timecal < (18*60+50)) {
      // Woke time.
      if (t >= 28) {
        if (millis() > NextNotify) {
          LINE.notify("ห้อง Server อุณหภูมิสูงกว่าปกติ " + String(t,2));
          NextNotify = NextNotify + LongNotify;
        }
      }
// 0:00-6:30   19:30-23:59
    } 
    if ((timecal > 0 && timecal < (6*60+30)) || (timecal > (19*60+30) && timecal < (23*60+59))) {
      // not work time.
      if (t <= 25) {
        if (millis() > NextNotify) {
          LINE.notify("ห้อง Server อาจจะยังไม่ปิดแอร์ อุณหภูมิ:" + String(t,2));
          NextNotify = NextNotify + LongNotify;
        }
      }
    }
  }
}



void WifiConnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void NTPSet() {
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  Serial.println("NTP Time set.");
  Serial.println(timeClient.getFormattedTime());
}

void ledblink() {
  digitalWrite(BUILTIN_LED1,LOW);
  delay(100);
  digitalWrite(BUILTIN_LED1,HIGH);
  delay(300);
  digitalWrite(BUILTIN_LED1,LOW);
  delay(100);
  digitalWrite(BUILTIN_LED1,HIGH);
  delay(300);
  digitalWrite(BUILTIN_LED1,LOW);
  delay(100);
  digitalWrite(BUILTIN_LED1,HIGH);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  SetupOTA();
  dht.setup(DHT_PIN, DHTesp::DHT11);

  LINE.setToken(LINE_TOKEN);  // กำหนด Line Token Line Notify
  //LINE.notify("bbbbbbbbbbbbbbb");     // ตัวอย่างส่งข้อความ

  long timerId_temp = timer.setInterval(5000L, CheckTemp); //5sec.
  timer.enable(timerId_temp);
  long timerId_NTP = timer.setInterval(86400000L, NTPSet); //5sec.
  timer.enable(timerId_NTP);

  Blynk.begin(auth, ssid, password);
  // Initialize a NTPClient to get time
  timeClient.begin();
  NTPSet();

  pinMode(BUILTIN_LED1, OUTPUT); // Initialize the BUILTIN_LED1 pin as an output
}


void loop() {
  WifiConnect();
  ArduinoOTA.handle();
  Blynk.run();
  timer.run();
}
