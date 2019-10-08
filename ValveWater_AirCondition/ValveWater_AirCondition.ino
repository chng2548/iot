//#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <TridentTD_LineNotify.h>


#include "DHTesp.h"
DHTesp dht;
BlynkTimer timer;



#define WIFI_SSID "xxxxx"
#define WIFI_PASSWD "xxxx"
#define Blynk_KEY "xxxxxx"
#define LINE_TOKEN  "xxxxxxxxx"
#define OTA_PASS "admin"

#include "credentials.h"  // *** Remove this line

// Wifi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWD;

// Line
const char* lineToken = LINE_TOKEN;

// Blynk
const char* auth = Blynk_KEY;



// Pin
#define RELAY_PIN 4  // GPIO4
#define DHT_PIN 5  // GPIO5
#define ButtonRelayVPIN V2  // Virtual PIN on app.
#define OpenRelayMinus 15  // minus

int timerId_relay, timerId_temp;

void sendSensor()
{
#ifdef BLYNK_PRINT
  digitalWrite(2,!digitalRead(2));
#endif

  float h = dht.getHumidity();
  float t = dht.getTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

#ifdef BLYNK_PRINT
  Serial.print(dht.getStatusString());
  Serial.print("\t");
  Serial.print(h, 1);
  Serial.print("\t\t");
  Serial.print(t, 1);
  Serial.println("\t\t");
#endif
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
  ArduinoOTA.setHostname("ESP8266-ValeWater");

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

BLYNK_WRITE(ButtonRelayVPIN) {
  if (timer.isEnabled(timerId_relay)) {
    timer.disable(timerId_relay);
  }
  int vstatus = param.asInt();
  digitalWrite(RELAY_PIN, vstatus);

  if (vstatus) { // v2 HIGH
    timer.restartTimer(timerId_relay);
    timer.enable(timerId_relay);
  }
}

void relayTuneOff() {
  timer.disable(timerId_relay);  
  digitalWrite(RELAY_PIN, HIGH);
  Blynk.virtualWrite(ButtonRelayVPIN, 0);
}


void setup() {
#ifdef BLYNK_PRINT
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
#endif

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  Serial.println("Booting");
  SetupOTA();
/*
  Blynk.begin(auth, ssid, password);
  Serial.println("Blynk Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
*/
  dht.setup(DHT_PIN, DHTesp::DHT11);
  timerId_temp = timer.setInterval(15000L, sendSensor);
  timerId_relay = timer.setInterval(OpenRelayMinus*60000, relayTuneOff);
  timer.disable(timerId_relay);

  //LINE.setToken(lineToken);  // กำหนด Line Token Line Notify
  //LINE.notify("bbbbbbbbbbbbbbb");     // ตัวอย่างส่งข้อความ

}

void loop() {
  if (WiFi.status() != WL_CONNECTED)  {
    Blynk.begin(auth, ssid, password);
    Serial.println("Blynk Ready");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  ArduinoOTA.handle();
  Blynk.run();
  timer.run();
}
