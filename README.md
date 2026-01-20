# Proyecto SCTR: Sistema de Control en Tiempo Real

## Descripción Técnica
Este proyecto implementa un sistema de control de iluminación automático basado en una Raspberry Pi Pico. El sistema utiliza una máquina de estados (FSM) basada en tablas ("Table-Driven") para gestionar las transiciones entre estados (Apagado, Parpadeo, Encendido) en función del voltaje medido por el sensor LDR.

## Autores y Roles
* **Yago Andrés Martínez:** 
* **Brais Ansede González:** 
* **Andrea Caeiro Almeida:** 
* **Diego Costas Méndez:**

## Esquema del Circuito
![Esquema del Circuito](esquema.png)

> El sistema conecta el sensor LDR (Pines GPIO-- y GPIO--).

## Instrucciones de Compilación
1. Clonar el repositorio.
2. Crear carpeta build: `mkdir build && cd build`
3. Configurar: `cmake ..`
4. Compilar: `make`
5. Arrastrar el archivo `.uf2` a la Raspberry Pi Pico.

## Instrucciones de Conexión con la placa para lectura de datos del sensor LDR
1. Ejecutar como administrador la consola Powershell de Windows.
2. Conectar la placa con el programa ya guardado archivo `.uf2` SIN pulsar el botón.
3. Introducir el comando `usbipd list` y anotar el `<BUSID>`.
4. Introducir los comandos:
   `usbipd bind --busid <BUSID>`
   `usbipd attach --wsl --busid <BUSID>`
6. En la consola de Ubuntu se introduce el comando:
  `sudo picocom -b 115200 /dev/ttyACM0`
