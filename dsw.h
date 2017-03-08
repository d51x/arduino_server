#ifndef DSW_H
#define DSW_H

#include <Arduino.h>
#include "types.h"
#include <PubSubClient.h>

struct DSW_Info {
	State state;	
	DeviceAddress address;
	#ifdef MEGA
	DswComment comment;
	#endif
};

class DSW_Temp {
	private:
		PubSubClient *_mqtt;
		char *_device;
	public:
		int value; 
		byte index;
		DSW_Info info;		
		
		
		DSW_Temp();
		//~DSW_Temp(){};
		void setMqttClient(PubSubClient *client, const char *device);
		
		void publish();
		void load_eeprom();
		void save_eeprom();

		
		void setState(State state);
		void setDeviceAddress(DeviceAddress address);
		#ifdef MEGA
		void setDeviceComment(DswComment comment);
		#endif
};
#endif //DSW_H
