#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID      "Ganesh"
#define WIFI_PASSWORD  "123456783"
#define DATABASE_URL   "https://soilsense-iot-d1c31-default-rtdb.firebaseio.com"
#define API_KEY        "AIzaSyDo4jyrbgxz7x585UEL9wKcfJQzWBwSPbM"

#define SOIL_PIN       34
#define DHT_PIN        4
#define DHT_TYPE       DHT11
#define LED_PIN        26
#define BUZZER_PIN     15
#define RELAY_PIN      23

#define DRY_VAL        4095
#define WET_VAL        1500
#define DRY_THRESHOLD  30
#define WET_STOP       60

#define SEND_INTERVAL     5000
#define BUZZ_INTERVAL    30000
#define BLINK_INTERVAL    5000
#define PUMP_MAX_MS      10000
#define PUMP_COOLDOWN_MS 60000
#define IST_OFFSET       19800

FirebaseData   fbData;
FirebaseConfig fbConfig;
FirebaseAuth   fbAuth;
DHT dht(DHT_PIN, DHT_TYPE);
WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", IST_OFFSET, 60000);

unsigned long lastSend      = 0;
unsigned long lastBuzz      = 0;
unsigned long lastBlink     = 0;
unsigned long pumpStartedAt = 0;
unsigned long lastPumpStop  = 0;
bool          isDry         = false;
bool          pumpRunning   = false;
int           count         = 0;

String getSoilStatus(int p){ if(p<30)return"Dry"; if(p<60)return"Moist"; return"Wet"; }
String getTempStatus(float t){ if(t<10)return"Cold"; if(t<25)return"Normal"; if(t<35)return"Warm"; return"Hot"; }
String getHumStatus(float h){ if(h<30)return"Low"; if(h<60)return"Normal"; return"High"; }

void beep(int times, int onMs, int offMs){
  for(int i=0;i<times;i++){
    digitalWrite(BUZZER_PIN,HIGH); delay(onMs);
    digitalWrite(BUZZER_PIN,LOW);  if(offMs>0)delay(offMs);
  }
}
void blinkLED(int times, int ms){
  for(int i=0;i<times;i++){
    digitalWrite(LED_PIN,HIGH); delay(ms);
    digitalWrite(LED_PIN,LOW);  delay(ms);
  }
}

// ── Pump ON — simple relay switch, NO blocking delay ─────────
void pumpON(){
  digitalWrite(RELAY_PIN, LOW);  // active LOW = pump ON
  pumpRunning   = true;
  pumpStartedAt = millis();
  Serial.println("PUMP ON — started");
}

// ── Pump OFF ─────────────────────────────────────────────────
void pumpOFF(const char* reason){
  digitalWrite(RELAY_PIN, HIGH); // pump OFF
  pumpRunning  = false;
  lastPumpStop = millis();
  Serial.printf("PUMP OFF — %s\n", reason);
  beep(2, 100, 80);
}

void setup(){
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout

  Serial.begin(115200);
  delay(500);

  pinMode(LED_PIN,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN,  OUTPUT);

  // Safety: ensure pump OFF at boot
  digitalWrite(RELAY_PIN,  HIGH);
  digitalWrite(LED_PIN,    LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.println("SoilSense Pro ESP32 v2.9");
  Serial.println("Brownout DISABLED");

  blinkLED(3, 200);
  delay(200);
  beep(2, 200, 150);
  Serial.println("Startup test OK!");

  dht.begin();
  Serial.println("[1/4] DHT11 Ready on GPIO4");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[2/4] Connecting WiFi");
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.println("\n      IP: "+WiFi.localIP().toString());

  fbConfig.api_key               = API_KEY;
  fbConfig.database_url          = DATABASE_URL;
  fbConfig.token_status_callback = tokenStatusCallback;
  Firebase.signUp(&fbConfig, &fbAuth, "", "");
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);
  Serial.println("[3/4] Firebase Ready!");

  timeClient.begin();
  timeClient.update();
  Serial.println("[4/4] NTP: "+timeClient.getFormattedTime());
  Serial.println("All systems GO! v2.9\n");
  beep(1, 600, 0);
}

