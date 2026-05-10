# Cableado HC-SR04 ↔ Raspberry Pi Pico 2 W

## Conexión básica

| HC-SR04 | Pico 2 W |
|---------|----------|
| VCC | **5 V** (según datasheet del módulo; muchos HC-SR04 requieren 5 V para funcionar bien) |
| GND | GND común con la Pico |
| TRIG | GPIO salida **3,3 V** (por defecto en firmware: **GPIO 14**) |
| ECHO | GPIO entrada **3,3 V** (por defecto: **GPIO 15**) |

Los pines por defecto se definen en `firmware/src/ultrasonic.h` (`ULTRASONIC_TRIG_PIN`, `ULTRASONIC_ECHO_PIN`). Puedes cambiarlos ahí y recompilar.

## Nivel lógico en ECHO

Muchos HC-SR04 devuelven **5 V** en `ECHO`. Los GPIO de la Pico **no son tolerantes a 5 V**. Opciones:

- **Divisor de tensión** (p. ej. resistencias 1 kΩ / 2 kΩ) para llevar el nivel alto de ECHO a ≈3,3 V.
- **Traductor de nivel** bidireccional o buffer compatible.

No conectar ECHO a 5 V directamente sobre un GPIO de 3,3 V.

## USB

El firmware usa **USB CDC** (stdio USB) para micro-ROS. Conecta el USB de la Pico al PC donde corre el **micro-ROS Agent** (puerto típico en Linux: `/dev/ttyACM0`).

## Velocidad del sonido

El cálculo en firmware usa **343 m/s** (aprox. aire a 20 °C). Para mayor precisión se podría compensar por temperatura; para la demo no es necesario.
