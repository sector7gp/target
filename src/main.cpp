#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <IRremote.hpp>
#include <FastLED.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>

#include "config.h"

// --- Globales ---
CRGB leds[NUM_LEDS];
char current_api_url[100];
char current_mdns_name[30];
String global_last_ir = "Esperando impacto...";
bool isHit = false;
unsigned long hitTime = 0;
WebServer* global_server = nullptr;

// --- Funciones de API ---
void sendHitToAPI(int playerID) {
    if (WiFi.status() == WL_CONNECTED && strlen(current_api_url) > 5) {
        HTTPClient http;
        http.begin(current_api_url);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"player_id\":" + String(playerID) + ", \"action\":\"hit\"}";
        http.POST(json);
        http.end();
    }
}

void sendResetToAPI() {
    if (WiFi.status() == WL_CONNECTED && strlen(current_api_url) > 5) {
        HTTPClient http;
        http.begin(current_api_url);
        http.addHeader("Content-Type", "application/json");
        String json = "{\"action\":\"reset\"}";
        http.POST(json);
        http.end();
    }
}

// --- Animaciones ---
void animationHit(CRGB pColor) {
    FastLED.setBrightness(255);
    fill_solid(leds, NUM_LEDS, pColor);
    FastLED.show();
}

void animationReset() {
    // Parpadeo azul rápido para indicar que se recibió el comando
    for(int i=0; i<3; i++) {
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        FastLED.show(); delay(80);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show(); delay(80);
    }
    isHit = false; // Forzamos la vuelta al Glow Verde inmediatamente
}

// --- Pantallas Web ---
void handleRoot() {
    if (!global_server) return;
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif; background:#121212; color:white; text-align:center; padding:20px;}";
    html += ".card{background:#1e1e1e; border-radius:15px; padding:20px; margin:10px auto; max-width:400px; border:1px solid #333;}";
    html += "h1{color:#00e676;} .val{font-size:1.5em; color:#ffeb3b; font-weight:bold;}";
    html += ".btn{background:#2196F3; color:white; border:none; padding:15px 30px; border-radius:30px; font-size:1.1em; cursor:pointer; margin-top:20px; width:100%; display:inline-block; text-decoration:none;}";
    html += ".cfg-link{color:#444; text-decoration:none; display:block; margin-top:30px;}</style>";
    html += "<script>setInterval(function(){ location.reload(); }, 2000);</script></head><body>";
    html += "<h1>BuzzLY Pro Monitor</h1>";
    html += "<div class='card'><h3>Último Jugador</h3><div class='val'>" + global_last_ir + "</div></div>";
    html += "<div class='card'><h3>Potencia WiFi</h3><div class='val'>" + String(WiFi.RSSI()) + " dBm</div></div>";
    html += "<div style='max-width:400px; margin:auto;'>";
    html += "<form action='/reset' method='POST'><button class='btn' type='submit'>RESET A VERDE</button></form>";
    html += "<a href='/settings' class='cfg-link'>🔧 Ajustes</a>";
    html += "</div></body></html>";
    global_server->send(200, "text/html", html);
}

void handleSettings() {
    String html = "<html><body style='background:#121212; color:white; font-family:sans-serif; text-align:center; padding:20px;'>";
    html += "<h1>Ajustes</h1><div style='background:#1e1e1e; padding:30px; border-radius:15px; display:inline-block; text-align:left;'>";
    html += "<form action='/save' method='POST'>";
    html += "API URL:<br><input type='text' name='api_url' value='" + String(current_api_url) + "' style='width:300px; padding:10px; margin:10px 0; background:#222; border:1px solid #444; color:white;'><br>";
    html += "Nombre de Red (mDNS):<br><input type='text' name='mdns_name' value='" + String(current_mdns_name) + "' style='width:300px; padding:10px; margin:10px 0; background:#222; border:1px solid #444; color:white;'><br>";
    html += "<input type='submit' value='GUARDAR Y REINICIAR' style='width:100%; padding:15px; background:#00e676; border:none; border-radius:5px; font-weight:bold; cursor:pointer;'>";
    html += "</form></div><br><a href='/' style='color:#2196F3; display:block; margin-top:20px;'>← Cancelar</a></body></html>";
    global_server->send(200, "text/html", html);
}

void handleSave() {
    Preferences prefs;
    prefs.begin("buzzly", false);
    if (global_server->hasArg("api_url")) prefs.putString("api_url", global_server->arg("api_url"));
    if (global_server->hasArg("mdns_name")) prefs.putString("mdns_name", global_server->arg("mdns_name"));
    prefs.end();
    global_server->send(200, "text/plain", "Datos guardados. Reiniciando hardware...");
    delay(1000);
    ESP.restart();
}

void handleResetWeb() {
    animationReset();
    sendResetToAPI();
    global_last_ir = "Reset por Web";
    global_server->sendHeader("Location", "/");
    global_server->send(303);
}

void setup() {
    delay(1000);
    Serial.begin(115200);
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(100);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();

    WiFiManager wm;
    if (!wm.autoConnect("BuzzLY-Setup")) { delay(1000); ESP.restart(); }

    Preferences prefs;
    prefs.begin("buzzly", true);
    String url = prefs.getString("api_url", API_ENDPOINT);
    String mname = prefs.getString("mdns_name", MDNS_HOSTNAME);
    url.toCharArray(current_api_url, 100);
    mname.toCharArray(current_mdns_name, 30);
    prefs.end();

    global_server = new WebServer(80);
    global_server->on("/", handleRoot);
    global_server->on("/settings", handleSettings);
    global_server->on("/save", HTTP_POST, handleSave);
    global_server->on("/reset", HTTP_POST, handleResetWeb);
    global_server->begin();

    IrReceiver.begin(IR_RECEIVE_PIN);
    MDNS.begin(current_mdns_name);
    ArduinoOTA.setHostname(current_mdns_name);
    ArduinoOTA.setPassword("Buzz987");
    ArduinoOTA.begin();
    MDNS.addService("http", "tcp", 80);
}

void loop() {
    ArduinoOTA.handle();
    if (global_server) global_server->handleClient();

    if (IrReceiver.decode()) {
        uint32_t code = IrReceiver.decodedIRData.decodedRawData;
        bool isMatch = false;

        if (code == CODE_RESET) {
            global_last_ir = "Reset por Control";
            animationReset(); // Esto ya pone isHit = false
            sendResetToAPI();
            isMatch = true;
        } else if (!isHit) {
            for(int i=0; i<6; i++) {
                if(code == players[i].code) {
                    global_last_ir = players[i].name;
                    animationHit(players[i].color);
                    isHit = true;
                    // El target queda bloqueado (isHit) infinitamente hasta el reset.
                    sendHitToAPI(i + 1);
                    isMatch = true;
                    break;
                }
            }
        }
        IrReceiver.resume();
    }

    if (!isHit) {
        uint8_t br = beatsin8(15, 30, 150);
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        FastLED.setBrightness(br);
        FastLED.show();
    }
    delay(5);
}
