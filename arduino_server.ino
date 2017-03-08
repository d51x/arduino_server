#include <EEPROM.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include "dsw.h"
#include "types.h"
#include "const.h"
#include "relay.h"
#include "thermostat.h"

#ifdef MEGA
//+++++#include "http.h"
#endif
//#include <SD.h>

//File webFile;

extern int __bss_end;
extern void *__brkval;

#define RESET_EEPROM1

Thermostat thermostat[MAX_THERMOSTAT];
Relay relay[MAX_RELAY];
DSW_Temp dsw_temp[MAX_DSW_TEMPERATURE_SENSORS];


OneWire  ds;
DallasTemperature temp_sensors;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte eth_ip[] = {192, 168, 1, 177};
byte mqtt[] = {192, 168, 1, 45};

IPAddress ip(eth_ip);
IPAddress mqtt_server(mqtt);

byte isRestarted = 1;

long lastReadingTime = 0;
long lastReadingTime2 = 0;
long lastReadingTime3 = 0;
long lastReadingTime4 = 0;

#ifdef MEGA
EthernetServer server(80);
#endif

EthernetClient ethClient;
PubSubClient mqtt_client(ethClient);

//String HTTP_request;

GlobalOptions options;

void(* resetFunc) (void) = 0;

int memoryFree()
{
   int freeValue;
   if((int)__brkval == 0)
      freeValue = ((int)&freeValue) - ((int)&__bss_end);
   else
      freeValue = ((int)&freeValue) - ((int)__brkval);
   return freeValue;
}


void load_options() {
#ifdef RESET_EEPROM1
	#ifdef MEGA
		for (word ee=0;ee<4096;ee++) EEPROM.write(ee, 0);
	#else
		for (word ee=0;ee<1024;ee++) EEPROM.write(ee, 0);
	#endif
#endif
	
#ifdef DEBUG1
	WRITE_TO_SERIAL(F("-------- "), F("function load_options"), F(" start"), F(""));	
#endif

	EEPROM.get(EEPROM_ADDRESS_GLOBAL_OPTIONS_START, options);	
	
	if ( (options.pin_onewire < 2) || (options.pin_onewire > 54)) options.pin_onewire = ONE_WIRE_BUS;
	if (( options.device_name[0] == 0 ) || (options.device_name[0] == 255)) strcpy_P(options.device_name, P_DEVICE_NAME); 
	if ( (options.firstStartTimeout == 255) || (options.firstStartTimeout == 0) ) options.firstStartTimeout = FIRST_START_TIMEOUT;
	if ( options.max_power == 16000 ) options.max_power = 0;
	if ( (options.thermostat_refresh == 255) || (options.thermostat_refresh == 0) ) options.thermostat_refresh = 10;
	if ( (options.temperature_refresh == 255) || (options.temperature_refresh == 0) ) options.temperature_refresh = READ_TEMP_INTERVAL;
	byte t0[6] = {0, 0, 0, 0, 0, 0};
	byte t1[6] = {255, 255, 255, 255, 255, 255};
	if ( (memcpy(options.eth_ip, t0, 4) == 0 ) 	|| (memcpy(options.eth_ip, t1, 4)) )	memcpy(options.eth_ip, eth_ip, sizeof(eth_ip));
	if ( (memcpy(options.mqtt_ip, t0, 4) == 0 ) || (memcpy(options.mqtt_ip, t1, 4)) ) 	memcpy(options.mqtt_ip, mqtt, sizeof(mqtt));
	if ( (memcpy(options.eth_mac, t0, 6) == 0 ) || (memcpy(options.eth_mac, t1, 6)) ) 	memcpy(options.eth_mac, mac, sizeof(mac));
	
	if ( (options.relays_count > 16) ) options.relays_count = 16;
	if ( (options.dsw_count > 16) ) options.dsw_count = 16;
	if ( (options.therms_count > 10) ) options.therms_count = 10;
	#ifdef MEGA
		if ( (options.asc_count > 6) ) options.asc_count = 6;
		if ( (options.i2c_count > 8) ) options.i2c_count = 8;
		if ( (options.pin_i2c_sda < 0) || (options.pin_i2c_sda > 54) ) options.pin_i2c_sda = I2C_SDA_PIN;
		if ( (options.pin_i2c_scl < 0) || (options.pin_i2c_scl > 54) ) options.pin_i2c_scl = I2C_SCL_PIN;
    #endif
	
#ifdef DEBUG1
	WRITE_TO_SERIAL(F("--------- "), F(" loaded options "), F(" "), F(""));
	WRITE_TO_SERIAL(F("OneWire Pin: "), options.pin_onewire, F(" "), F(""));
	WRITE_TO_SERIAL(F("Device name: "), (char*)options.device_name, F(" "), F(""));
	WRITE_TO_SERIAL(F("firstStartTimeout: "), options.firstStartTimeout, F("    max_power: "), options.max_power);	
	WRITE_TO_SERIAL(F("thermostat_refresh: "), options.thermostat_refresh, F("   temperature_refresh: "), options.temperature_refresh);
		
	WRITE_TO_SERIAL_BYTE_ARR(F("eth_ip: "), options.eth_ip, DEC);
	WRITE_TO_SERIAL_BYTE_ARR(F("eth_mac: "), options.eth_mac, HEX);
	WRITE_TO_SERIAL_BYTE_ARR(F("mqtt_ip: "), options.mqtt_ip, DEC);
	
	WRITE_TO_SERIAL(F("relays_count: "), options.relays_count, F("   therms_count: "), options.therms_count);
	WRITE_TO_SERIAL(F("dsw_count: "), options.dsw_count, F(" "), F(""));
	
	#ifdef MEGA
	  WRITE_TO_SERIAL(F("asc_count: "), options.asc_count, F("   i2c_count: "), options.i2c_count);
	  WRITE_TO_SERIAL(F("i2c sda pin: "), options.pin_i2c_sda, F("   i2c scl pin: "), options.pin_i2c_scl);
	#endif
	
	WRITE_TO_SERIAL(F("-------- "), F("function load_options"), F(" end"), F(""));	
#endif

  
#ifdef TEST
	options.pin_onewire = ONE_WIRE_BUS;
	strcpy_P(options.device_name, P_DEVICE_NAME);
	options.max_power = 3500;
	options.thermostat_refresh = 10;
	options.temperature_refresh = READ_TEMP_INTERVAL;
	memcpy(options.eth_ip, eth_ip, sizeof(eth_ip));
	memcpy(options.mqtt_ip, mqtt, sizeof(mqtt));
	memcpy(options.eth_mac, mac, sizeof(mac));
	options.relays_count = MAX_RELAY;
	options.dsw_count = MAX_DSW_TEMPERATURE_SENSORS;
	options.therms_count = MAX_THERMOSTAT;	
	#ifdef MEGA
	options.asc_count = 0;
	options.i2c_count = 0;
	options.pin_i2c_sda = I2C_SDA_PIN;
	options.pin_i2c_scl = I2C_SCL_PIN;
	#endif
#endif

}

