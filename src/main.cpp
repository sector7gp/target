#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <FastLED.h>
#include <HTTPClient.h>
#include <IRremote.hpp>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "config.h"

// --- Globales ---
CRGB leds[MAX_LEDS];
char current_mdns_name[30];
char current_mqtt_broker[50];
char current_mqtt_topic_hit[50];
char current_mqtt_topic_reset[50];
int current_num_leds = 5;

String global_last_ir = "Esperando...";
bool isHit = false;
WebServer *global_server = nullptr;
WiFiManager wm;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool services_started = false;
unsigned long lastMqttRetry = 0;

// --- Prototipos ---
void animationReset();
void reportAction(int playerID, bool isReset);

// --- CSS Premium ---
String getStyle() {
  String s = "<style>";
  s += "body{font-family:'Segoe UI',Roboto,Helvetica,Arial,sans-serif; background:#08080a; color:#f0f0f0; margin:0; padding:20px; display:flex; flex-direction:column; align-items:center; min-height:100vh;}";
  s += ".container{width:100%; max-width:440px; background:rgba(255,255,255,0.02); backdrop-filter:blur(20px); border-radius:35px; padding:35px; box-shadow:0 30px 60px rgba(0,0,0,0.8); border:1px solid rgba(255,255,255,0.1); box-sizing:border-box;}";
  s += "h1{font-size:34px; font-weight:900; margin-bottom:30px; background:linear-gradient(90deg, #bfff00 0%, #00ff00 100%); -webkit-background-clip:text; -webkit-text-fill-color:transparent; text-align:center; letter-spacing:-1.5px;}";
  s += ".card{background:rgba(255,255,255,0.05); border-radius:24px; padding:22px; margin-bottom:20px; border-left:6px solid #bfff00; box-shadow:5px 5px 15px rgba(0,0,0,0.3);}";
  s += ".label{font-size:10px; text-transform:uppercase; letter-spacing:3px; color:#777; margin-bottom:12px; font-weight:800;}";
  s += ".value{font-size:24px; font-weight:800; color:#fff;}";
  s += ".btn{display:block; width:100%; padding:22px; border-radius:22px; border:none; font-size:18px; font-weight:900; cursor:pointer; transition:all 0.4s cubic-bezier(0.175, 0.885, 0.32, 1.275); text-transform:uppercase; letter-spacing:2px; margin-top:20px; text-align:center; text-decoration:none; box-sizing:border-box;}";
  s += ".btn-primary{background:linear-gradient(135deg, #bfff00 0%, #00ff00 100%); color:#000; box-shadow:0 15px 30px rgba(191,255,0,0.35);}";
  s += ".btn-primary:hover{transform:scale(1.02); box-shadow:0 20px 40px rgba(191,255,0,0.5);}";
  s += ".btn-outline{background:rgba(255,255,255,0.08); border:2px solid rgba(255,255,255,0.1); color:#ccc; font-size:14px;}";
  s += ".btn-outline:hover{background:rgba(255,255,255,0.15); color:#fff; border-color:#bfff00; transform:scale(1.01);}";
  s += "form{width:100%;} input{width:100%; box-sizing:border-box; background:rgba(0,0,0,0.5); border:2px solid #222; border-radius:18px; padding:18px; color:#fff; margin-bottom:20px; font-size:18px;}";
  s += "input:focus{outline:none; border-color:#bfff00; background:rgba(0,0,0,0.7);}";
  s += ".section-title{font-size:14px; font-weight:900; color:#bfff00; margin:40px 0 25px 0; border-bottom:2px solid #222; padding-bottom:10px; letter-spacing:1.5px;}";
  s += ".status-dot{display:inline-block; width:14px; height:14px; border-radius:50%; margin-right:12px; vertical-align:middle;}";
  s += ".status-online{background:#bfff00; box-shadow:0 0 20px #bfff00;}";
  s += ".status-offline{background:#ff2d00; box-shadow:0 0 20px #ff2d00;}";
  s += "</style>";
  return s;
}

String getHeader(String title) {
  String h = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  h += "<title>" + title + " | BuzzLY</title>";
  h += getStyle();
  h += "</head><body><div class='container'>";
  return h;
}

