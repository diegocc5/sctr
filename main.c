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
#include "hardware/adc.h"
/* ------------------------------------------------------------
 * CONFIGURACIÓN HARDWARE
 * ------------------------------------------------------------ */

/* Pin del LED (Actuador) */
#define LED_PIN 2

/* Pin del Sensor (Entrada Analógica) */
#define ADC_PIN 26      // GP26 corresponde al canal ADC 0
#define ADC_CHANNEL 0

/* Factor de conversión: 3.3V / 4095 (12 bits) */
const float CONVERSION_FACTOR = 3.3f / (1 << 12);


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
    gpio_put(LED_PIN, 1); // encender LED físico
}

void poca_luz(void){
    /* Simulación de parpadeo rápido  ESTO PARPADEA 1 VEZ?¿¿ */   
    gpio_put(LED_PIN, 1); sleep_ms(100);
    gpio_put(LED_PIN, 0); sleep_ms(100);
}

void luz(void){
    gpio_put(LED_PIN, 0); // apagar LED físico
}

void print_adc_voltage(float voltaje){
    printf("Voltaje LDR: %.2f V\n", voltaje);
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
enum event event_parser(float tension)
{
    if (voltaje < 1.0f) {
        return OSCURO;
    } else if (voltaje >= 1.0f && voltaje < 2.5f) {
        return TENUE;
    } else {
        return LUMINOSO;
    }


    return NONE;
}

/* ------------------------------------------------------------
 * FUNCIÓN PRINCIPAL
 * ------------------------------------------------------------ */

int main(void)
{
    /* Inicializa los sistemas de entrada/salida estándar. */
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    adc_init();
    adc_gpio_init(ADC_PIN); // Prepara el pin GP26 para leer analógico
    adc_select_input(ADC_CHANNEL); // Selecciona el canal 0



    /* Retardo para asegurar inicialización. */
    sleep_ms(100);

    
    

    /*Estado inicial, LED apagado. */
    oscuridad();
    /* Corrección: El estado inicial de la variable debe coincidir con la lógica */
    enum state st = APAGADO; 

    /* Bucle for infinito para comprobar constantemente el estado del sensor de luminosidad
     * y ejecutar los consiguientes cambios en el led. */
    for (;;) {

        /* Leer el valor crudo del ADC (0-4095) */
        uint16_t raw_value = adc_read();

         /* Convertir a Voltaje (0.0V - 3.3V) */
        float voltaje = raw_value * CONVERSION_FACTOR;

        // Imprimir en consola
        print_adc_voltage(voltaje);

        /* Convertir la entrada de valor de luminosidad en un evento de la FSM. */
        enum event ev = event_parser(voltaje);

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
