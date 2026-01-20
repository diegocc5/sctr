# Versión mínima de CMake requerida
cmake_minimum_required(VERSION 3.13)

# Importa el SDK de la Raspberry Pi Pico
include(pico_sdk_import.cmake)

# Define el nombre del proyecto y los lenguajes utilizados en este caso C
project(proyecto_sctr_c C CXX)

# Inicializa las configuraciones del SDK
pico_sdk_init()

# Crea el ejecutable a partir del código fuente
add_executable(sctr_app main.c)

# Habilitar salida USB para los printf
pico_enable_stdio_usb(sctr_app 1)
pico_enable_stdio_uart(sctr_app 1)

# Crea el ejecutable a partir del código fuente
target_link_libraries(sctr_app pico_stdlib hardware_adc)

# Genera archivos .uf2, .bin y .elf al compilar
pico_add_extra_outputs(sctr_app)
