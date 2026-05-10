cmake_minimum_required(VERSION 3.12)

if(NOT PICO_BOARD)
  set(PICO_BOARD "pico2_w" CACHE STRING "Placa Raspberry Pi Pico (pico, pico_w, pico2, pico2_w, ...)")
endif()

include(pico_sdk_import.cmake)

project(pico_sonar_node C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libmicroros/libmicroros.a")
  message(FATAL_ERROR "Falta libmicroros. Genera la carpeta firmware/libmicroros con micro_ros_config/build_libmicroros.sh (ver README.md).")
endif()

link_directories(libmicroros)

add_executable(pico_sonar_node
  src/main.c
  src/ultrasonic.c
  src/micro_ros_app.c
  pico_uart_transport.c
)

target_include_directories(pico_sonar_node PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  libmicroros/include
)

target_link_libraries(pico_sonar_node
  pico_stdlib
  hardware_gpio
  m
  microros
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

pico_enable_stdio_usb(pico_sonar_node 1)
pico_enable_stdio_uart(pico_sonar_node 0)
add_compile_definitions(PICO_UART_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_DEFAULT_CRLF=0)

pico_add_extra_outputs(pico_sonar_node)
