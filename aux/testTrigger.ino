#include <Arduino.h>
#include <IRremote.hpp>

// --- Configuración de Pines ---
#define IR_SEND_PIN 2

// Definición de los 7 botones físicos
const uint8_t buttonPins[7] = {0, 1, 3, 4, 5, 6, 7};

// --- Códigos de Jugadores (Mismos que BuzzLY Target) ---
const uint32_t playerCodes[7] = {
    0xE51AFF00, // Jugador 1 (Rojo)
    0x659AFF00, // Jugador 2 (Verde)
    0x5DA2FF00, // Jugador 3 (Azul)
    0x1DE2FF00, // Jugador 4 (Amarillo)
    0xB748FF00, // Jugador 5 (Magenta)
    0x758AFF00, // Jugador 6 (Cyan)
    0xBF40FF00  // Reset
};

const char *playerNames[7] = {"Jugador 1 (ROJO)",    "Jugador 2 (VERDE)",
                              "Jugador 3 (AZUL)",    "Jugador 4 (AMARILLO)",
                              "Jugador 5 (MAGENTA)", "Jugador 6 (CYAN)",
                              "Reset / Off"};

void setup() {
  Serial.begin(115200);

  // Configurar todos los pines de botón como entrada con pullup
  for (int i = 0; i < 7; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // Iniciar módulo emisor IR
  IrSender.begin(IR_SEND_PIN);

  Serial.println("\n==================================");
  Serial.println(" 🔫 BuzzLY Multi-Player Gun Ready");
  Serial.printf("    Pin IR LED : GPIO%d\n", IR_SEND_PIN);
  Serial.println("    7 Botones activos (GPIO 0,1,3,4,5,6,7)");
  Serial.println("==================================");
}

void loop() {
  // Escanear los 7 botones
  for (int i = 0; i < 7; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      delay(20); // Debounce

      if (digitalRead(buttonPins[i]) == LOW) {
        Serial.printf("\n>>> 💥 DISPARO: %s\n", playerNames[i]);
        Serial.printf("    Enviando: 0x%08X\n", playerCodes[i]);

        // Enviar código base
        IrSender.sendNECRaw(playerCodes[i], 0);

        int repeticiones = 0;

        // Mientras mantengas el botón apretado, enviar repeticiones
        while (digitalRead(buttonPins[i]) == LOW) {
          delay(110);
          IrSender.sendNECRepeat();
          repeticiones++;

          // Limitar repeticiones para que no se trabe si el botón falla
          if (repeticiones > 50)
            break;
        }

        Serial.printf("    Disparo finalizado (%d rpt)\n", repeticiones);
        delay(200); // Pequeña pausa entre disparos
      }
    }
  }
  delay(10);
}