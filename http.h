#ifndef HTTP_H
#define HTTP_H

#include <Ethernet.h>
#include <avr/pgmspace.h>

const char PROGMEM header_http_ok[]={\
	"HTTP/1.1 200 OK\r\n"\
	"Content-Type: text/html\r\n"\
	"<meta http-equiv='content-type' content='text/html; charset=UTF-8'>\r\n"\
	"Connection: keep-alive\r\n"\
	"\r\n\0"\
};

const char http_Found[] PROGMEM = {\
    "HTTP/1.0 302 Found\r\n"\
    "Location: %s/\r\n"\
		"Connnection: keep-alive\r\n"\
		"\0"\
};

const char http_Unauthorized[] PROGMEM = {\
    "HTTP/1.0 401 Unauthorized\r\n"\
    "Content-Type: text/html\r\n\r\n"\
    "<h1>401 Unauthorized</h1>\0"\
};	

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



void successHeader(EthernetClient &client);
void redirectHeader(EthernetClient &client, const char *path);

/*
request - input url
result - slug
*/
String getUrlFromHeader(String *request);
String getNextSlug(String *request);


#endif //HTTP_H
