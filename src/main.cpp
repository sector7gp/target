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
String global_last_ir = "Esperando...";
bool isHit = false;
unsigned long hitTime = 0;
WebServer* global_server = nullptr;

// Animaciones
void animationReset();
void animationHit(CRGB pColor);

void handleRoot() {
    if (!global_server) return;
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif; background:#121212; color:white; text-align:center; padding:20px;}";
    html += ".card{background:#1e1e1e; border-radius:15px; padding:20px; margin:10px auto; max-width:400px; border:1px solid #333;}";
    html += "h1{color:#00e676;} .val{font-size:1.5em; color:#ffeb3b; font-weight:bold;}";
    html += ".btn{background:#2196F3; color:white; border:none; padding:15px 30px; border-radius:30px; font-size:1.1em; cursor:pointer; margin-top:20px; width:100%; display:inline-block; text-decoration:none;}";
    html += ".cfg-link{color:#888; text-decoration:none; display:block; margin-top:30px; font-size:0.9em;}</style>";
    html += "<script>setInterval(function(){ location.reload(); }, 2000);</script></head><body>";
    
    html += "<h1>BuzzLY Monitor</h1>";
    html += "<div class='card'><h3>Último Jugador</h3><div class='val'>" + global_last_ir + "</div></div>";
    html += "<div class='card'><h3>Señal WiFi</h3><div class='val'>" + String(WiFi.RSSI()) + " dBm</div></div>";
    
    html += "<div style='max-width:400px; margin:auto;'>";
    html += "<form action='/reset' method='POST'><button class='btn' type='submit'>RESET AZUL</button></form>";
    html += "<a href='/settings' class='cfg-link'>⚙️ Configuración</a>";
    html += "</div></body></html>";
    global_server->send(200, "text/html", html);
}

void handleSettings() {
    String html = "<html><body style='background:#121212; color:white; font-family:sans-serif; text-align:center;'>";
    html += "<h1>Configuración</h1><form action='/save' method='POST' style='background:#1e1e1e; padding:30px; border-radius:15px; display:inline-block;'>";
    html += "API URL:<br><input type='text' name='api_url' value='" + String(current_api_url) + "' style='width:300px; padding:10px; margin:10px;'><br>";
    html += "<input type='submit' value='GUARDAR' style='padding:10px 20px; background:#00e676; border:none; border-radius:5px; cursor:pointer;'>";
    html += "</form><br><a href='/' style='color:#2196F3;'>Volver</a></body></html>";
    global_server->send(200, "text/html", html);
}

void handleSave() {
    if (global_server->hasArg("api_url")) {
        Preferences prefs;
        prefs.begin("buzzly", false);
        prefs.putString("api_url", global_server->arg("api_url"));
        prefs.end();
        global_server->send(200, "text/plain", "Guardado. Reiniciando...");
        delay(1000);
        ESP.restart();
    }
}

void handleResetWeb() {
    animationReset();
    global_last_ir = "Reset Web";
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

    {
        WiFiManager wm;
        if (!wm.autoConnect("BuzzLY-Setup")) {
            delay(1000);
            ESP.restart();
        }
    }

    Preferences prefs;
    prefs.begin("buzzly", true);
    String url = prefs.getString("api_url", API_ENDPOINT);
    url.toCharArray(current_api_url, 100);
    prefs.end();

    global_server = new WebServer(80);
    global_server->on("/", handleRoot);
    global_server->on("/settings", handleSettings);
    global_server->on("/save", HTTP_POST, handleSave);
    global_server->on("/reset", HTTP_POST, handleResetWeb);
    global_server->begin();

    IrReceiver.begin(IR_RECEIVE_PIN);
    
    // OTA y mDNS
    MDNS.begin(MDNS_HOSTNAME);
    ArduinoOTA.setHostname(MDNS_HOSTNAME);
    ArduinoOTA.setPassword("Buzz987");
    ArduinoOTA.onStart([]() { fill_solid(leds, NUM_LEDS, CRGB::Magenta); FastLED.show(); });
    ArduinoOTA.begin();
    MDNS.addService("http", "tcp", 80);

    Serial.println("App Ready.");
}

void loop() {
    ArduinoOTA.handle();
    if (global_server) global_server->handleClient();

    if (IrReceiver.decode()) {
        uint32_t code = IrReceiver.decodedIRData.decodedRawData;
        
        bool validAction = false;
        if (code == CODE_RESET) {
            global_last_ir = "Reset Control";
            animationReset();
            validAction = true;
        } else {
            for(int i=0; i<6; i++) {
                if(code == players[i].code) {
                    global_last_ir = players[i].name;
                    animationHit(players[i].color);
                    isHit = true;
                    hitTime = millis();
                    validAction = true;
                    
                    // Aquí podrías llamar a sendHitToAPI(i+1)
                    break;
                }
            }
        }

        if (!validAction) {
            Serial.println("Código descartado: 0x" + String(code, HEX));
        }
        
        IrReceiver.resume();
    }

    if (isHit && (millis() - hitTime > 1500)) isHit = false;

    if (!isHit) {
        uint8_t br = beatsin8(15, 30, 150);
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        FastLED.setBrightness(br);
        FastLED.show();
    }
    delay(5);
}

void animationHit(CRGB pColor) {
    FastLED.setBrightness(255);
    fill_solid(leds, NUM_LEDS, pColor);
    FastLED.show();
}

void animationReset() {
    for(int i=0; i<4; i++) {
        fill_solid(leds, NUM_LEDS, CRGB::Blue);
        FastLED.show(); delay(150);
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show(); delay(150);
    }
}