void init_temperature_sensors() {
#ifdef DEBUG1
	WRITE_TO_SERIAL(F("-------- "), F("function init_temperature_sensors"), F(" start"), F(""));	
#endif 	

	for (byte i=0;i<options.dsw_count;i++) {
		dsw_temp[i].index = i;
		dsw_temp[i].load_eeprom();
		dsw_temp[i].setMqttClient(&mqtt_client, options.device_name);
		#ifdef DEBUG1
			WRITE_TO_SERIAL(F("dsw sensor: "), dsw_temp[i].index+1, F(" state "), dsw_temp[i].info.state);
			WRITE_TO_SERIAL_BYTE_ARR(F("address: "), dsw_temp[i].info.address, HEX);
			#ifdef MEGA
			WRITE_TO_SERIAL_CHAR_ARR(F("comment: "), (char *)dsw_temp[i].info.comment);	
			#endif
		#endif		
	}
	

	
#ifdef TEST 
	byte addr_f1_kitchen[8] = {0x28, 0xFF, 0xB8, 0xBA, 0x73, 0x16, 0x05, 0xC1};
	memcpy(dsw_temp[0].info.address, addr_f1_kitchen, 8); 	
	dsw_temp[0].info.state = ENABLE;
	
	byte addr_f1_living[8] = {0x28, 0xFF, 0x81, 0xE9, 0x74, 0x16, 0x03, 0x41};
	memcpy(dsw_temp[1].info.address, addr_f1_living, 8); 
	dsw_temp[1].info.state = ENABLE;
	
	byte addr_f1_boiler_room[8] = {0x28, 0xFF, 0x9B, 0xC3, 0x73, 0x16, 0x05, 0x4C};
	memcpy(dsw_temp[2].info.address, addr_f1_boiler_room, 8); 
	dsw_temp[2].info.state = ENABLE;	
	
	memcpy(dsw_temp[3].info.address, addr_f1_kitchen, 8); 	
	dsw_temp[3].info.state = ENABLE;
	
	memcpy(dsw_temp[4].info.address, addr_f1_living, 8); 
	dsw_temp[4].info.state = ENABLE;

	#ifdef MEGA
	strcpy(dsw_temp[0].info.comment, "kitchen");
	strcpy(dsw_temp[1].info.comment, "living room");
	strcpy(dsw_temp[2].info.comment, "boiler room");
	strcpy(dsw_temp[3].info.comment, "room 2.1");
	strcpy(dsw_temp[4].info.comment, "room 2.2");
	#endif
	
#endif
	
#ifdef DEBUG1 
	WRITE_TO_SERIAL(F("-------- "), F("---------------------"), F(" end"), F(""));
	for (byte i=0;i<options.dsw_count;i++) {
			WRITE_TO_SERIAL(F("dsw sensor: "), dsw_temp[i].index+1, F(" state "), dsw_temp[i].info.state);
			WRITE_TO_SERIAL_BYTE_ARR(F("address: "), dsw_temp[i].info.address, HEX);
			#ifdef MEGA
			//WRITE_TO_SERIAL_CHAR_ARR(F("comment: "), dsw_temp[i].info.comment);	
			Serial.println(dsw_temp[i].info.comment);
			#endif
	} 
  WRITE_TO_SERIAL(F("-------- "), F("function init_temperature_sensors"), F(" end"), F(""));	
 #endif 	  
}

