cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(proyecto_sctr_c C CXX)
pico_sdk_init()

add_executable(sctr_app main.c)

# Habilitar salida USB para los printf
pico_enable_stdio_usb(sctr_app 1)
pico_enable_stdio_uart(sctr_app 1)

# IMPORTANTE: Aquí cambiamos hardware_i2c por hardware_adc
target_link_libraries(sctr_app pico_stdlib hardware_adc)

pico_add_extra_outputs(sctr_app)
