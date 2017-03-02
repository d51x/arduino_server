#include "http.h"

void successHeader(EthernetClient client){
  send(client, header_http_ok);
}

void redirectHeader(EthernetClient client, const char *path){
  client.println(F("HTTP/1.1 302 Moved Temporarily"));
  client.println(F("Content-Type: text/html"));
  client.print(F("Location: ")); client.println(path);
  client.println(F("Connnection: keep-alive"));
  client.println();
}

/*
request - input url
param - cutted request without slug
result - slug
*/
String getUrlFromHeader(String *request) {
	return request->substring( request->indexOf(' ') + 1, request->lastIndexOf(' '));
}

String getNextSlug(String *request) {
	String res = "";
	if ( request[0] != '/' ) request->remove( 0, 1);
	byte pos = request->indexOf('/');
	res = request->substring(0, pos );
	request->remove( 0, pos + 1);
	return res;
}

void send(EthernetClient client, const char *ptr)
{
  char c;
  while(c=pgm_read_byte(ptr++)) client.print(c);
}