void init_relays() {
  for (byte i=0;i<options.relays_count;i++) {
	relay[i].index = i;    
    relay[i].load_eeprom();
	relay[i].setMqttClient(&mqtt_client, options.device_name);
	if (relay[i].begin())	delay(options.firstStartTimeout * 1000); 	
  }

#ifdef DEBUG1
  for (byte i=0;i<options.relays_count;i++) {
	WRITE_TO_SERIAL(F("Relay: "), relay[i].index+1, F(" pin "), relay[i].info.pin);  
	WRITE_TO_SERIAL(F("status: "), relay[i].info.status, F(" state "), relay[i].info.state);  
	WRITE_TO_SERIAL(F("flash: "), relay[i].info.to_flash, F(" signal "), relay[i].info.signalType);  
	#ifdef MEGA
	WRITE_TO_SERIAL_CHAR_ARR(F("comment: "), (char *)relay[i].info.comment);
	#endif
  }
#endif
  
#ifdef TEST  
  Serial.println("Setting up ... Relay test data start....");
  relay[0].info.pin = 2;  
  relay[1].info.pin = 3; 	
  relay[2].info.pin = 5;   	
  relay[3].info.pin = 6;   	
  relay[4].info.pin = 7;   	
  
  for (byte i=0;i<5;i++) {
	relay[i].info.state = ENABLE;	 
	relay[i].info.to_flash = true;
	relay[i].info.signalType = ( i == 4 ) ? NORMAL : INVERT;
	if (relay[i].begin())	{
		delay(options.firstStartTimeout * 1000); 	
	}
  }

#endif

#ifdef DEBUG1
  Serial.println("Setting up ... Relay test data end....");
  for (byte i=0;i<options.relays_count;i++) {
	WRITE_TO_SERIAL(F("Relay: "), relay[i].index+1, F(" pin "), relay[i].info.pin);  
	WRITE_TO_SERIAL(F("status: "), relay[i].info.status, F(" state "), relay[i].info.state);  
	WRITE_TO_SERIAL(F("flash: "), relay[i].info.to_flash, F(" signal "), relay[i].info.signalType);  
	#ifdef MEGA
	WRITE_TO_SERIAL_CHAR_ARR(F("comment: "), (char *)relay[i].info.comment);
	#endif
  }
#endif
  
}


void init_thermostats() {
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function init_thermostats"), F(" start"), F(""));	
#endif 	
  for (byte i=0;i<options.therms_count;i++) {
	thermostat[i].index = i;  
	thermostat[i].setMqttClient(&mqtt_client, options.device_name);
	
    thermostat[i].load_eeprom();
	if ( thermostat[i].info.idx_relay < options.relays_count) {
		// !!!!! костыль, relay_index надо заполнять через web-интерфейс, по дефолту прописывать 255 и делать проверку на это
		byte relay_index = i; //thermostat[i].info.idx_relay;
		//WRITE_TO_SERIAL(F("Relay INDEX: "), relay_index, F(" index "), relay[i].index);  
		//Serial.println(relay[i].index);
		thermostat[i].setRelay(  &relay[relay_index] );
		//Serial.println(thermostat[i].relay->index);
	}
	if ( thermostat[i].info.idx_temp < options.dsw_count) {
		/// !!! костыль, аналогично выше
		byte temp_index = i; // thermostat[i].info.idx_temp
		thermostat[i].setTemp(  &dsw_temp[temp_index] );
	}
  }

  
  
#ifdef TEST
	 //srand( time(0) );
	 for (byte i=0;i<5;i++) { 
		//therm[i].setRelay(rel[i]);	
		//therm[i].setTemp(dsw_temp[i]);   
		thermostat[i].info.priority = (i+1);
		thermostat[i].info.set_temp = 2000; 
		thermostat[i].info.delta = 5; 
		thermostat[i].info.step = 5; 
		thermostat[i].info.power = 1000; 
		thermostat[i].info.state = ENABLE; 
		thermostat[i].info.status = ON; 
		thermostat[i].info.mode = AUTO; 
	 }
#endif  

#ifdef DEBUG1
  for (byte i=0;i<options.therms_count;i++) {
	WRITE_TO_SERIAL(F("Therm: "), thermostat[i].index+1, F(" state "), thermostat[i].info.state);  
	WRITE_TO_SERIAL(F("status: "), thermostat[i].info.status, F(" priority "), thermostat[i].info.priority);  
	WRITE_TO_SERIAL(F("mode: "), thermostat[i].info.mode, F(" power "), thermostat[i].info.power);  
	WRITE_TO_SERIAL(F("set_temp: "), thermostat[i].info.set_temp, F(" delta "), thermostat[i].info.delta);  
	#ifdef MEGA
	WRITE_TO_SERIAL_CHAR_ARR(F("comment: "), (char *)thermostat[i].info.comment);
	#endif
  }
  
  WRITE_TO_SERIAL(F("-------- "), F("function init_thermostats"), F(" start"), F(""));	
   
#endif
 
}


void publish_all_data() {
	//relay states
  for (byte i=0;i<options.relays_count;i++) {
	if (relay[i].info.state == ENABLE)
		relay[i].publish();
  }	
  
  for (byte i=0;i<options.therms_count;i++) {
	thermostat[i].publish_state();
	thermostat[i].publish_status();
	if (thermostat[i].info.state == ENABLE)	thermostat[i].publish_set_temp();
	thermostat[i].publish_mode();
  }		
	
}

