# 🎯 BuzzLY Target Pro - Firmware Inteligente para ESP32-C3

Este proyecto convierte un ESP32-C3 SuperMini en un blanco de tiro infrarrojo inteligente de nivel profesional con comunicación MQTT en tiempo real, monitoreo de señal y telemetría avanzada.

## 🛠️ Hardware y Conexiones

| Componente | Pin del ESP32-C3 | Descripción |
| :--- | :--- | :--- |
| **LED Strip (WS2812B)** | **GPIO 3** | Entrada de datos (DIN). Configurable vía Web. |
| **LED RGB (Ánodo Común)** | **G:1, R:2, B:0, A:5** | Alternativa física/compacta. |
| **Sensor IR Receiver** | **GPIO 4** | Pin de datos del receptor (IRremote 4.x). |

## 📡 Capacidades Pro (V3.1)

- **🚀 MQTT Telemetría**: Envío instantáneo de hits con ID de target único.
  - **Topic Hit**: `target/hit` -> `{"target":"nombre", "player_id":X, "action":"hit"}`.
  - **Topic Reset Seguro**: `target/reset`. Requiere el JSON `{"action":"reset"}` para validar el comando.
- **📊 Monitoreo en Tiempo Real**: El portal web muestra la intensidad de señal WiFi (**RSSI**) en dBm para diagnóstico de campo.
- **⚡ Performance No Bloqueante**: El sensor IR y los efectos visuales inician en milisegundos, operando en paralelo a la negociación de red.

## 🎮 Mapeo de Jugadores

| Jugador | Color LED | Reset |
| :--- | :--- | :--- |
| **J1 a J6** | Rojo, Verde, Azul, Naranja, Magenta, Cian | **Violeta** 🟣 |

## ✨ Interfaz "Target Pro"
- **Refresco Optimizado**: El monitor web se actualiza cada **15 segundos** para reducir el tráfico de red en despliegues masivos.
- **Estética Buzz Neon**: UI moderna basada en Glassmorphism y tipografía optimizada.

## 🚀 Instalación
1. Flashear vía USB la primera vez (`pio run -t upload`).
2. Configurar WiFi y MQTT en el portal cautivo o `/settings`.
3. Actualizar vía **OTA** en el futuro (Pass: `Buzz987`).

---
*Mando del Comando Estelar - BuzzLY Target System.*
