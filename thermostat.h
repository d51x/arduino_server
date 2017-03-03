#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <Arduino.h>
#include "types.h"
#include "relay.h"
#include "dsw.h"
#include <PubSubClient.h>

struct ThermostatInfo {
		State state;  // disable / enable	
		Status status;  // off / on
		byte priority;
		Mode mode; // manual / auto
		byte idx_relay;
		byte idx_temp;
		int set_temp;  // установка температуры * 100
		byte step;   // шаг ругилировки * 10, т.е. 5 * 10 = 50 --> 0,5 градуса
		byte delta;  // дельта от set_temp, значение * 10, т.е. 5 * 10 = 50 --> 0,5 градуса 
		word power; // мощность термостата, задается руками
		#ifdef MEGA
		ThermostatComment comment;
		#endif
};

class Thermostat {
	public:
	    byte index;
		ThermostatInfo info;
		Relay *relay;
		DSW_Temp *tempSensor;
		PubSubClient *mqtt;

		Thermostat();
		//~Thermostat(){};
		
		void setRelay(Relay *relay);
		void setTemp(DSW_Temp *tempSensor);
		
		void load_eeprom();
		void save_eeprom();

		void publish_state(PubSubClient *mqtt_client, const char *device);
		void publish_status(PubSubClient *mqtt_client, const char *device);
		void publish_set_temp(PubSubClient *mqtt_client, const char *device);
		void publish_mode(PubSubClient *mqtt_client, const char *device);
		void turnOFF(const char *device);
		void turnON(const char *device);
		void saveMode(byte* payload);
		void setMode(Mode mode);
		void saveState(byte* payload);
		void setState(State state, const char *device);
		void saveSetTemp(byte* payload);
		void setStatus(byte* payload, const char *device);
		void setStatus(Status st, const char *device);
};
#endif //THERMOSTAT_H
