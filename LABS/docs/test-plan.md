# Plan de pruebas

## Nivel 1 — Hardware (sin ROS)

**Objetivo:** comprobar eco y distancia aproximada.

1. Montaje según [wiring.md](wiring.md) (TRIG/ECHO, niveles seguros).
2. Flashear firmware.
3. (Opcional) Añadir temporalmente `printf` en `ultrasonic_read_distance_m` o medir con osciloscopio/logic analyzer el pulso ECHO.

**Criterio:** obstáculo a distancia conocida produce lecturas coherentes antes de depender del agente.

## Nivel 2 — Comunicación micro-ROS

**Objetivo:** agente y tópico visibles.

1. Host con ROS 2 Humble y `micro_ros_agent` (o Docker `microros/micro-ros-agent:humble`).
2. Arrancar agente en el puerto serie correcto (p. ej. `serial --dev /dev/ttyACM0 -b 115200`).
3. Alimentar / resetear la Pico con el firmware de este proyecto.

**Criterio:**

- `ros2 topic list` incluye `/ultrasonic/range`.
- `ros2 topic echo /ultrasonic/range` muestra mensajes `sensor_msgs/Range` con `frame_id` `ultrasonic_frame`.
- Al mover un obstáculo, el campo `range` cambia de forma razonable; lecturas inválidas aparecen como `range: -1.0` (eco ausente/timeout).

## Nivel 3 — Integración visualizador

**Objetivo:** cadena completa hasta representación simple.

1. `colcon build` del paquete `range_visualizer` y `source install/setup.bash`.
2. Modo texto: `ros2 run range_visualizer range_visualizer` — comprobar logs periódicos.
3. Modo rejilla: `ros2 run range_visualizer range_visualizer --ros-args -p mode:=grid` — comprobar publicación en `/ultrasonic/grid_demo` con `ros2 topic echo` o RViz (`Map`).

**Evidencia sugerida:** capturas de terminal (`topic list`, `echo`) y, si aplica, RViz mostrando la rejilla demo.

## Checklist rápido

- [ ] Niveles ECHO seguros para 3,3 V
- [ ] `libmicroros` generado para **Humble**
- [ ] `PICO_BOARD` correcto al compilar
- [ ] Agente y firmware en el mismo baud/puerto
- [ ] Tópico `/ultrasonic/range` estable
- [ ] (Opcional) `range_visualizer` en modo `text` o `grid`
