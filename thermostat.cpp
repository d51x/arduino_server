#include "thermostat.h"
#include <EEPROM.h>

Thermostat::Thermostat() :	index(255)
{
  #ifdef MEGA
  memset(info.comment, 0, sizeof(ThermostatComment)) ;
  #endif
	info.priority = 255;
	info.state = DISABLE;
	info.status = OFF;
	info.mode = MANUAL;
	info.set_temp = -12700;
	info.step = 0;
	info.delta = 0;
	info.power = 0;
	info.idx_relay = 255;
	info.idx_temp = 255;
	this->relay = NULL;
	this->tempSensor = NULL;
	this->_mqtt = NULL;
	this->_device = NULL;	

}

void Thermostat::setMqttClient(PubSubClient* mqtt, const char *device){
    this->_mqtt = mqtt;
    this->_device = device;
}

void Thermostat::load_eeprom() {
    int addr = EEPROM_ADDRESS_THERMOSTAT_START + index*THERMOSTAT_DATA_SIZE;
	EEPROM.get(addr, info );
	
	info.mode = (info.mode != MANUAL ) ? AUTO : MANUAL;
	info.state = (info.state != ENABLE ) ? DISABLE : ENABLE;
	info.status = ( info.status != ON  ) ? OFF : ON;	
}

void Thermostat::save_eeprom() {
    int addr = EEPROM_ADDRESS_THERMOSTAT_START + index*THERMOSTAT_DATA_SIZE; 
    ThermostatInfo tmp;
	EEPROM.get(addr, tmp ); 
	if ( memcpy(&tmp, &info, sizeof(ThermostatInfo)) != 0 ) EEPROM.put(addr, info ); 	
}

void Thermostat::setRelay(Relay *rel) {
	this->relay = rel;
}

void Thermostat::setTemp(DSW_Temp *temp){
	this->tempSensor = temp;
}

void Thermostat::turnOFF() {
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("Thermostat::turnOFF "), index+1, F(" is OFF"), F(""));	
#endif 
	info.status = OFF;
	relay->turnOFF();
	//WRITE_TO_SERIAL(F("Relay::index "), relay->index+1, F(" is "), relay->info.state);	
	publish_status();

	
}

void Thermostat::turnON() {
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("Thermostat::turnON "), index+1, F(" is ON"), F(""));	

#endif 	
	info.status = ON;
	relay->turnON();	
	  //WRITE_TO_SERIAL(F("Relay::index "), relay->index+1, F(" is "), relay->info.state);	
	  
	publish_status();
}

void Thermostat::publish_state() {
	//(const char* topic, int value) {
  if (index == 255) return;
  byte i = strlen(_device);
  byte l = strlen(P_THERM_STATE);
  char* buffer = (char*)malloc( i+l+1+2 );
  if ( buffer == NULL) return;
  strcpy(buffer, _device);
  strcat_P(buffer, (char*)P_THERM_STATE);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  char* s = (char*)malloc( 4 );
  strcpy_P(s, (info.state == ENABLE) ? CONST_ON : CONST_OFF);
   //WRITE_TO_SERIAL(F("dsw_temp_publish_by_topic: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  _mqtt->publish( buffer, s);
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("publish_state MQTT topic: "), buffer, F(" value: "), s);	
#endif   
  free(buffer);
 free(s);
}

void Thermostat::publish_status() {
	//(const char* topic, int value) {
  if (index == 255) return;
  byte i = strlen(_device);
  byte l = strlen(P_THERM_STATUS);
  char* buffer = (char*)malloc( i+l+1+2 );
  if ( buffer == NULL) return;
  strcpy(buffer, _device);
  strcat_P(buffer, (char*)P_THERM_STATUS);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  char* s = (char*)malloc( 4 );
  strcpy_P(s, (info.status == ON) ? CONST_ON : CONST_OFF);
   //WRITE_TO_SERIAL(F("dsw_temp_publish_by_topic: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  _mqtt->publish( buffer, s);
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("publish_status MQTT topic: "), buffer, F(" value: "), s);	
#endif   
  free(buffer);
 free(s);
}

