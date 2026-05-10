# Example: verificación micro-ROS (Pico 2 W)

Firmware **independiente** del proyecto ultrasónico: solo comprueba **USB-serial**, **micro-ROS Agent** y un publicador `std_msgs/msg/Int32` en `/pico_connection_test` (nodo `pico_link_check`). Úsalo antes de depender del sensor o del firmware principal.

## Requisitos

- Misma base que el firmware principal: `PICO_SDK_PATH`, toolchain ARM, `libmicroros` ya generada en [`../firmware/libmicroros`](../firmware/libmicroros) (ver [README del repositorio](../../README.md) sección *Generar libmicroros*).

## Compilar

Desde la carpeta **`LABS/`** (laboratorio dentro del clon):

```bash
export PICO_SDK_PATH=/ruta/al/pico-sdk
cd example
mkdir -p build && cd build
cmake -G Ninja -DPICO_BOARD=pico2_w ..
ninja
```

Salida: `pico_link_check.uf2` (y `.elf`).

## Flashear

Modo BOOTSEL y copiar `pico_link_check.uf2` al volumen de la placa.

## Host ROS 2 Humble

1. Arranca **micro-ROS Agent** en el puerto serie de la Pico (USB CDC), por ejemplo:

   ```bash
   docker run -it --rm -v /dev:/dev --privileged --net=host microros/micro-ros-agent:humble serial --dev /dev/ttyACM0 -b 115200
   ```

2. Comprueba el tópico:

   ```bash
   source /opt/ros/humble/setup.bash
   ros2 topic list | grep pico_connection
   ros2 topic echo /pico_connection_test
   ```

Deberías ver el campo `data` del `Int32` incrementándose aproximadamente cada segundo.

## WSL2, Distrobox y entornos

Guía paso a paso (Windows + WSL2, Arch nativo, Arch + Distrobox Ubuntu, Cursor/VS Code): sección **Ruta A / B / C** del [README del repositorio](../../README.md).

En WSL2 el USB-serial de la Pico debe estar **reenviado** con **usbipd-win**; sin eso no verás `/dev/ttyACM0` en WSL.