// --- Callback MQTT ---
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (String(topic) == current_mqtt_topic_reset) {
    animationReset();
    global_last_ir = "MQTT Reset";
  }
}

// --- Comunicación ---
void reportAction(int playerID, bool isReset) {
  if (mqttClient.connected()) {
    String targetName = String(current_mdns_name);
    String msg;
    if (isReset) {
      msg = "{\"target\":\"" + targetName + "\", \"action\":\"reset\"}";
    } else {
      msg = "{\"target\":\"" + targetName + "\", \"player_id\":" + String(playerID) + ", \"action\":\"hit\"}";
    }
    mqttClient.publish(isReset ? current_mqtt_topic_reset : current_mqtt_topic_hit, msg.c_str());
  }
}

// --- Animaciones ---
void animationHit(CRGB pColor) {
  for (int i = 0; i < 15; i++) {
    fill_solid(leds, current_num_leds, pColor);
    for (int j = 0; j < 3; j++) {
      int pixel = random(0, current_num_leds);
      leds[pixel] = (random(0, 2) == 0) ? CRGB::White : CRGB::Yellow;
    }
    FastLED.setBrightness(255);
    FastLED.show();
    delay(30);
  }
  fill_solid(leds, current_num_leds, pColor);
  FastLED.show();
}

void animationReset() {
  for (int i = 0; i < 3; i++) {
    fill_solid(leds, current_num_leds, CRGB::Blue);
    FastLED.show(); delay(80);
    fill_solid(leds, current_num_leds, CRGB::Black);
    FastLED.show(); delay(80);
  }
  isHit = false;
}

// --- Web Handlers ---
void handleRoot() {
  String html = getHeader("Monitor");
  html += "<h1>BuzzLY Pro</h1>";
  html += "<div class='card'><div class='label'>Status Sistema</div><div class='value'><span class='status-dot status-online'></span>Operativo</div></div>";
  html += "<div class='card'><div class='label'>Ultimo Impacto</div><div class='value' id='last_ir'>" + global_last_ir + "</div></div>";
  html += "<div class='card'><div class='label'>MQTT Link</div>";
  String mqttStatus = mqttClient.connected() ? "status-online" : "status-offline";
  String mqttText = mqttClient.connected() ? "Conectado" : "Desconectado";
  html += "<div class='value'><span class='status-dot " + mqttStatus + "'></span>" + mqttText + "</div></div>";
  html += "<form action='/reset' method='POST'><button class='btn btn-primary' type='submit'>Reset Blanco</button></form>";
  html += "<a href='/settings' class='btn btn-outline'>Configuracion Avanzada</a>";
  html += "</div><script>setInterval(function(){fetch('/').then(r=>r.text()).then(h=>{document.body.innerHTML=new DOMParser().parseFromString(h,'text/html').body.innerHTML;});},3000);</script></body></html>";
  global_server->send(200, "text/html", html);
}

void handleSettings() {
  String html = getHeader("Ajustes");
  html += "<h1>Setup</h1>";
  html += "<form action='/save' method='POST'>";
  html += "<div class='section-title'>PARAMETROS DE HARDWARE</div>";
  html += "<div class='label'>Nombre mDNS</div><input type='text' name='mdns_name' value='" + String(current_mdns_name) + "'>";
  html += "<div class='label'>Cantidad de LEDs</div><input type='number' name='num_leds' value='" + String(current_num_leds) + "'>";
  html += "<div class='section-title'>COMMUNICATION (MQTT)</div>";
  html += "<div class='label'>Broker Server</div><input type='text' name='mqtt_broker' value='" + String(current_mqtt_broker) + "'>";
  html += "<div class='label'>Topic Hit</div><input type='text' name='topic_hit' value='" + String(current_mqtt_topic_hit) + "'>";
  html += "<div class='label'>Topic Reset</div><input type='text' name='topic_reset' value='" + String(current_mqtt_topic_reset) + "'>";
  html += "<button class='btn btn-primary' type='submit'>Aplicar y Reiniciar</button>";
  html += "</form><a href='/' class='btn btn-outline'>Volver al Monitor</a></div></body></html>";
  global_server->send(200, "text/html", html);
}

