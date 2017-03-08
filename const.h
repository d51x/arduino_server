#ifndef CONST_H
#define CONST_H
#include <avr/pgmspace.h>
#include <Arduino.h>

#define DEBUG
#define RESET_EEPROM
#define TEST

#define MEGA

#define CONST_ON (const char *)F("ON")
#define CONST_OFF (const char *)F("OFF")

#define ONE_WIRE_BUS 8
#ifdef MEGA
	#define I2C_SDA_PIN	2
	#define I2C_SCL_PIN	0
#endif

#define FIRST_START_TIMEOUT 1//*1000
#define READ_TEMP_INTERVAL 10//*1000 //30sec
#define TERMOSTAT_DELTA 50 // i.e. 1 grad

#ifdef MEGA
	#define MAX_THERMOSTAT 10
	#define MAX_RELAY 16
	#define MAX_DSW_TEMPERATURE_SENSORS 16
	#define MAX_DSW_SENSORS 16
#else
	#define MAX_THERMOSTAT 5
	#define MAX_RELAY 5
	#define MAX_DSW_TEMPERATURE_SENSORS 10
	#define MAX_DSW_SENSORS 10	
#endif

#define DEVICE_NAME_SIZE 10

#define EEPROM_ADDRESS_GLOBAL_OPTIONS_START 0 

#define RELAY_DATA_SIZE 20
#ifdef MEGA
#define EEPROM_ADDRESS_RELAY_START 	500
#else
#define EEPROM_ADDRESS_RELAY_START 	340
#endif

#define DSW_DATA_SIZE 24
#define EEPROM_ADDRESS_DSW_START 	100 

#define THERMOSTAT_DATA_SIZE 30
#ifdef MEGA
#define EEPROM_ADDRESS_THERMOSTAT_START 	900
#else
#define EEPROM_ADDRESS_THERMOSTAT_START 	460
#endif

#ifdef MEGA
	#define I2C_DEVICE_DATA_SIZE 18
	#define EEPROM_ADDRESS_I2C_DEVICE_START 	1200
#endif


#ifdef DEBUG
  #define WRITE_TO_SERIAL(val1,val2, val3, val4) { Serial.print(val1); Serial.print(val2); Serial.print(val3); Serial.println(val4); }
  #define WRITE_TO_SERIAL_BYTE_ARR(val0, val1, val2) { Serial.print(val0); for (byte i=0;i<sizeof(val1);i++) {Serial.print(val1[i], val2); Serial.print(" ");} Serial.println();}
  #define WRITE_TO_SERIAL_CHAR_ARR(val0, val1) { Serial.print(val0); for (byte i=0;i<sizeof(val1);i++) {Serial.write( val1[i]);} Serial.println();}
#endif

#ifdef MEGA
	const char P_DEVICE_NAME[] PROGMEM         = "arduino";
#else
	const char P_DEVICE_NAME[] PROGMEM         = "uno";
#endif

const char P_DSW[] PROGMEM         = "/dsw/";

const char P_GPIO_OUT_CMD[] PROGMEM         = "/output/gpio/cmd/";
const char P_GPIO_OUT_STATE[] PROGMEM         = "/output/gpio/state/";

const char P_THERM_STATE[] PROGMEM         = "/therm/state/state/";
const char P_THERM_STATE_CMD[] PROGMEM         = "/therm/state/cmd/";
const char P_THERM_STATUS[] PROGMEM         = "/therm/status/state/";
const char P_THERM_STATUS_CMD[] PROGMEM         = "/therm/status/cmd/";
const char P_THERM_SET_TEMP_CMD[] PROGMEM         = "/therm/set/cmd/";
const char P_THERM_SET_TEMP_STATE[] PROGMEM         = "/therm/set/state/";
const char P_THERM_MODE_CMD[] PROGMEM         = "/therm/mode/cmd/";
const char P_THERM_MODE_STATE[] PROGMEM         = "/therm/mode/state/";

const char P_TOPIC_ARDUINO_RESTART[] PROGMEM           = "/restart";
const char P_TOPIC_ARDUINO_RESET[] PROGMEM           = "/reset";
const char P_TOPIC_ARDUINO_UPTIME[] PROGMEM           = "/uptime";
const char P_TOPIC_ARDUINO_MEMORY_FREE[] PROGMEM           = "/memoryfree";

#endif //CONST_H