# Biblioteca estática micro-ROS (Humble)

Esta carpeta debe contener al menos:

- `libmicroros.a`
- `include/` con los encabezados generados

No se versionan los binarios en el repositorio por tamaño. Genera los archivos con [micro_ros_config/build_libmicroros.sh](micro_ros_config/build_libmicroros.sh) o copiando `libmicroros/` desde un build reciente de [micro_ros_raspberrypi_pico_sdk](https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk) rama `humble`.

Tras generar, verifica que exista `libmicroros/libmicroros.a` antes de ejecutar CMake.
