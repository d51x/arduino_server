#include "relay.h"
#include <EEPROM.h>

Relay::Relay() : index(255)
{
	#ifdef MEGA
	memset(info.comment, 0, sizeof(RelayComment)) ;
	#endif
	info.pin = 255;
	info.state = DISABLE; 
	info.signalType = NORMAL;
	info.to_flash = false;
	info.status = OFF;  
	this->_mqtt = NULL;
	this->_device = NULL;
	//setMqttClient(mqtt);
}

void Relay::setMqttClient(PubSubClient* mqtt, const char *device){
    this->_mqtt = mqtt;
    this->_device = device;
}

void Relay::load_eeprom() {
    int addr = EEPROM_ADDRESS_RELAY_START + index*RELAY_DATA_SIZE;
	EEPROM.get(addr, info );

	info.signalType = (info.signalType != NORMAL ) ? INVERT : NORMAL;
	info.state = (info.state != ENABLE ) ? DISABLE : ENABLE;
	info.to_flash = ( !info.to_flash  ) ? false : true;
	info.status = ( info.status != ON  ) ? OFF : ON;
}

void Relay::save_eeprom() {
    RelayInfo tmp;
	int addr = EEPROM_ADDRESS_RELAY_START + index*RELAY_DATA_SIZE;
	EEPROM.get(addr, tmp ); 
	if ( memcpy(&tmp, &info, sizeof(RelayInfo)) != 0 ) EEPROM.put(addr, info ); 
}

void Relay::save_status_eeprom() {
    Status tmp;
	int addr = EEPROM_ADDRESS_RELAY_START + index*RELAY_DATA_SIZE + 4;
	EEPROM.get(addr, tmp );
	if ( tmp != info.status ) EEPROM.put(addr, info.status);	
}

boolean Relay::begin() {
	if ( info.pin > 60 ) return false;	
	if ( info.state == DISABLE) return false;
	pinMode( info.pin, OUTPUT);

	if ( info.to_flash ) {
		digitalWrite( info.pin, (info.signalType == INVERT) ? ( (info.status == ON) ? LOW : HIGH ) : ( (info.status == ON) ? HIGH : LOW ));
		save_status_eeprom();
		return true;
	} else {
		digitalWrite(info.pin, (info.signalType == INVERT) ? HIGH : LOW);	
		return false;		
	}
	
}

void Relay::publish() {
	//(const char* topic, int value) {
  if (index == 255) return;
  byte i = strlen(_device);
  byte l = strlen(P_GPIO_OUT_STATE);
  char* buffer = (char*)malloc( i+l+1+2 );
  if ( buffer == NULL) return;
  WRITE_TO_SERIAL(F("-------- "), _device, F(""), F(""));	
  strcpy(buffer, _device);
  strcat_P(buffer, (char*)P_GPIO_OUT_STATE);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  char* s = (char*)malloc( 4 );
  strcpy_P(s, (info.status == ON) ? CONST_ON : CONST_OFF);
   WRITE_TO_SERIAL(F("relay_publish: "), (char*)buffer, F(" val "), (char*)s);
  _mqtt->publish( buffer, s);
  //mqtt_client->publish( buffer, status);
  
#ifdef DEBUG  
  WRITE_TO_SERIAL(F("Relay::publish MQTT topic: "), buffer, F(" value: "), s);	
  Serial.println();
#endif 
  
  free(buffer);
  free(s);
  
}

void Relay::turnOFF() {
	info.status = OFF;
	digitalWrite(info.pin, (info.signalType == INVERT) ? HIGH : LOW);
	if (info.to_flash) { save_status_eeprom(); }
	publish();
}

void Relay::turnON() {
	info.status = ON;
	digitalWrite(info.pin, (info.signalType == INVERT) ? LOW : HIGH);
	if (info.to_flash) { save_status_eeprom(); }
	publish();
}

void Relay::switch_relay(byte* payload) {
	  char* cstring = (char*)payload;
	  ( strcmp_P( cstring, CONST_ON) == 0 ) ? turnON() : turnOFF();
	  // (payload[0] == 1 ) ? turnON() : turnOFF();
}

void Relay::switch_relay(Status status) {
	  ( status == ON ) ? turnON() : turnOFF();
}
