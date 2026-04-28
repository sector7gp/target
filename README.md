# 🎯 BuzzLY Target Pro - Firmware Inteligente para ESP32-C3

Este proyecto convierte un ESP32-C3 SuperMini en un blanco de tiro infrarrojo inteligente con monitoreo web, comunicación MQTT en tiempo real, actualizaciones inalámbricas (OTA) y una interfaz de usuario premium con estética "Buzz Neon Green".

## 🛠️ Hardware y Conexiones

| Componente | Pin del ESP32-C3 | Descripción |
| :--- | :--- | :--- |
| **LED Strip (WS2812B)** | **GPIO 3** | Entrada de datos (DIN). Configurable desde la web. |
| **Sensor IR Receiver** | **GPIO 4** | Pin de datos del receptor (tipo TSOP/VS1838). |
| **Alimentación** | **5V / GND** | Se recomienda alimentar por el pin 5V (USB o externa). |

> [!IMPORTANT]
> **Modo de Flash**: El ESP32-C3 SuperMini debe flashearse en modo **DIO**. Esto ya está configurado en el archivo `platformio.ini`.

## 📡 Conectividad y Comunicación (Dual Mode)

El sistema reporta cada impacto mediante dos protocolos simultáneos:

- **🚀 MQTT (Recomendado)**: Envía mensajes JSON instantáneos a un Broker configurable.
  - **Hit Topic**: `target/hit` -> `{"target":"target1", "player_id":1, "action":"hit"}`
  - **Reset Topic**: `target/reset` -> El blanco escucha este topic para reiniciarse remotamente.
- **🔗 HTTP API**: Realiza un POST JSON a la URL que configures en los ajustes.
- **Arranque Paralelo**: El sistema inicia el sensor IR y las animaciones de inmediato al encenderse, sin esperar a que el WiFi se conecte.

## 🎮 Comandos IR (NEC Protocol)

| Botón (Control 24b) | Código RAW | Acción |
| :--- | :--- | :--- |
| **Rojo** | `0xE51AFF00` | Jugador 1 |
| **Verde** | `0x659AFF00` | Jugador 2 |
| **Azul** | `0x5DA2FF00` | Jugador 3 |
| **Amarillo** | `0x1DE2FF00` | Jugador 4 |
| **Magenta** | `0xB748FF00` | Jugador 5 |
| **Cyan** | `0x758AFF00` | Jugador 6 |
| **Off / Reset** | `0xBF40FF00` | Reset visual a "Glow Verde" |

## ✨ Interfaz Web "Buzz Neon Green"

Acceso vía mDNS en `http://target1.local` (o el nombre configurado).

- **Monitor (`/`)**: Diseño Premium con Glassmorphism y estatus en tiempo real de MQTT y último impacto.
- **Ajustes (`/settings`)**: Configuración dinámica sin flashear:
  - Nombre mDNS y cantidad de LEDs (1-30).
  - Broker MQTT y tópicos personalizados.
  - Endpoint de API Legacy.

## 🚀 Instalación y Mantenimiento

1. Abrir con PlatformIO en VS Code.
2. Flashear la primera vez vía USB: `pio run -t upload`.
3. Mantenimiento remoto vía **OTA** activo (Password: `Buzz987`).

## 🔫 Herramientas Extra
- **`aux/testTrigger.ino`**: Firmware para crear un emisor de prueba con 7 botones (6 jugadores + 1 reset).

---
*Mando del Comando Estelar - BuzzLY Target System.*
