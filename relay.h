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
	RelayComment comment;
};


class Relay {
	public:
		byte index;
		RelayInfo info;

		Relay();
		//~Relay(){};
		boolean begin();
		
		void load_eeprom();
		void save_eeprom();
		void save_status_eeprom();
		
		void publish(PubSubClient *mqtt_client, const char *device);
		
		void turnOFF();
		void turnON();
		
		void switch_relay(byte* payload);
		void switch_relay(Status status);

};
#endif //RELAY_H
