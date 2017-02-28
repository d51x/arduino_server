#ifndef DSW_H
#define DSW_H

#include <Arduino.h>
#include "types.h"
#include <PubSubClient.h>

struct DSW_Info {
	State state;	
	DeviceAddress address;
	DswComment comment;
};

class DSW_Temp {
	public:
		int value; 
		byte index;
		DSW_Info info;		
		
		
		DSW_Temp();
		//~DSW_Temp(){};
		
		void publish(const PubSubClient &mqtt_client, const char *device);
		void load_eeprom();
		void save_eeprom();

		
		void setState(State state);
		void setDeviceAddress(DeviceAddress address);
		void setDeviceComment(DswComment comment);
};
#endif //DSW_H
