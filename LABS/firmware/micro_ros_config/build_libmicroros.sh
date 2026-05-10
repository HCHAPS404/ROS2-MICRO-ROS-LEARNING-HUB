#!/usr/bin/env bash
# Genera LABS/firmware/libmicroros (ruta relativa al clon) usando la imagen oficial micro-ROS (ROS 2 Humble).
# Requisitos: git, docker. Ejecutar desde cualquier directorio; el script se auto-ubica.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FW_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
OUT_DIR="${FW_DIR}/libmicroros"
WORKDIR="$(mktemp -d)"
cleanup() { rm -rf "${WORKDIR}"; }
trap cleanup EXIT

echo "Clonando micro_ros_raspberrypi_pico_sdk (humble) en temporal..."
git clone --depth 1 -b humble https://github.com/micro-ROS/micro_ros_raspberrypi_pico_sdk.git "${WORKDIR}/sdk"

echo "Descargando microros/micro_ros_static_library_builder:humble ..."
docker pull microros/micro_ros_static_library_builder:humble

echo "Compilando libmicroros (puede tardar varios minutos)..."
docker run --rm -v "${WORKDIR}/sdk:/project" microros/micro_ros_static_library_builder:humble

mkdir -p "${OUT_DIR}"
cp -a "${WORKDIR}/sdk/libmicroros/." "${OUT_DIR}/"
echo "Listo: ${OUT_DIR} contiene libmicroros.a e include/."