void Thermostat::publish_set_temp() {
  if (index == 255) return;
  byte i = strlen(_device);
  byte l = strlen(P_THERM_SET_TEMP_STATE);
  char* buffer = (char*)malloc( i+l+1+2 );
  if ( buffer == NULL) return;
  strcpy(buffer, _device);
  strcat_P(buffer, (char*)P_THERM_SET_TEMP_STATE);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  if ( info.set_temp < -10000 || info.set_temp >= 8500 ) return;
  char *temp = (char*)malloc(7);
  if ( temp == NULL) return;
  sprintf_P(temp, PSTR("%d.%02d"), (int)info.set_temp / 100, (int)(info.set_temp) % 100);
  
  
   //WRITE_TO_SERIAL(F("dsw_temp_publish_by_topic: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  _mqtt->publish( buffer, temp);
#ifdef DEBUG1
  WRITE_TO_SERIAL(F("publish_set_temp MQTT topic: "), buffer, F(" value: "), temp);	
#endif   
  free(buffer);  
  free(temp);  
}

void Thermostat::publish_mode(){
  if (index == 255 ) return;
  byte i = strlen(_device);
  byte l = strlen(P_THERM_MODE_STATE);
  char* buffer = (char*)malloc( i+l+1+2 );
  if ( buffer == NULL) return;
  strcpy(buffer, _device);
  strcat_P(buffer, (char*)P_THERM_MODE_STATE);
  sprintf_P(buffer, PSTR("%s%d"), buffer, index+1);

  char* s = (char*)malloc( 4 );
  strcpy_P(s, (info.mode == AUTO) ? CONST_ON : CONST_OFF);
   //WRITE_TO_SERIAL(F("dsw_temp_publish_by_topic: "), (char*)buffer, F("\ttemp: "), (char *)temp);
  _mqtt->publish( buffer, s);
  
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("Thermostat::publish_mode MQTT topic: "), buffer, F(" value: "), s);	
#endif 
  
  free(buffer);   
  free(s); 
}

void Thermostat::saveMode(byte* payload) {
	  char* cstring = (char*)payload;
	  info.mode = ( strcmp_P( cstring, CONST_ON) == 0 ) ?  AUTO : MANUAL ;
	  //WRITE_TO_SERIAL(F("Thermostat::saveMode: "), cstring, F(" info.mode: "), (info.mode == AUTO) ? "AUTO" : "MANUAL" );	
	  save_eeprom();
}

void Thermostat::saveState(byte* payload) {
	  char* cstring = (char*)payload;
	  info.state = ( strcmp_P( cstring, CONST_ON) == 0 ) ? ENABLE : DISABLE;
	  //WRITE_TO_SERIAL(F("Thermostat::saveState: "), cstring, F(" info.state: "), (info.mode == ENABLE) ? "ENABLE" : "DISABLE" );
	  save_eeprom();
}

void Thermostat::setState(State st) {
	  info.state = st;
	  setStatus( ( info.state == DISABLE ) ? OFF : ON);
	  save_eeprom();
}

void Thermostat::saveSetTemp(byte* payload) {
  int i = atof( (char *) payload ) * 100;
  if ( (i <= -12700) || (i >= 8500) ) return;
  info.set_temp = i;
  save_eeprom();
}

void Thermostat::setStatus(byte* payload) {
	  char* cstring = (char*)payload;
	  info.status = ( strcmp_P( cstring, CONST_ON) == 0 ) ? ON : OFF;	
	  setStatus(info.status);
}

void Thermostat::setStatus(Status st) {
		info.status = st;
		( info.status == ON ) ? relay->turnON() : relay->turnOFF();
}

void Thermostat::setMode(Mode m) {
		info.mode = m;
		save_eeprom();
}

