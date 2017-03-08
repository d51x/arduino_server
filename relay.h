#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>
#include "types.h"
#include <PubSubClient.h>
#include "const.h"

struct RelayInfo {
	byte pin;
	State state; //   Состояние - разрешено/запрещено	
	SignalType signalType;  //    Тип сигнала - прямой или инвертный
	boolean to_flash; //   Хранить во флеш
	Status status; //   Статус - вкл/выкл
	#ifdef MEGA
	RelayComment comment;
	#endif
};


class Relay {
	private:
		PubSubClient *_mqtt;
		char *_device;
	public:
		byte index;
		RelayInfo info;

		
		
		Relay();
		//~Relay(){};
		
		boolean begin();
		void setMqttClient(PubSubClient *client, const char *device);
		
		void load_eeprom();
		void save_eeprom();
		void save_status_eeprom();
		
		void publish();
		
		void turnOFF();
		void turnON();
		
		void switch_relay(byte* payload);
		void switch_relay(Status status);

};
#endif //RELAY_H
