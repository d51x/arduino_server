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
	private:
		PubSubClient *_mqtt;
		char *_device;
		
	public:
	    byte index;
		ThermostatInfo info;
		Relay *relay;
		DSW_Temp *tempSensor;

		Thermostat();
		//~Thermostat(){};
		
		void setMqttClient(PubSubClient *client, const char *device);
		void setRelay(Relay *relay);
		void setTemp(DSW_Temp *tempSensor);
		
		void load_eeprom();
		void save_eeprom();

		void publish_state();
		void publish_status();
		void publish_set_temp();
		void publish_mode();
		void turnOFF();
		void turnON();
		void saveMode(byte* payload);
		void setMode(Mode mode);
		void saveState(byte* payload);
		void setState(State state);
		void saveSetTemp(byte* payload);
		void setStatus(byte* payload);
		void setStatus(Status st);
};
#endif //THERMOSTAT_H
