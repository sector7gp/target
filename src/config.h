#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <FastLED.h>

// --- Configuración de Red ---
#define API_ENDPOINT "http://tuservidor.com/api/hit"
#define MDNS_HOSTNAME "buzzly-target"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_TOPIC_HIT "target/hit"
#define MQTT_TOPIC_RESET "target/reset"

// --- Configuración de Hardware ---
#define LED_PIN 3
#define MAX_LEDS 30
#define IR_RECEIVE_PIN 4

// --- Códigos IR (Control 24 botones NEC) ---
#define CODE_PLAYER1 0xE51AFF00 // Rojo
#define CODE_PLAYER2 0x659AFF00 // Verde
#define CODE_PLAYER3 0x5DA2FF00 // Azul
#define CODE_PLAYER4 0x1DE2FF00 // Amarillo
#define CODE_PLAYER5 0xB748FF00 // Magenta
#define CODE_PLAYER6 0x758AFF00 // Cyan
#define CODE_RESET 0xBF40FF00   // Botón OFF

// --- Estructura para mapear colores a jugadores ---
struct Player {
  uint32_t code;
  CRGB color;
  const char *name;
};

const Player players[] = {{CODE_PLAYER1, CRGB::Red, "Jugador 1"},
                          {CODE_PLAYER2, CRGB::Green, "Jugador 2"},
                          {CODE_PLAYER3, CRGB::Blue, "Jugador 3"},
                          {CODE_PLAYER4, CRGB::Orange, "Jugador 4"},
                          {CODE_PLAYER5, CRGB::Magenta, "Jugador 5"},
                          {CODE_PLAYER6, CRGB::Cyan, "Jugador 6"}};

#endif
