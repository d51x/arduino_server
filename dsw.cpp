#include "dsw.h"
#include "const.h"
#include <EEPROM.h>
#include "types.h"


DSW_Temp::DSW_Temp() :	value(-12700),	index(255)
{
	 memset(info.address, 0, sizeof(DeviceAddress)) ;
	#ifdef MEGA 
	 memset(info.comment, 0, sizeof(DswComment)) ;
	#endif 
	 info.state = DISABLE;
}


void DSW_Temp::publish(const PubSubClient &mqtt_client, const char *device) {
  if ( index > (MAX_DSW_SENSORS - 1) ) return;
  if ( value <= -12700 || value >= 8500 ) return;
  char *temp = (char*)malloc(7);
  if ( temp == NULL) return;
  sprintf_P(temp, PSTR("%d.%02d"), (int)value / 100, (int)(value) % 100);
  
  byte i = strlen(device);
  byte l = strlen(P_DSW);
  char* buffer = (char*)malloc( i + l+1+2 );
  if ( buffer == NULL) return;
  
  strcpy(buffer, device);
  strcat_P(buffer, (char*)P_DSW);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  mqtt_client.publish( buffer, temp);
  
  #ifdef DEBUG1 
	WRITE_TO_SERIAL(F("MQTT topic: "), buffer, F(" value: "), temp);	
  #endif 	

  free(temp);
  free(buffer);
}

void DSW_Temp::load_eeprom() {
	int a = EEPROM_ADDRESS_DSW_START + index*DSW_DATA_SIZE;
    EEPROM.get(a, info);  
}

void DSW_Temp::save_eeprom() {
	int a = EEPROM_ADDRESS_DSW_START + index*DSW_DATA_SIZE;
    DSW_Info tmp; 
	EEPROM.get(a, tmp ); 
	if ( memcpy(&tmp, &info, sizeof(DSW_Info)) != 0 ) EEPROM.put(a, info ); 
  #ifdef DEBUG1 	
	WRITE_TO_SERIAL(F("DSW_Temp::save_eeprom state "), (State)info.state, F(" tmp: "), (State)tmp.state);	
	WRITE_TO_SERIAL(F("DSW_Temp::save_eeprom address "), (char *)info.address, F(" tmp: "), (char *)tmp.address);	
	WRITE_TO_SERIAL(F("DSW_Temp::save_eeprom state "), (char *)info.state, F(" tmp: "), (char *)tmp.state);	
  #endif 
}
		
		
void DSW_Temp::setState(State st) {
	info.state = st;
	save_eeprom();	
}

void DSW_Temp::setDeviceAddress(DeviceAddress addr) {
	memcpy(info.address, addr, sizeof(DeviceAddress));
	save_eeprom();	
}

#ifdef MEGA
void DSW_Temp::setDeviceComment(DswComment comm) {
	memcpy(info.comment, comm, sizeof(DswComment));
	save_eeprom();	
}
#endif
