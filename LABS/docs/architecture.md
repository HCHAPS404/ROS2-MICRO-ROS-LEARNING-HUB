# Arquitectura

Las rutas de archivos (`firmware/…`, `example/…`) son relativas a la carpeta **`LABS/`** del repositorio.

## Flujo de datos

1. **HC-SR04** — El firmware dispara `TRIG`, mide el ancho de pulso en `ECHO` y convierte a metros (`firmware/src/ultrasonic.c`).
2. **Filtrado** — Tres lecturas consecutivas; con tres válidas se aplica mediana; con menos, promedio o única muestra (`firmware/src/micro_ros_app.c`).
3. **micro-ROS** — Nodo `pico_sonar_node`, publicador en `/ultrasonic/range`, tipo `sensor_msgs/msg/Range`, aproximadamente **5 Hz** (temporizador 200 ms).
4. **Transporte** — USB serial (stdio USB) vía `pico_uart_transport.c`, compatible con el ejemplo oficial de micro-ROS para Pico SDK.
5. **micro-ROS Agent** — En el host ROS 2 Humble, convierte el enlace serial en participante DDS y tópicos ROS 2.
6. **Host** — `ros2 topic echo`, RViz u otros nodos; este repo incluye `range_visualizer` (texto o `nav_msgs/OccupancyGrid` mínima en `/ultrasonic/grid_demo`).

```mermaid
flowchart LR
  HC[HC-SR04]
  FW[Firmware Pico]
  Agent[micro_ros_agent]
  ROS[ROS2_Humble]
  HC --> FW
  FW -->|USB_serial| Agent
  Agent --> ROS
```

## Módulos firmware

| Archivo | Función |
|---------|---------|
| `src/main.c` | Arranque: `ultrasonic_init()`, `micro_ros_app_run()`. |
| `src/ultrasonic.c` | GPIO, disparo, eco con timeout, distancia en m. |
| `src/micro_ros_app.c` | Transporte, ping al agente, nodo, publicador, ejecutor. |
| `pico_uart_transport.c` | Adaptación serial micro-ROS (upstream). |

## Convenciones ROS 2

| Elemento | Valor |
|----------|--------|
| Nodo | `pico_sonar_node` |
| Tópico | `/ultrasonic/range` |
| Tipo | `sensor_msgs/msg/Range` |
| `frame_id` | `ultrasonic_frame` |
| Lectura inválida | `range = -1.0` |

## Plan B (Pico 2 / RP2350)

El repositorio [micro-ROS/micro_ros_raspberrypi_pico_sdk](https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk) está orientado principalmente a RP2040. Para **Pico 2 W (RP2350)** mantén Pico SDK actualizado (`PICO_BOARD=pico2_w`). Si la compilación o el runtime fallan, usa como referencia proyectos comunitarios recientes para RP2350 + micro-ROS (búsqueda: “Micro-ROS RP2350”, Pico SDK 2.x) y adapta solo **CMake / flags / versión de SDK**, conservando los módulos `ultrasonic` y `micro_ros_app` de este repositorio.
