# Proyecto SCTR: Sistema de Control en Tiempo Real

## Descripción Técnica
Este proyecto implementa un sistema de control de iluminación automático basado en una Raspberry Pi Pico y un sensor INA219. El sistema utiliza una máquina de estados (FSM) basada en tablas ("Table-Driven") para gestionar las transiciones entre estados (Apagado, Parpadeo, Encendido) en función de la corriente leída por el sensor.

## Autores y Roles
* **Yago Andrés Martínez:** 
* **Brais Ansede González:** 
* **Andrea Caeiro Almeida:** 
* **Diego Costas Méndez:**

## Esquema del Circuito
![Esquema del Circuito](esquema.png)

> El sistema conecta el sensor INA219 mediante I2C (Pines GP0 y GP1).

## Instrucciones de Compilación
1. Clonar el repositorio.
2. Crear carpeta build: `mkdir build && cd build`
3. Configurar: `cmake ..`
4. Compilar: `make`
5. Arrastrar el archivo `.uf2` a la Raspberry Pi Pico.