void callback(char* topic, byte* payload, unsigned int length) {
  byte* payload2 = NULL;
  payload[length] = '\0';
  payload2 = (byte*)malloc(length+1);
  if ( NULL == payload2 ) return;
  memcpy(payload2, payload, length+1);
  
#ifdef DEBUG1 
  WRITE_TO_SERIAL(F("callback: "), topic, F(" value "), (char *)payload);	
#endif 
  
  /* topic  -  <device_name>/mqtt_path/idx */
  char dev[DEVICE_NAME_SIZE+1] = "";
  char path[30] = "";
  char idx[5] = "";
  byte i = 255;
  
  char *tmp = strchr(topic, '/');
  char pos;
  if ( tmp == NULL ) return;
  pos = tmp - topic + 1;
  strncpy(dev, topic, pos-1);
  
  char *tmp2 = strrchr(tmp, '/');
  
  if (strcmp(tmp, tmp2) == 0 ) {
    // printf("idx is absent\n");
    strcpy(path, tmp2);
  } else {
    pos = tmp2 - tmp + 1;
    strncpy(path, tmp, pos);
    strncpy(idx, &tmp2[1], strlen(&tmp2[1]));
	i = atoi(idx);
  }

    if ( strcmp_P(path, P_GPIO_OUT_CMD ) == 0) { 
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("callback: "), path, F(" idx "), i);	
  WRITE_TO_SERIAL(F("swtch relay: "), i, F(" to "), (char*)payload2);	
#endif 		
      if ( i == 255 ) return;
	  relay[i-1].switch_relay(payload2);
    } else if (strcmp_P(path, P_THERM_MODE_CMD ) == 0) {
		#ifdef DEBUG1  
		  WRITE_TO_SERIAL(F("callback: "), path, F(" idx "), i);	
		  WRITE_TO_SERIAL(F("THERM_MODE: "), i, F(" to "), (char*)payload2);	
		#endif 	
		if ( i == 255 ) return;
		thermostat[i-1].saveMode(payload2);
	} else if (strcmp_P(path, P_THERM_STATE_CMD ) == 0) {
		#ifdef DEBUG1  
		  WRITE_TO_SERIAL(F("callback: "), path, F(" idx "), i);	
		  WRITE_TO_SERIAL(F("THERM_STATE: "), i, F(" to "), (char*)payload2);	
		#endif 			
		if ( i == 255 ) return;
		thermostat[i-1].saveState(payload2);
	} else if (strcmp_P(path, P_THERM_STATUS_CMD ) == 0) {
		#ifdef DEBUG1  
		  WRITE_TO_SERIAL(F("callback: "), path, F(" idx "), i);	
		  WRITE_TO_SERIAL(F("THERM_STATUS: "), i, F(" to "), (char*)payload2);	
		#endif 			
		if ( i == 255 ) return;
		thermostat[i-1].setStatus(payload2);
	} else if (strcmp_P(path, P_THERM_SET_TEMP_CMD ) == 0) {
		#ifdef DEBUG1  
		  WRITE_TO_SERIAL(F("callback: "), path, F(" idx "), i);	
		  WRITE_TO_SERIAL(F("THERM_SET_TEMP: "), i, F(" to "), (char*)payload2);	
		#endif 			
		if ( i == 255 ) return;
		thermostat[i-1].saveSetTemp(payload2);
	}	
	else if (strcmp_P(path, P_TOPIC_ARDUINO_RESET) == 0) {
		resetFunc();
	}
  free(payload2);  
};

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect( options.device_name)) {
      //Serial.println("connected");
      // Once connected, publish an announcement...
      //mqtt_client.publish("outTopic","hello world");
      // ... and resubscribe

      //relay_state_publish( TOPIC_RELAY_F1_LIVING_STATE, Relay01_state);
	  publish_all_data();
	  char *s =(char*)malloc( strlen(options.device_name + 3) );
	  strcpy(s, options.device_name);
	  strcat(s, "/#");
		#ifdef DEBUG1  
		  WRITE_TO_SERIAL(F("reconnect mqtt: "), s, F(""), F(""));	
		#endif 		  
      mqtt_client.subscribe( s );
	  //mqtt_client.subscribe( "uno/#" );
      free(s);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  } 
}

void setup() {
	//for (int t=0;t<1024;t++) EEPROM.write(t, 255);
#ifdef DEBUG1
	Serial.begin(9600);
	while (!Serial) {
		//  ; // wait for serial port to connect. Needed for native USB port only
	}  
	WRITE_TO_SERIAL(F("----------- "), F(" setup() "), F(" ------------ "), F(""));
#endif 

    // initialize SD card
/*     Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        //return;    // init failed
    }
 */
  delay(100);
  
  load_options();

#ifdef DEBUG1
	WRITE_TO_SERIAL(F("--------- "), F(" loaded options "), F(" "), F(""));
	WRITE_TO_SERIAL(F("OneWire Pin: "), options.pin_onewire, F(" "), F(""));
	WRITE_TO_SERIAL(F("Device name: "), (char*)options.device_name, F(" "), F(""));
	WRITE_TO_SERIAL(F("firstStartTimeout: "), options.firstStartTimeout, F(" "), F(""));	
	WRITE_TO_SERIAL(F("max_power: "), options.max_power, F(" "), F(""));
	WRITE_TO_SERIAL(F("thermostat_refresh: "), options.thermostat_refresh, F(" "), F(""));
	WRITE_TO_SERIAL(F("temperature_refresh: "), options.temperature_refresh, F(" "), F(""));
	
	WRITE_TO_SERIAL_BYTE_ARR(F("eth_ip: "), options.eth_ip, DEC);
	WRITE_TO_SERIAL_BYTE_ARR(F("eth_mac: "), options.eth_mac, HEX);
	WRITE_TO_SERIAL_BYTE_ARR(F("mqtt_ip: "), options.mqtt_ip, DEC);
	
	WRITE_TO_SERIAL(F("relays_count: "), options.relays_count, F(" "), F(""));
	WRITE_TO_SERIAL(F("dsw_count: "), options.dsw_count, F(" "), F(""));
	WRITE_TO_SERIAL(F("thermo_count: "), options.therms_count, F(" "), F(""));
	#ifdef MEGA
	WRITE_TO_SERIAL(F("asc_count: "), options.asc_count, F(" "), F(""));
	WRITE_TO_SERIAL(F("i2c_count: "), options.i2c_count, F(" "), F(""));
	#endif
	WRITE_TO_SERIAL(F("-------- "), F("function load_options"), F(" end"), F(""));	
#endif


  
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  
  Ethernet.begin(mac, ip);
  #ifdef MEGA
  server.begin();
  #endif

   ds.begin(options.pin_onewire);
   temp_sensors = DallasTemperature(&ds);
  
  temp_sensors.begin();
  delay(50);
  
   // initialize dsw sensors
  init_temperature_sensors();
  delay(50);
  
  // initialize relays
  init_relays();
  delay(50);
  
  // initialize termostats
  init_thermostats(); 
  delay(50);
  
  if (mqtt_client.connected()) {
    // publish
	publish_all_data();
  }
}

