#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "const.h"

enum SignalType {NORMAL, INVERT};
enum Status {OFF, ON};
enum State {DISABLE, ENABLE};
enum Mode {MANUAL, AUTO};
//typedef char AddrMQTT[20];
typedef uint8_t DeviceAddress[8];

#ifdef MEGA
typedef char DswComment[15];
typedef char RelayComment[15];
typedef char ThermostatComment[15];
#endif

typedef char DeviceName[10];

struct GlobalOptions {	
  byte pin_eth;
  byte pin_onewire;
  DeviceName device_name;
  byte firstStartTimeout;
  word max_power;
  byte thermostat_refresh;
  byte temperature_refresh;
  byte eth_ip[4];
  byte eth_mac[6];
  byte mqtt_ip[4];
  byte relays_count;
  byte dsw_count;
  byte therms_count;
#ifdef MEGA  
  byte asc_count;
  byte i2c_count;
	byte pin_i2c_sda;
	byte pin_i2c_scl;
#endif	
};

#endif //TYPES_H