void handleSave() {
  Preferences prefs;
  prefs.begin("buzzly", false);
  if (global_server->hasArg("mdns_name")) prefs.putString("mdns_name", global_server->arg("mdns_name"));
  if (global_server->hasArg("num_leds")) prefs.putInt("num_leds", global_server->arg("num_leds").toInt());
  if (global_server->hasArg("mqtt_broker")) prefs.putString("mqtt_broker", global_server->arg("mqtt_broker"));
  if (global_server->hasArg("topic_hit")) prefs.putString("topic_hit", global_server->arg("topic_hit"));
  if (global_server->hasArg("topic_reset")) prefs.putString("topic_reset", global_server->arg("topic_reset"));
  prefs.end();
  global_server->send(200, "text/plain", "OK. Rebooting...");
  delay(1000);
  ESP.restart();
}

void handleResetWeb() {
  animationReset();
  reportAction(0, true);
  global_last_ir = "Web Reset";
  global_server->sendHeader("Location", "/");
  global_server->send(303);
}

void reconnectMQTT() {
  if (millis() - lastMqttRetry > 5000) {
    lastMqttRetry = millis();
    String clientId = "BuzzLY-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      mqttClient.subscribe(current_mqtt_topic_reset);
    }
  }
}

void startNetworkServices() {
  if (services_started) return;
  global_server = new WebServer(80);
  global_server->on("/", handleRoot);
  global_server->on("/settings", handleSettings);
  global_server->on("/save", HTTP_POST, handleSave);
  global_server->on("/reset", HTTP_POST, handleResetWeb);
  global_server->begin();
  MDNS.begin(current_mdns_name);
  ArduinoOTA.setHostname(current_mdns_name);
  ArduinoOTA.setPassword("Buzz987");
  ArduinoOTA.begin();
  mqttClient.setServer(current_mqtt_broker, 1883);
  mqttClient.setCallback(mqttCallback);
  services_started = true;
}

void setup() {
  delay(1000);
  Preferences prefs;
  prefs.begin("buzzly", true);
  current_num_leds = prefs.getInt("num_leds", 5);
  prefs.getString("mdns_name", MDNS_HOSTNAME).toCharArray(current_mdns_name, 30);
  prefs.getString("mqtt_broker", MQTT_BROKER).toCharArray(current_mqtt_broker, 50);
  prefs.getString("topic_hit", MQTT_TOPIC_HIT).toCharArray(current_mqtt_topic_hit, 50);
  prefs.getString("topic_reset", MQTT_TOPIC_RESET).toCharArray(current_mqtt_topic_reset, 50);
  prefs.end();

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, MAX_LEDS);
  FastLED.setBrightness(100);
  fill_solid(leds, current_num_leds, CRGB::Black);
  FastLED.show();
  IrReceiver.begin(IR_RECEIVE_PIN);
  wm.setConfigPortalBlocking(false);
  wm.autoConnect("BuzzLY-Setup");
}

void loop() {
  wm.process();
  if (WiFi.status() == WL_CONNECTED) {
    startNetworkServices();
    ArduinoOTA.handle();
    if (global_server) global_server->handleClient();
    if (!mqttClient.connected()) reconnectMQTT();
    else mqttClient.loop();
  }

  if (IrReceiver.decode()) {
    uint32_t code = IrReceiver.decodedIRData.decodedRawData;
    if (code == CODE_RESET) {
      global_last_ir = "IR Reset";
      animationReset();
      reportAction(0, true);
    } else if (!isHit) {
      for (int i = 0; i < 6; i++) {
        if (code == players[i].code) {
          global_last_ir = players[i].name;
          animationHit(players[i].color);
          isHit = true;
          reportAction(i + 1, false);
          break;
        }
      }
    }
    IrReceiver.resume();
  }

  if (!isHit) {
    uint8_t br = beatsin8(15, 30, 150);
    fill_solid(leds, current_num_leds, CRGB::Green);
    FastLED.setBrightness(br);
    FastLED.show();
  }
  delay(5);
}
