# Proyecto SCTR: Sistema de Control en Tiempo Real

## Descripción Técnica
Este proyecto implementa un sistema de control de iluminación automático basado en una Raspberry Pi Pico. El sistema utiliza una máquina de estados (FSM) basada en tablas ("Table-Driven") para gestionar las transiciones entre estados (Apagado, Parpadeo, Encendido) en función del voltaje medido por el sensor LDR.

## Autores y Roles
* **Yago Andrés Martínez: desarrollo de hardware y software principal.** 
* **Brais Ansede González: colaboración general.** 
* **Andrea Caeiro Almeida: documentación software y compilación.** 
* **Diego Costas Méndez: gestión del repositorio y documentación inicial.**

## Esquema del Circuito
![Esquema del Circuito](esquema2.png)

> Conexión Sensor LDR Pin 36 (Tensión 3.3V OUT).

> Resistencia de 10 kΩ conectada en serie con el Sensor LDR.

> Conexión ADC0 Pin 31 (GP26) a la salida del Sensor LDR.

> Conexión LED Pin 4 (I2C1 SDA - GP2).


## Instrucciones de Compilación
1. Clonar el repositorio.
2. Crear carpeta build: `mkdir build && cd build`
3. Configurar: `cmake ..`
4. Compilar: `make`
5. Arrastrar el archivo `.uf2` a la Raspberry Pi Pico.

##  Instrucciones de Conexión con la placa para lectura de datos del ADC por consola
1. Ejecutar como administrador la consola Powershell de Windows.
2. Introducir el comando `usbipd list` y anotar el `<BUSID>` del dispositivo serie usb (COM3).
3. Introducir los comandos:
   `usbipd bind --busid <BUSID>`
   `usbipd attach --wsl --busid <BUSID>`
4. En la consola de Ubuntu se introduce el comando:
  `sudo picocom -b 115200 /dev/ttyACM0`

## Modificación respecto a la propuesta inicial
Inicialmente la medición de la tensión en el sensor LDR iba a realizarse mediante un sensor INA219, este estaría conectado a la Raspberry Pi Pico para la lectura de valores y posterior toma de decisiones. Pero tras considerarlo, se decidió eliminar este sensor para facilitar el proceso, y tomar los valores del sensor LDR directamente en la Raspberry Pi Pico a través de uno de sus puertos ADC.

## Prueba en video del proyecto



https://github.com/user-attachments/assets/37311a48-2561-4aa9-8383-69e395b56a59




