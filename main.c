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

/* Funciones para el estado del led. */
void oscuridad(void){
    gpio_put(2, 0);
    sleep_ms(500);
}
void poca_luz(void){
    /* blink_led_forever(2,100);*/
    
}
void luz(void){
    gpio_put(2, 1);
    sleep_ms(500);
}
void blink_led_forever(uint pin, uint period_ms) {
    while (true) {
        gpio_put(pin, !gpio_get(pin));
        sleep_ms(period_ms);
    }
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