void temperature_update() {
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function temperature_update"), F(" start"), F(""));	
#endif 	
  temp_sensors.requestTemperatures();
  for (byte i=0;i<MAX_DSW_TEMPERATURE_SENSORS;i++) {
    if (dsw_temp[i].info.state == ENABLE) {
		//WRITE_TO_SERIAL(F("Sensor "), i, F(" state "), F("ENABLE "));	
      dsw_temp[i].value = (int)( (float)temp_sensors.getTempC(dsw_temp[i].info.address) * 100);
/* 		#ifdef DEBUG  
			char tmp[8] = "";
			Serial.print("Sensor ");	
			for (uint8_t j = 0; j < 8; j++)
			{
				Serial.print("0x");
				if (dsw_temp[i].addr[j] < 0x10) Serial.print("0");
				Serial.print(dsw_temp[i].addr[j], HEX);
				if (j < 7) Serial.print(", ");
			}
			WRITE_TO_SERIAL(F(""), F(""), F(" temp: "), dsw_temp[i].value);	
		#endif 	 */ 	  
      dsw_temp[i].publish();
    }  
  }
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function temperature_update"), F(" end"), F(""));	
#endif 	  
}

void free_memory_publish() {
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function free_memory_publish"), F(" start"), F(""));	
#endif 	
  int m = memoryFree();

  byte i = strlen(options.device_name);
  byte l = strlen(P_TOPIC_ARDUINO_MEMORY_FREE);
  char* buffer = (char*)malloc( i + l+1+2 );
  if ( buffer == NULL) return;
  
  strcpy(buffer, options.device_name);
  strcat_P(buffer, (char*)P_TOPIC_ARDUINO_MEMORY_FREE);

  char mm[5];
  itoa(m, mm, 10);
  mqtt_client.publish( buffer, mm);
  
  #ifdef DEBUG1 
	WRITE_TO_SERIAL(F("MQTT topic: "), buffer, F(" value: "), mm);	
  #endif 	

  free(buffer);

  
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function free_memory_publish"), F(" end"), F(""));	
#endif 	 	
	
}


void thermostat_sort_by_priority() {
	return;
 #ifdef DEBUG1 
  WRITE_TO_SERIAL(F("thermostat_sort_by_priority...."), F(""), F(""), F(""));	
#endif 
	
  Thermostat tmp;
  bool exit = false;

	while (!exit) {
		exit = true;
		for (byte i=0;i<options.therms_count-1;i++) {
			if (thermostat[i].info.priority < thermostat[i+1].info.priority) // сравниваем два соседних элемента
				{
				 // выполняем перестановку элементов массива
				 memcpy((Thermostat*)&tmp, (Thermostat*)&thermostat[i], sizeof(Thermostat));
				 memcpy((Thermostat*)&thermostat[i], (Thermostat*)&thermostat[i+1], sizeof(Thermostat));
					memcpy((Thermostat*)&thermostat[i+1], (Thermostat*)&tmp, sizeof(Thermostat));
				 exit = false; // на очередной итерации была произведена перестановка элементов
				}		
		}
}

 #ifdef DEBUG1
  Serial.println(F("Result: "));
  for (byte i=0;i<options.therms_count;i++) {
	  Serial.print("i: ");   Serial.print(i);   Serial.print(" therm: ");   Serial.print(thermostat[i].index);   Serial.print(" priority: ");   Serial.print(thermostat[i].info.priority); 
	    Serial.print("relay: ");   Serial.print( thermostat[i].relay->index); 
		  Serial.print("dsw: ");   Serial.print(thermostat[i].tempSensor->index); 
		  Serial.println(); 
  }
#endif   
}



