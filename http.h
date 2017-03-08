#ifndef HTTP_H
#define HTTP_H

#include <Ethernet.h>
#include <avr/pgmspace.h>

const char PROGMEM header_http_ok[]=
	"HTTP/1.1 200 OK\r\n"\
	"Content-Type: text/html\r\n"\
	"<meta http-equiv='content-type' content='text/html; charset=UTF-8'>\r\n"\
	"Connection: keep-alive\r\n"\
	"\r\n";

const char http_Found[] PROGMEM = "HTTP/1.0 302 Found\r\n"\
    "Location: %s/\r\n"\
		"Connnection: keep-alive\r\n";

const char http_Unauthorized[] PROGMEM = "HTTP/1.0 401 Unauthorized\r\n"\
    "Content-Type: text/html\r\n\r\n"\
    "<h1>401 Unauthorized</h1>";	

 const char page_header[] PROGMEM = "";/*"<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"\
"<title>%s</title>"\   //device name
"<meta http-equiv=\"REFRESH\" content=\"60\">"\
"<meta name=\"viewport\" content=\"width=640\">"\
"<meta name=\"mobile-web-app-capable\" content=\"yes\">"\
"<link rel=\"stylesheet\" href=\"main.css\">"\
"</head><body><br><div style=\"text-align: center\"><div style=\"display: inline-block\">"\
"<div class=\"name fll\">"\
"%s"\ //$device_name
"<div class=\"www\">Dminty<a href=\"http://d51x.ru\" target=\"_blank\">d51x.ru</a><br>ver 0.1.1</div></div>"\
"<div class=\"spV2 fll\"></div><div class=\"spV fll\"></div><div class=\"spV2 fll\"></div>"\
"<div class=\"sys fll\">"\
"Free memory: %d B.<br>"\  // free memory
"Uptime: %s<br>"\  // uptime
"</div></div></div><div class=\"c2\">";	
		 */
const char page_footer[] PROGMEM = "<div class=\"h\" style=\"background:#808080\">Config:</div>"\
"<div class=\"c\"><a href=\"index.html\">Main</a><a href=\"options.html\">Options</a><a href=\"onewire.html\">OneWire</a>"\ 
"<a href=\"relays.html\">Relays</a><a href=\"therms.html\">Thermostats</a></div></div></body></html>";

const char page_block_end[] PROGMEM = "</div><br>";

//----------------------------------------------------------------------		
const char page_main_dsw_start[] PROGMEM = "<div class=\"h\" style=\"background: #7D8EE2\">OneWire temperature sensors:</div>"\
	"<div class=\"c\">";
			// here: page_main_dsw_item

const char page_main_dsw_item[] PROGMEM = "<span class=\"tmp %d\"><b>%d:</b> %0.1f °C.</span>";

//----------------------------------------------------------------------
const char page_main_relays_start[] PROGMEM = "	<div class=\"h\" style=\"background: #73c140\">Relays:</div>"\
	"<div class=\"c\" style=\"padding-bottom: 28px\">";
const char page_main_relays_item[] PROGMEM = "<a href=\"#\" rel=\"gpio\" data-id=\"%d\" d-val=\"%d\"> <div class=\"g_0 k fll\">%d</div> </a> <div class=\"dummy fll\"> </div>";

//----------------------------------------------------------------------
const char page_main_thermostats_start[] PROGMEM = "<div class=\"h\" style=\"background: #f48c37\">Thermostats:</div>"\
	"<div class=\"c\" style=\"padding-bottom: 28px\">";  //page_main_thermostats_item
const char page_main_thermostats_item[] PROGMEM = "<a href=\"#\" rel=\"gpio\" data-id=\"%d\" d-val=\"%d\"> <div class=\"g_0 k fll3\">%d</div> </a><div class=\"dummy fll\"> </div>";
	
//----------------------------------------------------------------------
const char HTTP_SLUG_RELAY[] PROGMEM = "relay";
const char HTTP_SLUG_THERM[] PROGMEM = "therm";
const char HTTP_SLUG_DSW[] PROGMEM = "dsw";

const char HTTP_SLUG_STATUS[] PROGMEM = "status";
const char HTTP_SLUG_STATE[] PROGMEM = "state";
const char HTTP_SLUG_SIGNAL[] PROGMEM = "signal";
const char HTTP_SLUG_FLASH[] PROGMEM = "flash";
const char HTTP_SLUG_MODE[] PROGMEM = "mode";
const char HTTP_SLUG_AUTO[] PROGMEM = "auto";
const char HTTP_SLUG_ON[] PROGMEM = "on";
const char HTTP_SLUG_ENABLE[] PROGMEM = "enable";



void successHeader(EthernetClient client);
void redirectHeader(EthernetClient client, const char *path);
void send(EthernetClient client, const char *ptr);
void generate_main_page(EthernetClient client);

/*
request - input url
result - slug
*/
String getUrlFromHeader(String *request);
String getNextSlug(String *request);


#endif //HTTP_H
