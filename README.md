# micro-ROS: Raspberry Pi Pico 2 W + HC-SR04 (ROS 2 Humble)

Todo el laboratorio (firmware, example, `ros2_ws`, documentación) está en la carpeta **[LABS/](LABS/)**. En la raíz del repositorio solo permanece este **README.md**.

Demostración: firmware en C que publica `sensor_msgs/msg/Range` en `/ultrasonic/range` por USB-serial con **micro-ROS Agent**, y un nodo Python opcional (`range_visualizer`). Incluye un **[LABS/example/](LABS/example/)** mínimo que publica `std_msgs/msg/Int32` en `/pico_connection_test` para validar enlace **sin** sensor.

**Arquitectura y convenciones:** [LABS/docs/architecture.md](LABS/docs/architecture.md) · **Cableado:** [LABS/docs/wiring.md](LABS/docs/wiring.md) · **Pruebas:** [LABS/docs/test-plan.md](LABS/docs/test-plan.md)

---

## Tabla de contenidos

1. [Mapa del repositorio](#mapa-del-repositorio)
2. [Qué necesitas en cada máquina](#qué-necesitas-en-cada-máquina)
3. [Ruta A — Arch Linux (nativo)](#ruta-a--arch-linux-nativo)
4. [Ruta B — Arch Linux + Distrobox (Ubuntu 22.04)](#ruta-b--arch-linux--distrobox-ubuntu-2204)
5. [Ruta C — Windows 11/10 + WSL2 (Ubuntu 22.04)](#ruta-c--windows-1110--wsl2-ubuntu-2204)
6. [Flujo común: `libmicroros`, firmware, flash, agente, ROS 2](#flujo-común-libmicroros-firmware-flash-agente-ros-2)
7. [Example mínimo y proyecto ultrasónico](#example-mínimo-y-proyecto-ultrasónico)
8. [Editar en Cursor o VS Code (enlace host ↔ WSL ↔ Distrobox)](#editar-en-cursor-o-vs-code-enlace-host--wsl--distrobox)
9. [Problemas frecuentes](#problemas-frecuentes)

---

## Mapa del repositorio

| Ruta | Rol |
|------|-----|
| [LABS/firmware/](LABS/firmware/) | Firmware principal: HC-SR04 + micro-ROS → `/ultrasonic/range` |
| [LABS/example/](LABS/example/) | Firmware mínimo: solo enlace → `/pico_connection_test` ([LABS/example/README.md](LABS/example/README.md)) |
| [LABS/ros2_ws/](LABS/ros2_ws/) | Paquete Python `range_visualizer` (host ROS 2) |
| [LABS/docs/](LABS/docs/) | Arquitectura, cableado, plan de pruebas |

Cadena lógica: **Pico (C + micro-ROS)** → **USB serial** → **micro-ROS Agent** → **ROS 2 Humble** en el host (tópicos DDS).

---

## Qué necesitas en cada máquina

- **Toolchain ARM** (`arm-none-eabi-gcc`, `newlib`), **CMake**, **Ninja**, **Git**, **Python 3**
- **[Pico SDK](https://github.com/raspberrypi/pico-sdk)** clonado y variable de entorno `PICO_SDK_PATH`
- **Docker** (recomendado) para el script que genera `LABS/firmware/libmicroros/`, o una carpeta `libmicroros` copiada desde otro build compatible con Humble
- En el **mismo Linux** donde corre ROS 2 (o accesible por red `localhost`): **micro-ROS Agent** y, si quieres, el workspace `LABS/ros2_ws` con `colcon`

ROS 2 **Humble** está soportado de forma oficial sobre **Ubuntu 22.04**. Por eso en **Windows** se recomienda **WSL2 con Ubuntu 22.04**; en **Arch** puedes compilar el firmware en el host y usar **Distrobox + Ubuntu 22.04** solo para ROS 2 y el agente (o instalar ROS 2 en Arch vía [AUR](https://wiki.archlinux.org/title/ROS) / contenedores, a tu elección).

---

## Ruta A — Arch Linux (nativo)

### A.1 Paquetes (firmware)

```bash
sudo pacman -S --needed base-devel git cmake ninja arm-none-eabi-gcc arm-none-eabi-newlib python docker
sudo systemctl enable --now docker   # si usas el script Docker de libmicroros
sudo usermod -aG docker,dialout $USER   # cerrar sesión y volver a entrar para aplicar grupos
```

### A.2 Pico SDK

```bash
git clone https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk
cd ~/pico-sdk && git submodule update --init
echo 'export PICO_SDK_PATH=$HOME/pico-sdk' >> ~/.bashrc   # o ~/.config/fish/config.fish
export PICO_SDK_PATH=$HOME/pico-sdk
```

### A.3 ROS 2 Humble en el mismo Arch

Opciones habituales: paquetes de la comunidad (AUR), imágenes Docker oficiales de ROS, o **saltar a [Ruta B](#ruta-b--arch-linux--distrobox-ubuntu-2204)** y dejar ROS 2 solo dentro de Ubuntu 22.04. Si instalas Humble en Arch, añade a tu shell:

```bash
source /opt/ros/humble/setup.bash
```

(Ajusta la ruta si tu instalación usa otro prefijo.)

### A.4 Puerto serie USB

Con la Pico conectada por USB:

```bash
ls -l /dev/ttyACM*
```

Tu usuario debe poder leer/escribir el dispositivo (grupo `dialout` / reglas `udev` según tu sistema).

Continúa en [Flujo común](#flujo-común-libmicroros-firmware-flash-agente-ros-2).

---

## Ruta B — Arch Linux + Distrobox (Ubuntu 22.04)

Objetivo: **compilar firmware** en Arch (toolchain nativa) y **ejecutar ROS 2 + agente** dentro de un contenedor Ubuntu 22.04 alineado con la documentación oficial de Humble.

### B.1 Instalar Distrobox y backend (Podman o Docker)

```bash
sudo pacman -S distrobox podman-docker   # o docker en lugar de podman-docker
```

Configura Podman/Docker según la [wiki de Arch](https://wiki.archlinux.org/title/Podman) si hace falta.

### B.2 Crear contenedor con acceso a red y dispositivos serie

Los nombres de dispositivo pueden cambiar al reconectar. Un enfoque práctico es crear el contenedor con privilegios suficientes para ver **/dev** dinámico (solo en máquinas de desarrollo que tú controlas):

```bash
distrobox create --name ros2-humble \
  --image docker.io/library/ubuntu:22.04 \
  --init \
  --additional-flags "--privileged -v /dev:/dev"
```

Entrar:

```bash
distrobox enter ros2-humble
```

Dentro del contenedor, instala locale, curl y sigue la [guía oficial de instalación de ROS 2 Humble en Ubuntu 22.04](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html) (repositorio `packages.ros.org`, `ros-humble-desktop` o `ros-humble-ros-base`, `ros-dev-tools`).

Instala también el agente (paquete o Docker dentro del contenedor). Ejemplo con imagen Docker del agente (requiere Docker en el contenedor o en el host, según montes el socket):

```bash
sudo apt update && sudo apt install -y docker.io
sudo usermod -aG docker $USER
# cerrar sesión del contenedor y volver a entrar para aplicar el grupo
```

### B.3 Montar el repositorio dentro del contenedor

Opción recomendada: el clon del repo vive en tu `$HOME` de Arch y entras con **home montado** (Distrobox monta el home del host por defecto). Así **Cursor en Arch** abre la misma carpeta que ves en `distrobox enter`.

Si clonaste en `~/proyectos/MICRO-ROS-EMB`, el código del laboratorio está en `~/proyectos/MICRO-ROS-EMB/LABS`; desde dentro del contenedor deberías ver la misma ruta bajo tu home.

### B.4 Compilar firmware: ¿dentro o fuera del contenedor?

- **Fuera (Arch host):** usa la sección A.1–A.2 y el [flujo de compilación](#2-compilar-el-firmware); el `.uf2` lo generas en el host.
- **Dentro (Ubuntu 22.04):** instala también `gcc-arm-none-eabi`, `cmake`, `ninja`, clona `pico-sdk` y exporta `PICO_SDK_PATH`; compila igual que en el flujo común.

### B.5 Verificar `/dev/ttyACM0` dentro de Distrobox

Con la Pico enchufada:

```bash
distrobox enter ros2-humble
ls -l /dev/ttyACM*
```

Si no aparece, revisa flags del contenedor, cable o permisos en el host. Continúa en [Flujo común](#flujo-común-libmicroros-firmware-flash-agente-ros-2).

---

## Ruta C — Windows 11/10 + WSL2 (Ubuntu 22.04)

Objetivo: desarrollo con **Cursor/VS Code en Windows**, compilación y ROS 2 **dentro de WSL2**, USB de la Pico **reenviado** a WSL.

### C.1 WSL2 + Ubuntu 22.04

En PowerShell (administrador):

```powershell
wsl --install
wsl --set-default-version 2
wsl --install -d Ubuntu-22.04
```

Actualiza Ubuntu y instala dependencias base:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential git cmake ninja-build python3 python3-pip \
  gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

### C.2 Pico SDK (dentro de WSL)

```bash
git clone https://github.com/raspberrypi/pico-sdk.git ~/pico-sdk
cd ~/pico-sdk && git submodule update --init
echo 'export PICO_SDK_PATH=$HOME/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

### C.3 Clonar **este** repositorio dentro del sistema de archivos de WSL

Evita compilar proyectos grandes solo en `/mnt/c/...` (OneDrive puede ralentizar o bloquear). Mejor:

```bash
mkdir -p ~/repos && cd ~/repos
git clone https://github.com/TU_USUARIO/ROS2-MICRO-ROS-LEARNING-HUB.git
cd ROS2-MICRO-ROS-LEARNING-HUB/LABS
```

(Sustituye la URL por la de tu fork o clon. Los comandos de compilación y ROS 2 de este README asumen que tu directorio de trabajo actual es **`LABS/`** — usa `cd …/LABS` tras clonar.)

### C.4 ROS 2 Humble en WSL

Sigue la [instalación oficial para Ubuntu 22.04](https://docs.ros.org/en/humble/Installation/Ubuntu-Install-Debians.html) dentro de la misma distro WSL.

### C.5 USB serial: **usbipd-win** (imprescindible en WSL2)

En **Windows** (PowerShell administrador), instala [usbipd-win](https://github.com/dorssel/usbipd-win), por ejemplo:

```powershell
winget install -e --id dorssel.usbipd-win
```

1. Conecta la Pico por USB.
2. Lista buses:

   ```powershell
   usbipd list
   ```

3. **Bind** (la primera vez por dispositivo):

   ```powershell
   usbipd bind --busid <X-Y>
   ```

4. **Adjuntar a WSL**:

   ```powershell
   usbipd attach --wsl --busid <X-Y>
   ```

5. En **WSL**:

   ```bash
   ls -l /dev/ttyACM*
   ```

Si no ves `ttyACM0`, revisa que el firmware use el puerto USB correcto y repite el attach tras desconectar/reconectar.

### C.6 Docker en WSL (para `build_libmicroros.sh`)

Instala Docker Desktop con integración WSL para tu distro, o Docker Engine dentro de WSL. Tu usuario debe poder ejecutar `docker` sin sudo donde lances el script.

Continúa en [Flujo común](#flujo-común-libmicroros-firmware-flash-agente-ros-2).

---

## Flujo común: `libmicroros`, firmware, flash, agente, ROS 2

Los pasos siguientes son los mismos en **Arch**, **Distrobox Ubuntu** o **WSL**, salvo rutas y el dispositivo serie.

Sitúate en la carpeta del laboratorio (hija de la raíz del clon):

```bash
cd LABS
```

(En adelante, las rutas `firmware/`, `example/` y `ros2_ws/` son relativas a **`LABS/`**.)

### 1. Generar `libmicroros` (Humble)

Con Docker disponible, **desde `LABS/`**:

```bash
chmod +x firmware/micro_ros_config/build_libmicroros.sh
./firmware/micro_ros_config/build_libmicroros.sh
```

Esto rellena `firmware/libmicroros/` con `libmicroros.a` e `include/`. Alternativa: copiar esa carpeta desde un build de [micro_ros_raspberrypi_pico_sdk](https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk) rama `humble`.

### 2. Compilar el firmware

```bash
export PICO_SDK_PATH=/ruta/al/pico-sdk    # ej. $HOME/pico-sdk
cd firmware
mkdir -p build && cd build
cmake -G Ninja -DPICO_BOARD=pico2_w ..
ninja
```

Salida esperada: `pico_sonar_node.uf2` (y `.elf`). Para **Pico** clásica: `-DPICO_BOARD=pico`.

> **RP2350 (Pico 2 W):** si falla compilación o enlace, revisa [LABS/docs/architecture.md](LABS/docs/architecture.md) (plan B).

### 3. Flashear

Modo **BOOTSEL** en la Pico, copia el `.uf2` al volumen `RPI-RP2` (o el nombre que muestre el sistema).

### 4. Agente micro-ROS (Humble)

Sustituye el dispositivo si no es `ttyACM0`:

```bash
docker run -it --rm -v /dev:/dev --privileged --net=host \
  microros/micro-ros-agent:humble serial --dev /dev/ttyACM0 -b 115200
```

O el ejecutable `micro_ros_agent` instalado desde los paquetes ROS 2 / fuente.

### 5. Verificar tópicos

```bash
source /opt/ros/humble/setup.bash
ros2 topic list
ros2 topic echo /ultrasonic/range
```

### 6. Nodo `range_visualizer` (opcional)

Desde **`LABS/`** (si acabas de compilar firmware, `cd ..` dos veces hasta estar en `LABS`, o abre una nueva terminal en `LABS`):

```bash
cd ros2_ws
source /opt/ros/humble/setup.bash
colcon build --packages-select range_visualizer
source install/setup.bash
ros2 run range_visualizer range_visualizer
# Rejilla mínima:
ros2 run range_visualizer range_visualizer --ros-args -p mode:=grid
```

---

## Example mínimo y proyecto ultrasónico

| Objetivo | Directorio | Tópico de prueba |
|----------|------------|------------------|
| Solo enlace micro-ROS + agente + ROS 2 | [LABS/example/](LABS/example/) | `/pico_connection_test` (`std_msgs/msg/Int32`) |
| HC-SR04 + publicación continua | [LABS/firmware/](LABS/firmware/) | `/ultrasonic/range` (`sensor_msgs/msg/Range`) |

Instrucciones detalladas del example: [LABS/example/README.md](LABS/example/README.md).

---

## Editar en Cursor o VS Code (enlace host ↔ WSL ↔ Distrobox)

### Windows + WSL2

1. Instala la extensión **WSL** (Remote Development) en VS Code; en Cursor suele existir soporte equivalente para abrir carpeta en WSL.
2. **Abrir carpeta en WSL:** `Ctrl+Shift+P` → “WSL: Open Folder in WSL…” → elige `~/repos/.../ROS2-MICRO-ROS-LEARNING-HUB/LABS` si quieres el laboratorio como raíz del workspace, o la raíz del clon si prefieres ver también el README de la raíz.
3. Así **editas y ejecutas terminales integradas** en la misma Ubuntu donde están `colcon`, `ros2` y (si configuraste) Docker.
4. Si abres desde `\\wsl$\Ubuntu-22.04\home\...` sin modo remoto, la integración de terminal puede seguir siendo la de Windows: para compilar ROS y usar el agente, abre terminal **dentro de WSL** (`wsl` en PowerShell o pestaña WSL en el IDE).

### Arch + Distrobox

1. Clona el repo en tu `$HOME` de Arch para que Distrobox lo vea montado.
2. `distrobox enter ros2-humble` y trabaja ahí con `code .` / `cursor .` si tienes la CLI instalada **dentro** del contenedor, **o** edita desde Arch en la misma ruta y ejecuta builds ROS en la terminal del contenedor.

### Firmware (C) en Windows sin WSL

Es posible instalar ARM GCC y CMake en Windows nativo, pero este README prioriza **WSL2** o **Arch** para coincidir con el ecosistema Pico SDK y scripts.

---

## Problemas frecuentes

| Síntoma | Qué revisar |
|---------|-------------|
| `push` rechazado en GitHub | El remoto tiene commits iniciales; `git pull origin main --allow-unrelated-histories` o acordar historial con `--force` solo si aceptas sobrescribir el remoto. |
| No existe `/dev/ttyACM0` en WSL | **usbipd-win**: bind + attach del bus correcto tras enchufar la Pico. |
| `Permission denied` en el puerto serie | En Linux: usuario en grupo `dialout`; en Docker agent: `--privileged` y `-v /dev:/dev` como en el ejemplo. |
| ROS 2 no encuentra paquetes | `source /opt/ros/humble/setup.bash` y, tras `colcon build`, `source install/setup.bash` desde `LABS/ros2_ws`. |
| LF/CRLF warnings en Git | Normal en Windows; no impiden el commit. Opcional: `git config core.autocrlf true` en Windows. |
| Build lento bajo OneDrive | Clona el repo a disco local/WSL y trabaja en `LABS/`; evita compilar solo bajo `C:\Users\...\OneDrive\...` si va lento (`LABS/firmware/build/`, `LABS/firmware/libmicroros`). |

---

## Referencias rápidas

- [micro-ROS Raspberry Pi Pico SDK](https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk)
- [micro-ROS Agent](https://micro.ros.org/docs/tutorials/advanced/agent/)
- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [ROS 2 Humble](https://docs.ros.org/en/humble/)
