cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(proyecto_sctr_c C CXX)
pico_sdk_init()

# Aquí le decimos que use tu archivo main.c
add_executable(sctr_app main.c)

# Habilitar salida por USB y UART (para que funcionen los printf)
pico_enable_stdio_usb(sctr_app 1)
pico_enable_stdio_uart(sctr_app 1)

# Enlazar las librerías necesarias (importante para el I2C del sensor)
target_link_libraries(sctr_app pico_stdlib hardware_i2c)

# Crear el archivo .uf2 que es el que se mete en la Raspberry
pico_add_extra_outputs(sctr_app)