void thermostat_sort_by_index() {
	return;
 #ifdef DEBUG1 
  WRITE_TO_SERIAL(F("thermostat_sort_by_index...."), F(""), F(""), F(""));	
#endif 	
  Thermostat tmp;
  bool exit = false;
	while (!exit) {
		exit = true;
		for (byte i=0;i<options.therms_count-1;i++) {
			if (thermostat[i].index > thermostat[i+1].index) // сравниваем два соседних элемента
				{
				 // выполняем перестановку элементов массива
				 memcpy((Thermostat*)&tmp, (Thermostat*)&thermostat[i], sizeof(Thermostat));
				 memcpy((Thermostat*)&thermostat[i], (Thermostat*)&thermostat[i+1], sizeof(Thermostat));
					memcpy((Thermostat*)&thermostat[i+1], (Thermostat*)&tmp, sizeof(Thermostat));
				 exit = false; // на очередной итерации была произведена перестановка элементов
				}		
		}
	}
  
 #ifdef DEBUG1 
  Serial.println(F("Result: "));
  for (byte i=0;i<options.therms_count;i++) {
		WRITE_TO_SERIAL(F("therm "), thermostat[i].index, F(" priority "), thermostat[i].info.priority);	
  }
#endif    
}
 
 
void termostat_update() {
	return;
#ifdef DEBUG1  
  WRITE_TO_SERIAL(F("-------- "), F("function termostat_update"), F(" start"), F(""));	
#endif 	
	
  
// Массив термостатов
// Период обновления
// Макс мощность
// Суммарная включенная мощность по статусу и состоянию

// Кол-во термостатов максимальное, чтобы не превышать нагрузку


// 1. Смотрим все включенные и принимаем решение, какой оставить, какой выключить

 #ifdef DEBUG1  
  WRITE_TO_SERIAL(F("turnoff thermostats...."), F(""), F(""), F(""));	
#endif 
  for (byte i=0;i<options.therms_count;i++) {
	//Thermostat t = thermostat[i];
    if ( thermostat[i].info.state == DISABLE ) continue;
	if ( thermostat[i].info.mode == MANUAL ) continue;
	if ( thermostat[i].info.status == OFF ) continue;
	int temp = thermostat[i].tempSensor->value;
	if ( ( temp <= -10000 || temp >= 8500 ) ||
	     ( thermostat[i].tempSensor->info.state == DISABLE ) )
			thermostat[i].turnOFF();
	int set_temp = thermostat[i].info.set_temp;
	byte delta =thermostat[i].info.delta*10;
	int max_temp = set_temp + delta;
	//int min_temp = set_temp - delta * 10;
	 
 #ifdef DEBUG1  
  WRITE_TO_SERIAL(F("therm "), i+1, F(" temp "), temp);	
  WRITE_TO_SERIAL(F("set_temp "), set_temp, F(" delta "), delta);
  WRITE_TO_SERIAL(F("max_temp "), max_temp, F(" "), F(" "));
  
#endif 
	
	if ( temp > max_temp ) thermostat[i].turnOFF();
  }
  

// 
// Цикл 2 по нижнему приоритету в режиме авто
// 2. Если суммарная мощность выше выделенной для всех включенных, то, отключаем с низким приоритетом до момента, когда суммарная мощность будет не более выделенной.
//
// вычисление сумарной мощности включенных без учета режима работы

   word cur_power = 0;
  for (byte i=0;i<options.therms_count;i++) {
    if ( (thermostat[i].info.state == ENABLE) &&
	     (thermostat[i].info.status == ON)
		) {
		 cur_power = cur_power + thermostat[i].info.power; 		
		}
  } 
 #ifdef DEBUG1 
  WRITE_TO_SERIAL(F("sum current power: "), cur_power, F(""), F(""));	
#endif 
  // сортируем по приоритету по возрастанию - макс приоритет 100, мин приоритет 1, т.е. первые элементы с низким приоритетом

  thermostat_sort_by_priority();

	//  посмотреть включенные auto и выключить, если есть превышение? смотрим по приоритету
   for (byte i=0;i<options.therms_count;i++) {
	  // byte r = options.therms_count - 1 - i;
    if ( (thermostat[i].info.state == ENABLE) &&
	     (thermostat[i].info.status == ON)&& 
		   (thermostat[i].info.mode == AUTO) &&
		   (cur_power > options.max_power)
		) {
			thermostat[i].turnOFF();
		 cur_power = cur_power - thermostat[i].info.power; 		
		}
  }  
// 
// Цикл 3 по высшему приоритету в режиме авто
// 3. Смотрим все выключенные.
// Если приоритет выключенного выше тех, которые включены, то их выключаем до тех пор, пока нельзя будет включить мощный. Здесь цикл.  
  //
#ifdef DEBUG1 
  WRITE_TO_SERIAL(F("main work "), cur_power, F(""), F(""));	
#endif 

   int k = options.therms_count-1;
   word tpwr = 0;
   while ( k >= 0 ) {
	  
	  if ( (thermostat[k].info.state == ENABLE) && 
	       (thermostat[k].info.status == OFF) && 
		   (thermostat[k].info.mode == AUTO) &&
		   (thermostat[k].tempSensor->value < thermostat[k].info.set_temp - thermostat[k].info.delta*10) &&
		   (thermostat[k].tempSensor->value > -10000) &&
		   (thermostat[k].tempSensor->value < 8500) &&
		   (thermostat[k].tempSensor->info.state == ENABLE)
		   )
	  { // только выключенные в режиме авто
 #ifdef DEBUG1  
  WRITE_TO_SERIAL(F("thermostat "), k, F(" try to ON ..."), F(""));	
#endif 		 
		  tpwr = cur_power + thermostat[k].info.power;
		   if ( tpwr <= options.max_power) {
			  // еще не превысили мощность, включаем
			   thermostat[k].turnON();
			   cur_power = tpwr;
				 #ifdef DEBUG1 
				  WRITE_TO_SERIAL(F("new current power: "), cur_power, F(""), F(""));	
				#endif 
		  } else {
			  // превысили мощность, нужно выключить наименее приоритетные
			  // выключаем с низким приоритетом
			  
				 #ifdef DEBUG1 
				  WRITE_TO_SERIAL(F("current power is overload!!! "), cur_power, F(", try to decrease power..."), F(""));	
				#endif 
				
			  // bool all_off = false;
			   for (int j=0;j<k;j++) {
				   #ifdef DEBUG1 
						 WRITE_TO_SERIAL(F("j: "), j, F(" k: "), k);	
						 WRITE_TO_SERIAL(F("index: "),thermostat[j].index, F(" relay: "), thermostat[j].relay->index);	
						 WRITE_TO_SERIAL(F("mode: "),thermostat[j].info.mode, F(" priority: "), thermostat[j].info.priority);	
				   #endif 
				   if ( (thermostat[j].info.state == ENABLE) && (thermostat[j].info.status == ON) && (thermostat[j].info.mode == AUTO))
				   { // только включенные в режиме авто
					 thermostat[j].turnOFF();
					 cur_power = cur_power - thermostat[j].info.power;
						 // #ifdef DEBUG  
						 //  WRITE_TO_SERIAL(F("decreasing current power: "), cur_power, F(""), F(""));	
						// #endif 
						k++;						
					 break;
				   }
				  // if ( j==k-1) all_off = true;
						 // #ifdef DEBUG1  
						  // WRITE_TO_SERIAL(F("nothing to OFF...."), F(""), F(""), F(""));	
						// #endif 				  
			   }
			 // if (!all_off) k++; // есть вероятность зацикливания 
		   }
	   } 
	   // else if ((thermostat[k].info.status == ON)&&
		   // (cur_power > options.max_power)) {
			// thermostat[k].turnOFF(options.device_name);
			// cur_power = cur_power - thermostat[k].info.power; 		   
	   // }
		k--;  
   }
  //вернем обратную сортировку по индексу
   thermostat_sort_by_index();
}