void loop(){
  unsigned long now = millis();

  // ── PUMP SAFETY TIMEOUT (non-blocking check) ─────────────
  // Runs every loop iteration — turns pump off if max time exceeded
  if(pumpRunning && (now - pumpStartedAt >= PUMP_MAX_MS)){
    pumpOFF("safety timeout");
  }

  // ── HEARTBEAT LED ─────────────────────────────────────────
  if(!isDry && !pumpRunning){
    if(now - lastBlink >= BLINK_INTERVAL){
      lastBlink = now;
      digitalWrite(LED_PIN, HIGH); delay(80); digitalWrite(LED_PIN, LOW);
    }
  }

  // ── MAIN SENSOR + FIREBASE LOOP ──────────────────────────
  if(now - lastSend >= SEND_INTERVAL){
    lastSend = now;

    if(!Firebase.ready()){
      Serial.println("Firebase not ready — skipping...");
      return;
    }

    timeClient.update();

    // Read soil (average 5)
    int total=0;
    for(int i=0;i<5;i++){ total+=analogRead(SOIL_PIN); delay(10); }
    int soilRaw = total/5;
    int soilPct = constrain(map(soilRaw, DRY_VAL, WET_VAL, 0, 100), 0, 100);

    // Read DHT11
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    if(isnan(temp)||isnan(hum)){ temp=0; hum=0; }
    float hi = (temp>0&&hum>0) ? dht.computeHeatIndex(temp,hum,false) : 0;

    // Soil status
    bool wasD = isDry;
    isDry = (soilPct < DRY_THRESHOLD);

    // ── PUMP DECISIONS (flags only, no blocking) ──────────
    if(isDry && !pumpRunning){
      bool cooledDown = (lastPumpStop==0) || (now-lastPumpStop >= PUMP_COOLDOWN_MS);
      if(cooledDown){
        Serial.println("Soil dry — turning pump ON");
        pumpON(); // non-blocking — just sets pin HIGH/LOW and returns
      } else {
        unsigned long rem = (PUMP_COOLDOWN_MS-(now-lastPumpStop))/1000;
        Serial.printf("Pump cooldown — %lu sec left\n", rem);
      }
    }

    // Turn pump OFF when soil is wet enough
    if(pumpRunning && soilPct >= WET_STOP){
      Serial.println("Soil wet enough — turning pump OFF");
      pumpOFF("soil reached WET_STOP");
    }

    // ── LED + BUZZER ──────────────────────────────────────
    if(isDry){
      digitalWrite(LED_PIN, HIGH);
      if(now-lastBuzz >= BUZZ_INTERVAL){
        lastBuzz=now;
        Serial.println("Dry alert buzz!");
        beep(3,400,200);
        blinkLED(3,150);
        digitalWrite(LED_PIN,HIGH);
      }
    } else {
      if(wasD){
        Serial.println("Soil OK — alert cleared");
        beep(1,500,0); blinkLED(2,300);
      }
      if(!pumpRunning){
        digitalWrite(LED_PIN,    LOW);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }

    // ── TIMESTAMP ─────────────────────────────────────────
    time_t epoch = timeClient.getEpochTime();
    struct tm *t = gmtime(&epoch);
    char ts[25];
    snprintf(ts,sizeof(ts),"%04d-%02d-%02dT%02d:%02d:%02d",
      t->tm_year+1900,t->tm_mon+1,t->tm_mday,
      t->tm_hour,t->tm_min,t->tm_sec);

    // ── SERIAL PRINT ──────────────────────────────────────
    Serial.println("─────────────────────────────────────");
    Serial.printf("Reading #%d | %s\n", ++count, ts);
    Serial.printf("Soil : %d%%  Raw:%d  [%s]\n", soilPct, soilRaw, getSoilStatus(soilPct).c_str());
    Serial.printf("Temp : %.1fC  Hum:%.1f%%  HI:%.1fC\n", temp, hum, hi);
    Serial.printf("Pump : %s\n", pumpRunning?"ON":"OFF");
    Serial.printf("Alert: %s\n", isDry?"DRY ACTIVE":"Normal");

    // ── FIREBASE PUSH ─────────────────────────────────────
    String path = "/sensor/moisture/" + String(epoch);
    FirebaseJson json;
    json.set("value",        soilPct);
    json.set("raw",          soilRaw);
    json.set("soil_status",  getSoilStatus(soilPct).c_str());
    json.set("temperature",  temp);
    json.set("humidity",     hum);
    json.set("heat_index",   hi);
    
    json.set("temp_status",  getTempStatus(temp).c_str());
    json.set("hum_status",   getHumStatus(hum).c_str());
    json.set("alert",        isDry);
    json.set("pump_running", pumpRunning);
    json.set("timestamp",    ts);

    if(Firebase.RTDB.setJSON(&fbData, path.c_str(), &json))
      Serial.println("Firebase: Sent ✓");
    else
      Serial.println("Firebase error: "+fbData.errorReason());

    Serial.println("─────────────────────────────────────\n");
  }
}
