/*
 * Proyecto SCTR : Control Automático de Iluminación ambiental mediante Raspberry Pi y sensor LDR
 *
 * Autores: Yago Andrés Martínez, Brais Ansede González, Andrea Caeiro Almeida y Diego Costas Méndez.
 *
 * Comportamiento:
 * APAGADO → ENCENDIDO con 'LUMINOSO'; APAGADO → PARPADEO con 'TENUE';
 * PARPADEO → ENCENDIDO con 'LUMINOSO'; PARPADEO → APAGADO con 'OSCURO';
 * ENCENDIDO → PARPADEO con 'TENUE'; ENCENDIDO → APAGADO con 'OSCURO';
 * El resto de combinaciones quedan como NULL y se validan antes de llamar.
 *
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

/* ------------------------------------------------------------
 * CONFIGURACIÓN GENERAL DEL SISTEMA
 * ------------------------------------------------------------ */

/* Selección del periférico I2C utilizado. */
#define I2C_PORT i2c0

/* Pines GPIO asignados al bus I2C. */
#define I2C_SDA  0
#define I2C_SCL  1

/* Dirección I2C del sensor INA219. */
#define INA219_ADDR 0x40

/* ------------------------------------------------------------
 * DEFINICIÓN DE REGISTROS DEL INA219
 * ------------------------------------------------------------ */

/* Registro de configuración del INA219. */
#define INA219_REG_CONFIG        0x00

/* Registro de voltaje en la resistencia shunt. */
#define INA219_REG_SHUNT_VOLTAGE 0x01

/* Registro de voltaje del bus. */
#define INA219_REG_BUS_VOLTAGE   0x02

/* Registro de corriente. */
#define INA219_REG_CURRENT       0x04

/* Registro de calibración. */
#define INA219_REG_CALIBRATION   0x05

/* ------------------------------------------------------------
 * FUNCIONES DE ACCESO AL SENSOR INA219
 * ------------------------------------------------------------ */

/* Escribe un valor de 16 bits en un registro del INA219
 * mediante el bus I2C. */
void ina219_write_register(uint8_t reg, uint16_t value) 
{
    /* Buffer de transmisión:
     * - byte 0: dirección del registro
     * - byte 1: parte alta del dato
     * - byte 2: parte baja del dato */
    uint8_t data[3];
    data[0] = reg;
    data[1] = (value >> 8) & 0xFF;
    data[2] = value & 0xFF;

    /* Escritura bloqueante por I2C. */
    i2c_write_blocking(I2C_PORT, INA219_ADDR, data, 3, false);
}

/* Lee un valor de 16 bits desde un registro del INA219. */
int16_t ina219_read_register(uint8_t reg) {
    uint8_t buf[2];

    /* Se escribe primero la dirección del registro a leer.
     * El parámetro 'true' indica que no se libera el bus
     * entre la escritura y la lectura (repeated start). */
    i2c_write_blocking(I2C_PORT, INA219_ADDR, &reg, 1, true);

     /* Lectura de dos bytes desde el registro indicado. */
    i2c_read_blocking(I2C_PORT, INA219_ADDR, buf, 2, false);

    /* Se combinan los dos bytes en un entero de 16 bits. */
    return (buf[0] << 8) | buf[1];
}

/* ------------------------------------------------------------
 * INICIALIZACIÓN DEL SENSOR INA219
 * ------------------------------------------------------------ */

 /* Configura el sensor INA219 con los parámetros deseados. */
 void ina219_init(void) {

    /* Configuración del sensor:
     * - Rango de voltaje del bus: 32 V
     * - Ganancia del amplificador: /8 (±320 mV en shunt)
     * - Resolución ADC: 12 bits
     * - Modo de funcionamiento continuo */
    uint16_t config = 0x019F;

    /* Escritura del registro de configuración. */
    ina219_write_register(INA219_REG_CONFIG, config);

    /* Valor de calibración:
     * - Pensado para corrientes pequeñas
     * - Se asume una resistencia shunt típica de 0.1 ohm */
    uint16_t calibration = 4096;

    /* Escritura del registro de calibración. */
    ina219_write_register(INA219_REG_CALIBRATION, calibration);
}

/* ------------------------------------------------------------
 * FUNCIONES DE LECTURA DE MAGNITUDES
 * ------------------------------------------------------------ */

/* Lee la corriente medida por el INA219 y la devuelve en mA. */
float ina219_read_current_mA(void) 
{
    /* Lectura cruda del registro de corriente. */
    int16_t raw = ina219_read_register(INA219_REG_CURRENT);

    /* Conversión a miliamperios:
     * se asume un LSB típico de 0.1 mA tras la calibración */
    return raw * 0.1f;

     /* NOTA:
     * La validez de esta conversión depende directamente
     * del valor de calibración configurado previamente. */
}