void uptime(const char* topic, long val) {
  byte l = strlen(topic);
  byte i = strlen(options.device_name);
  char* buffer = (char*)malloc( i+l+1 );
  char* sval =  (char*)malloc( 10 );
  if ( buffer == NULL) return;
  if ( sval == NULL) { free(buffer); return; }
  ltoa(val, sval, 10);
  strcpy(buffer, options.device_name);  
  strcat_P(buffer, (char*)topic);   
  mqtt_client.publish( buffer, sval);
  free(buffer);
  free(sval);
}

/*
void print_relays(EthernetClient &client, Relay *rel) {
  for (byte i=0;i<MAX_RELAY;i++) {
	client.print(F("<strong>Relay "));	 client.print(rel[i].index+1); client.print(F("</strong>"));  
    client.print(F("<a href=\"/relay/state/"));  client.print(rel[i].index);	
	client.print((rel[i].info.state == DISABLE) ?  F("/on\"><b> disable </b>") : F("/off\"><b> enable </b>"));
	client.print(F("</a>"));
	client.print(F("<span> Status </span>"));
	client.print(F("<a href=\"/relay/status/"));  client.print(rel[i].index);	 	
	client.print((rel[i].info.status == OFF) ?  F("/on\"><b>OFF</b>") : F("/off\"><b>ON</b>")); 
	client.print(F("</a>"));
    client.print(F("<span> Signal </span>"));
	client.print(F("<a href=\"/relay/signal/"));  client.print(rel[i].index); 	
	client.print((rel[i].info.signalType == NORMAL) ?  F("/invert\"><b>NORMAL</b>") : F("/normal\"><b>INVERT</b>")); 
	client.print(F("</a>"));
    client.print(F("<span> Flash </span>")); 
	client.print(F("<a href=\"/relay/flash/"));  client.print(rel[i].index); 	
	client.print((rel[i].info.to_flash == true) ?  F("/no\"><b>YES</b>") : F("/yes\"><b>NO</b>")); 
	client.print(F("</a>"));
	client.print(F("<span> Pin </span>")); 
	client.print(rel[i].info.pin);
	client.print(F("<br>"));
  }

}
*/

/*
void print_temp(EthernetClient &client, DSW_Temp *temp) {
  for (byte i=0;i<MAX_DSW_TEMPERATURE_SENSORS;i++) {
	client.print(F("<strong>DSW "));	 client.print(i+1); client.print(F("</strong>"));  
    client.print(F("<a href=\"/dsw/state/"));  client.print(i);	
	client.print((temp[i].info.state == DISABLE) ?  F("/on\"><b> disable </b>") : F("/off\"><b> enable </b>"));
	client.print(F("</a>"));
    client.print(F("<span> Address </span>")); 
;
	for (uint8_t j = 0; j < 8; j++)
	{
				client.print(F("0x"));
				if (temp[i].info.address[j] < 0x10) client.print(F("0"));
				client.print(temp[i].info.address[j], HEX);
				if (j < 7) client.print(F(", "));
	}	
    client.print(F("<span> Temp </span>")); 
	client.print(temp[i].value);
	client.print(F("<br>"));
  }
}
*/

/*
void print_thermostat(EthernetClient &client, Thermostat *therm) {
  for (byte i=0;i<options.therms_count;i++) {
	client.print(F("<strong>Thermostat "));	 client.print(therm[i].index+1); client.print(F("</strong>"));  
    client.print(F("<a href=\"/therm/state/"));  client.print(therm[i].index);	
	client.print((therm[i].info.state == DISABLE) ?  F("/on\"><b> disable </b>") : F("/off\"><b> enable </b>"));
	client.print(F("</a>"));
	client.print(F("<span> Status </span>"));	
	client.print((therm[i].info.status == OFF) ?  F("<b>OFF</b>") : F("<b>ON</b>")); 
    client.print(F("<span> Priority </span>")); 
	client.print(therm[i].info.priority);
	
    client.print(F("<span> Mode </span>")); 
	client.print(F("<a href=\"/therm/mode/"));  client.print(therm[i].index); 	
	client.print((therm[i].info.mode == AUTO) ?  F("/manual\"><b>AUTO</b>") : F("/auto\"><b>MANUAL</b>")); 
	client.print(F("</a>"));
//     client.print(F("<span> Relay </span>")); 
	// client.print(therm[i].relay.idx);
	// client.print(F("<span> Sensor </span>")); 
	// client.print(therm[i].tempSensor.value);
    // client.print(F("<span> Address </span>")); 
	// char tmp[8] = "";
	// for (uint8_t j = 0; j < 8; j++)
	// {
				// client.print(F("0x"));
				// if (therm[i].tempSensor.addr[j] < 0x10) client.print(F("0"));
				// client.print(therm[i].tempSensor.addr[j], HEX);
				// if (j < 7) client.print(F(", "));
	// } 
	client.print(F("<br>"));
  }
}
*/

