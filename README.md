# 🎯 BuzzLY Target Pro - Firmware Inteligente para ESP32-C3

Este proyecto convierte un ESP32-C3 SuperMini en un blanco de tiro infrarrojo inteligente con monitoreo web, comunicación MQTT en tiempo real, actualizaciones inalámbricas (OTA) y una interfaz de usuario premium con estética **"Ultra-Buzz Neon Green"**.

## 🛠️ Hardware y Conexiones

| Componente | Pin del ESP32-C3 | Descripción |
| :--- | :--- | :--- |
| **LED Strip (WS2812B)** | **GPIO 3** | Entrada de datos (DIN). Configurable desde la web (hasta 30 LEDs). |
| **Sensor IR Receiver** | **GPIO 4** | Pin de datos del receptor (tipo TSOP/VS1838). |
| **Alimentación** | **5V / GND** | Se recomienda alimentar por el pin 5V (USB o externa). |

> [!IMPORTANT]
> **Modo de Flash**: El ESP32-C3 SuperMini debe flashearse en modo **DIO**. Esto ya está configurado en el archivo `platformio.ini`.

## 📡 Conectividad y Comunicación (MQTT Pro)

El sistema está optimizado para comunicación de alto rendimiento vía MQTT:

- **🚀 MQTT Inmediato**: Envía mensajes JSON al instante a un Broker configurable.
  - **Hit Topic**: `target/hit` -> `{"target":"target1", "player_id":1, "action":"hit"}`
  - **Reset Topic**: `target/reset` -> Escucha para reinicio remoto (JSON `{"action":"reset"}` o mensaje simple).
- **Arranque Paralelo**: El sensor IR y los LEDs funcionan de inmediato al encender el target, mientras el WiFi se conecta en segundo plano sin bloquear el sistema.

## 🎮 Mapeo de Jugadores y Colores

| Jugador | Botón (Control 24b) | Código RAW | Color LED |
| :--- | :--- | :--- | :--- |
| **J1** | **Rojo** | `0xE51AFF00` | Rojo 🔴 |
| **J2** | **Verde** | `0x659AFF00` | Verde 🟢 |
| **J3** | **Azul** | `0x5DA2FF00` | Azul 🔵 |
| **J4** | **Naranja** | `0x1DE2FF00` | Naranja 🟠 |
| **J5** | **Magenta** | `0xB748FF00` | Magenta 🟣 |
| **J6** | **Cian** | `0x758AFF00` | Cian 🔵 |
| **Reset** | **Off / Reset** | `0xBF40FF00` | **Violeta** 🟣 (Animación) |

## ✨ Interfaz Web "Buzz Neon Green"

Acceso vía mDNS en `http://nombre-configurado.local` (ej: `target1.local`).

- **Monitor (`/`)**: Interfaz premium con **Glassmorphism**, indicadores de conexión MQTT y reporte del último impacto.
- **Ajustes (`/settings`)**: Configuración dinámica de mDNS, cantidad de LEDs, Bróker y Tópicos MQTT sin reprogramar.

## 🚀 Instalación y Mantenimiento

1. Abrir con PlatformIO en VS Code.
2. Flashear vía USB la primera vez: `pio run -t upload`.
3. Actualizaciones futuras vía **OTA** inalámbrica (Contraseña: `Buzz987`).

---
*Mando del Comando Estelar - BuzzLY Target System.*
