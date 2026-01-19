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

/* Lee el voltaje del bus y lo devuelve en voltios. */
float ina219_read_bus_voltage_V(void) 
{
    /* Lectura cruda del registro de voltaje del bus. */
    int16_t raw = ina219_read_register(INA219_REG_BUS_VOLTAGE);
    
    
    /* El registro usa los bits [15:3]
     * Cada LSB equivale a 4 mV. */
    return ((raw >> 3) * 4) / 1000.0f;
}


/* enumerando de estados */
enum state {
	APAGADO = 0,
	PARPADEO,
	ENCENDIDO,
	STATE_MAX
};
/* enumerando de eventos */
enum event {
	NONE = 0,
	OSCURO,
	TENUE,
	LUMINOSO,
	EVENT_MAX
};
/* acciones */
void oscuridad(void){
	printf("sin luz")
};
void poca_luz(void){
	printf("poca luz")
};
void luz(void){
	printf("luz")
};

/* transiciones */
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
/* tabla de transiciones */
enum state (*trans_table[STATE_MAX][EVENT_MAX])(void) = {
	[APAGADO] = {
		[TENUE] = oscuro_pocaluz,
		[LUMINOSO] = oscuro_luz,
	},
	[PARPADEO] = {
		[OSCURO] = pocaluz_oscuro,
		[LUMINOSO] = pocaluz_luz
	}
	[ENCENDIDO] = {
		[TENUE] = luz_pocaluz,
		[OSCURO] = luz_oscuro
	}

};

/*parseador de eventos se le mete intensidad y se compara 
con valores para oscuro, tenue y luminoso*/

enum event event_parser(float intensidad)                     <------------- AVERIGUAR VALORES TENUE OSCURO LUMINOSO
{
	
	if (intensidad < 0.0066){
		if ( intensidad > 0.00033){
			return TENUE;}
		else return OSCURO;}

	if (intensidad >= 0.0066){
		return LUMINOSO;}
	

	return NONE;
}
int main(void)
{

	stdio_init_all();

    	/* Inicializar I2C */
    	i2c_init(I2C_PORT, 100 * 1000);
    	gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    	gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    	gpio_pull_up(I2C_SDA);
    	gpio_pull_up(I2C_SCL);

    	sleep_ms(100);

   	ina219_init();

	printf("oscuro");
	oscuridad();
	enum state st = OSCURO;

	for (;;) {
		float intensidad = ina219_read_current_mA();                         <-------- FUNCION SACA INTENSIDAD DEL SENSOR
		if (intensidad == EOF) { break; }                                    <--------- son necesarias estras dos lineas?¿?¿
		if (intensidad == '\n' || ch == '\r') { continue; }

		enum event ev = event_parser(instensidad);
		enum state (*tr)(void) = trans_table[st][ev];
		if (tr == NULL) { 
			printf("Transicion no definida (st=%d, ev=%d)\n", st, ev);
			continue;
		}
		st = tr(); 
	}

	return 0;
}