/*
void generatePage(EthernetClient client){
  //client.println(F("<!DOCTYPE HTML>"));
  //client.println(F("<html>"));
  //client.println(F("<head>"));
  //client.println(F("<title>Server</title>"));
  //client.println(F("</head>"));

  //client.println(F("<body>"));

	client.print(F("<h2>")); client.print(options.device_name); client.print(F("</h2>"));
  //print_relay01_info(client);
  print_relays(client, relay);  client.println(F("<br>"));
  print_temp(client, dsw_temp);  client.println(F("<br>"));
  print_thermostat(client, thermostat); 
  
  client.println(F("<br>"));
  //client.println(F("</body>"));
  //client.println(F("</html>"));
}

*/
/*
void listenForEthernetClients() {
	// listen for incoming clients
	EthernetClient client = server.available();
	if (!client) return;
    // an http request ends with a blank line
    String request;
    boolean currentLineIsBlank = true;
    boolean requestLineReceived = false;
	
	while (client.connected()) {
		if (client.available()) {	
 			char c = client.read(); // read 1 byte (character) from client
			//HTTP_request += String(c);  // save the HTTP request 1 char at a time
			// TODO: HTTP_request += String(c); // !!!!!!!!!!!!!!!
			// last line of client request is blank and ends with \n
			// respond to client only after last line received
			if (c == '\n' && currentLineIsBlank) {
				request = getUrlFromHeader(&request);
				//WRITE_TO_SERIAL(F("Request: "), request, F(""), F(""));	
				
			  if(request=="/") {
				//successHeader(client);
				//generatePage(client);
			  } else {
				  
				  
				if ( request.startsWith("ajax_") ) {
					// ajax request here
					
				} else {
					// other pages or http get requests
					String dev = getNextSlug(&request);	
					String func = getNextSlug(&request);
					String idx = getNextSlug(&request);
					
					// разбор get запросов вида /dev/func/idx для страниц
					if ( dev.equals( HTTP_SLUG_RELAY ) ) {
						if ( func.equals( HTTP_SLUG_STATE ) ) {
							//redirectHeader(client, "/");
						} else if (func.equals( HTTP_SLUG_STATUS )) {
							relay[idx.toInt()].switch_relay( request.equals( CONST_ON ) ? OFF : ON);
							relay[idx.toInt()].publish( mqtt_client, options.device_name );
							//redirectHeader(client, "/");						
						} else if (func.equals( HTTP_SLUG_SIGNAL )) {
							//redirectHeader(client, "/");						
						} else if (func.equals( HTTP_SLUG_FLASH )) {
							//redirectHeader(client, "/");						
						}
					} else if ( dev.equals( HTTP_SLUG_THERM ) ) {
						if ( func.equals( HTTP_SLUG_STATE ) ) {
							thermostat[idx.toInt()].setState( request.equals( HTTP_SLUG_ON ) ? ENABLE : DISABLE, options.device_name);
							redirectHeader(client, "/");
						} else if ( func.equals( HTTP_SLUG_MODE ) ) {
							thermostat[idx.toInt()].setMode( request.equals( HTTP_SLUG_AUTO ) ? AUTO : MANUAL);
							//redirectHeader(client, "/");
						}					
					} else if ( dev.equals( HTTP_SLUG_DSW ) ) {
						if ( func.equals( HTTP_SLUG_STATE ) ) {
							dsw_temp[idx.toInt()].setState( request.equals( HTTP_SLUG_ENABLE ) ? ENABLE : DISABLE);
							//redirectHeader(client, "/");
						}					
					}						
				}
				// parse request
				//   /relay/state/0/off

				
			  }			  
				break;
			} else if (c == '\n') {
				// you're starting a new line
				currentLineIsBlank = true;
				if ( !requestLineReceived ) { requestLineReceived = true; }          
			} else if (c != '\r') {
				if(!requestLineReceived) { request += String(c); }          
				// you've gotten a character on the current line
				currentLineIsBlank = false;
			}	 		
		}
	}
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");		
}
*/

void loop() {
  
  // TODO something
  // update temp 
  if ((millis() - lastReadingTime2) > (options.temperature_refresh * 1000)) {
    lastReadingTime2 = millis();
    temperature_update();
	free_memory_publish();
	//WRITE_TO_SERIAL(F("FreeMem: "), memoryFree(), F(""), F(""));	
  }

  if ((millis() - lastReadingTime3) > (options.thermostat_refresh * 1000)) {
    lastReadingTime3 = millis();
    termostat_update();
  }  
  
  if ((millis() - lastReadingTime4) > (10 * 1000)) {
    lastReadingTime4 = millis();
    uptime(P_TOPIC_ARDUINO_UPTIME, lastReadingTime4);
	publish_all_data();
  }
  
  if (!mqtt_client.connected()) {
    reconnect();
  }
  
  mqtt_client.loop();
  
  // listen for incoming clients
 // listenForEthernetClients();  
  
}
