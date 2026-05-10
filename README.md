# micro-ROS: Raspberry Pi Pico 2 W + HC-SR04 (ROS 2 Humble)

Demostración mínima: firmware en C que publica `sensor_msgs/msg/Range` en `/ultrasonic/range` por USB-serial con micro-ROS Agent, y un nodo Python opcional para texto o una rejilla muy simple.

**Entorno de compilación del firmware recomendado:** Arch Linux (toolchain `arm-none-eabi-gcc`, CMake, Ninja). El código es portable; las rutas son relativas al repositorio.

## Requisitos (Arch Linux)

- `base-devel`, `cmake`, `ninja`, `arm-none-eabi-gcc`, `arm-none-eabi-newlib`, `git`, `python`
- [Pico SDK](https://github.com/raspberrypi/pico-sdk) clonado y variable `PICO_SDK_PATH`
- Docker (solo para generar `libmicroros` con el script oficial)
- En el PC con ROS 2 **Humble**: `micro_ros_agent`, workspace Python (`colcon`) para `range_visualizer`

```bash
export PICO_SDK_PATH=/ruta/al/pico-sdk
```

## 1. Generar `libmicroros` (Humble)

Desde el repositorio:

```bash
chmod +x firmware/micro_ros_config/build_libmicroros.sh
./firmware/micro_ros_config/build_libmicroros.sh
```

Esto rellena `firmware/libmicroros/` con `libmicroros.a` e `include/`. Si prefieres no usar Docker, copia esa carpeta desde un build de [micro_ros_raspberrypi_pico_sdk](https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk) rama `humble`.

## 2. Compilar el firmware

```bash
cd firmware
mkdir -p build && cd build
cmake -G Ninja -DPICO_BOARD=pico2_w ..
ninja
```

Salida esperada: `pico_sonar_node.uf2` (y `.elf`). Para **Pico** clásica u otra placa, cambia `PICO_BOARD` (por ejemplo `pico`).

> **RP2350 (Pico 2 W):** si aparecen errores de compilación o enlace con la línea oficial, revisa [docs/architecture.md](docs/architecture.md) (plan B con forks/ejemplos comunitarios).

## 3. Flashear

Con la placa en modo BOOTSEL, copia el `.uf2` al volumen `RPI-RP2` (o el nombre que muestre tu sistema).

## 4. Agente micro-ROS (host Humble)

Con el firmware usando transporte USB CDC (stdio USB), en el host:

```bash
# Ejemplo con Docker (ajusta el dispositivo)
docker run -it --rm -v /dev:/dev --privileged --net=host microros/micro-ros-agent:humble serial --dev /dev/ttyACM0 -b 115200
```

O el binario `micro_ros_agent` equivalente instalado en tu Humble.

## 5. Verificar tópicos

```bash
source /opt/ros/humble/setup.bash
ros2 topic list
ros2 topic echo /ultrasonic/range
```

## 6. Nodo `range_visualizer` (opcional)

```bash
cd ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select range_visualizer
source install/setup.bash
# Texto (por defecto)
ros2 run range_visualizer range_visualizer
# Rejilla mínima 1×N
ros2 run range_visualizer range_visualizer --ros-args -p mode:=grid
```

Cableado y pruebas: [docs/wiring.md](docs/wiring.md) y [docs/test-plan.md](docs/test-plan.md).

## Example: solo enlace micro-ROS

Para validar agente + USB + publicación **sin** ultrasónico, compila el firmware en [example/](example/) y sigue [example/README.md](example/README.md) (tópico `/pico_connection_test`).
