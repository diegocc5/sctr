# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.13)

# Importa el SDK de la Raspberry Pi Pico
include(pico_sdk_import.cmake)

# Define el nombre del proyecto
project(proyecto_sctr_c C CXX)

# Inicializa las configuraciones del SDK
pico_sdk_init()

# Crea el ejecutable a partir del código fuente
add_executable(sctr_app main.c)

# Habilitar salida por USB y UART ()
pico_enable_stdio_usb(sctr_app 1)
pico_enable_stdio_uart(sctr_app 0)

# Enlazar las librerías necesarias (importante para el I2C del sensor)
target_link_libraries(sctr_app pico_stdlib hardware_i2c)

# Crear el archivo .uf2 que es el que cargará y ejecutará en la placa
pico_add_extra_outputs(sctr_app)
