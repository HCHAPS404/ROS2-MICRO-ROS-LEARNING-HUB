cmake_minimum_required(VERSION 3.12)

if(NOT PICO_BOARD)
  set(PICO_BOARD "pico2_w" CACHE STRING "Placa Raspberry Pi Pico (pico, pico_w, pico2, pico2_w, ...)")
endif()

get_filename_component(FW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../firmware" ABSOLUTE)

include("${FW_ROOT}/pico_sdk_import.cmake")

project(pico_link_check C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(MICROROS_LIB "${FW_ROOT}/libmicroros/libmicroros.a")
if(NOT EXISTS "${MICROROS_LIB}")
  message(FATAL_ERROR "Falta libmicroros. Ejecuta firmware/micro_ros_config/build_libmicroros.sh (ver README.md raíz).")
endif()

link_directories("${FW_ROOT}/libmicroros")

add_executable(pico_link_check
  main.c
  "${FW_ROOT}/pico_uart_transport.c"
)

target_include_directories(pico_link_check PRIVATE
  "${FW_ROOT}"
  "${FW_ROOT}/libmicroros/include"
)

target_link_libraries(pico_link_check
  pico_stdlib
  microros
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

pico_enable_stdio_usb(pico_link_check 1)
pico_enable_stdio_uart(pico_link_check 0)
add_compile_definitions(PICO_UART_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_ENABLE_CRLF_SUPPORT=0)
add_compile_definitions(PICO_STDIO_DEFAULT_CRLF=0)

pico_add_extra_outputs(pico_link_check)
