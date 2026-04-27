#include <Arduino.h>
#include <IRremote.hpp>

// --- Configuración de Pines ---
#define IR_SEND_PIN 2
#define BUTTON_PIN  1

// --- Códigos de Prueba (Mismos que BuzzLY Target) ---
#define CODE_PLAYER1 0xE51AFF00 // Botón Rojo
#define CODE_PLAYER2 0x659AFF00 // Botón Verde
#define CODE_PLAYER3 0x5DA2FF00 // Botón Azul
#define CODE_PLAYER4 0x1DE2FF00 // Botón Amarillo
#define CODE_PLAYER5 0xB748FF00 // Botón Magenta
#define CODE_PLAYER6 0x758AFF00 // Botón Cyan
#define CODE_RESET   0xBF40FF00 // Botón Off / Reset

// 🎯 ELIGE AQUÍ QUÉ CÓDIGO QUERÉS PROBAR:
uint32_t active_code = CODE_PLAYER1; 

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Iniciar módulo emisor IR en el pin configurado
  IrSender.begin(IR_SEND_PIN);

  Serial.println("\n==================================");
  Serial.println(" 🔫 BuzzLY Test Trigger Listo");
  Serial.printf("    Pin botón : GPIO%d\n", BUTTON_PIN);
  Serial.printf("    Pin IR    : GPIO%d\n", IR_SEND_PIN);
  Serial.printf("    Dato cfg  : 0x%08X\n", active_code);
  Serial.println("==================================");
  Serial.println("Esperando disparo...");
}

void loop() {
  // Lógica de disparo con filtro anti-rebote (debounce)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(20); // Esperar 20ms para estabilizar el botón
    
    // Si sigue apretado, es un disparo real
    if (digitalRead(BUTTON_PIN) == LOW) {
      
      Serial.println("\n>>> 💥 DISPARO INICIADO");
      Serial.printf("    Enviando : 0x%08X (NEC 32 bits raw)\n", active_code);

      // 1. Enviamos el código base crudo (0 repeticiones iniciales)
      IrSender.sendNECRaw(active_code, 0);
      
      int repeticiones = 0;
      
      // 2. Mientras mantengas el botón apretado, mandamos la señal de "repetición".
      // Esto simula cómo los controles remotos reales manejan botones mantenidos.
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(110); // Espacio estándar NEC de 110ms entre repeticiones
        IrSender.sendNECRepeat();
        repeticiones++;
      }

      Serial.printf("    Repeticiones: %d\n", repeticiones);
      Serial.println(">>> ⏹️ DISPARO FINALIZADO");
      Serial.println("Esperando...");
    }
  }
}