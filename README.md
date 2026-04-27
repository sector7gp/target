# 🎯 BuzzLY Target - Firmware Estable para ESP32-C3

Este proyecto convierte un ESP32-C3 SuperMini en un blanco de tiro infrarrojo inteligente con monitoreo web, actualizaciones inalámbricas (OTA) y feedback visual mediante una tira de LEDs WS2812B.

## 🛠️ Hardware y Conexiones

| Componente | Pin del ESP32-C3 | Descripción |
| :--- | :--- | :--- |
| **LED Strip (WS2812B)** | **GPIO 3** | Entrada de datos (DIN). Capacidad: 10 LEDs. |
| **Sensor IR Receiver** | **GPIO 4** | Pin de datos del receptor (tipo TSOP/VS1838). |
| **Alimentación** | **5V / GND** | Se recomienda alimentar por el pin 5V (USB o externa). |

> [!IMPORTANT]
> **Modo de Flash**: El ESP32-C3 SuperMini debe flashearse en modo **DIO**. Esto ya está configurado en el archivo `platformio.ini`.

## 📡 Conectividad y Red

- **Portal de Configuración**: Si no hay WiFi, se crea la red `BuzzLY-Setup`.
- **mDNS Name**: Acceso por defecto en `http://target1.local` (configurable desde la web).
- **OTA Updates**: Actualización inalámbrica habilitada (Pass: `Buzz987`).
- **API Endpoint**: Envía un JSON tipo `{"player_id": 1, "action": "hit"}` a la URL que configures.

## 🎮 Comandos IR (NEC Protocol)

El sistema utiliza un filtrado estricto. Solo estos comandos activan el sistema:

| Botón (Control 24b) | Código RAW | Acción |
| :--- | :--- | :--- |
| **Rojo** | `0xE51AFF00` | Impacto Jugador 1 |
| **Verde** | `0x659AFF00` | Impacto Jugador 2 |
| **Azul** | `0x5DA2FF00` | Impacto Jugador 3 |
| **Amarillo** | `0x1DE2FF00` | Impacto Jugador 4 |
| **Magenta** | `0xB748FF00` | Impacto Jugador 5 |
| **Cyan** | `0x758AFF00` | Impacto Jugador 6 |
| **Verde Oscuro (Reset)** | `0xBF40FF00` | Reset visual a "Glow Verde" |

## 💻 Interfaz Web

- **Home (`/`)**: Muestra el último jugador que impactó y la potencia del WiFi. Incluye un botón para resetear el estado a Verde Glowing manualmente.
- **Settings (`/settings`)**: Permite cambiar la URL de la API y el nombre de red (mDNS) sin tocar el código. Al guardar, la placa se reinicia automáticamente.

## 🚀 Instalación (PlatformIO)

1. Abrir la carpeta en VS Code con PlatformIO.
2. Editar `src/config.h` si necesitás cambiar los códigos IR por defecto.
3. Conectar el ESP32-C3 por USB.
4. Ejecutar el comando de subida: `pio run -t upload`.
5. Una vez configurado el WiFi, las siguientes subidas se pueden hacer por OTA automáticamente.

---
*Desarrollado para BuzzLY Target System.*