/* ------------------------------------------------------------
 * MÁQUINA DE ESTADOS (LÓGICA)
 * ------------------------------------------------------------ */

/* Enum para los estados del led, este tendrá los valores de APAGADO,
 * PARPADEO y ENCENDIDO. Se añade STATE_MAX para su uso más adelante para dimensionar 
 * tablas. */
enum state {
    APAGADO = 0,
    PARPADEO,
    ENCENDIDO,
    STATE_MAX
};

/* Enum para los eventos de transición del led, estos serán OSCURO,
 * TENUE y LUMINOSO. Se añade el evento por defecto NONE y EVENT_MAX para su uso
 * más adelante para dimensionar. */
enum event {
    NONE = 0,
    OSCURO,
    TENUE,
    LUMINOSO,
    EVENT_MAX
};

/* Funciones para mostrar en consola el estado del led. */
void oscuridad(void){
    gpio_put(2, 0);
    sleep_ms(500);
}
void poca_luz(void){
    printf("LED: TENUE (PARPADEO).\n");
}
void luz(void){
    gpio_put(2, 1);
    sleep_ms(500);
}

/* Funciones de tipo enum state para devolver un valor de estado. */
enum state oscuro_pocaluz(void)
{
    poca_luz();
    return PARPADEO;
}
enum state oscuro_luz(void)
{
    luz();
    return ENCENDIDO;
}
enum state pocaluz_luz(void)
{
    luz();
    return ENCENDIDO;
}
enum state pocaluz_oscuro(void)
{
    oscuridad();
    return APAGADO;
}
enum state luz_pocaluz(void)
{
    poca_luz();
    return PARPADEO;
}
enum state luz_oscuro(void)
{
    oscuridad();
    return APAGADO;
}

/* Enum state para crear la Tabla de Transiciones que, a partir del estado actual 
 * y del evento recibido, permite determinar y ejecutar la función de transición 
 * correspondiente. */
enum state (*trans_table[STATE_MAX][EVENT_MAX])(void) = {
    [APAGADO] = {
        [TENUE] = oscuro_pocaluz,
        [LUMINOSO] = oscuro_luz,
    },
    [PARPADEO] = {
        [OSCURO] = pocaluz_oscuro,
        [LUMINOSO] = pocaluz_luz
    },
    [ENCENDIDO] = {
        [TENUE] = luz_pocaluz,
        [OSCURO] = luz_oscuro
    }
};

/* El parser de eventos convierte el valor leído por el sensor de luz 
 * en eventos que la máquina de estados utiliza para decidir la acción 
 * sobre el LED. */
enum event event_parser(float intensidad)
{
    if (intensidad < 6.6){
        if ( intensidad > 0.33){
            return TENUE;}
        else return OSCURO;}

    if (intensidad >= 6.6){
        return LUMINOSO;}

    return NONE;
}

/* ------------------------------------------------------------
 * FUNCIÓN PRINCIPAL
 * ------------------------------------------------------------ */

int main(void)
{
    /* Inicializa los sistemas de entrada/salida estándar. */
    stdio_init_all();
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    /* Inicialización del bus I2C:
     * - Se configura la velocidad a 100 kHz
     * - Se asignan las funciones I2C a los pines SDA y SCL
     * - Se habilitan las resistencias pull-up necesarias para el bus. */
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    /* Retardo para asegurar inicialización. */
    sleep_ms(100);

    /* Inicializa el sensor INA219 de corriente y voltaje. */
    ina219_init();

    /*Estado inicial, LED apagado. */
    oscuridad();
    /* Corrección: El estado inicial de la variable debe coincidir con la lógica */
    enum state st = APAGADO; 

    /* Bucle for infinito para comprobar constantemente el estado del sensor de luminosidad
     * y ejecutar los consiguientes cambios en el led. */
    for (;;) {

        /* Variable donde se guarda el valor obtenido de intensidad lumínica por el sensor. */
        float intensidad = ina219_read_current_mA();

        /* Convertir la entrada de valor de luminosidad en un evento de la FSM. */
        enum event ev = event_parser(intensidad);

        /* Validación de evento nulo antes de buscar en tabla */
        if (ev == NONE) {
             sleep_ms(100);
             continue;
        }

        /* Se busca la transición válida para el estado actual y el evento producido.
         * - Transición existente: tr apunta a una función.
         * - Transición no existente: tr será NULL.         
         */
        enum state (*tr)(void) = trans_table[st][ev];

        if (tr == NULL) { 
            /* Transición no válida, se mantiene estado */
        } else {
             /* Se ejecuta la acción de transición y se actualiza el estado. */
            st = tr(); 
        }

        /* Pequeño retardo para estabilidad */
        sleep_ms(500);
    }

    return 0;
}
