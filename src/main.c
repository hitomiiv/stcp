#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stcp.h"

int main()
{
	stcp_initialize();

	const int timeout_ms = 500;
	const int port = 80;
	const char* request = "HEAD / HTTP/1.2\r\n\r\n";
	char buffer[1024];
	buffer[0] = 0;

	stcp_address* addr = stcp_create_address_hostname("www.google.com", port);
	stcp_channel* channel = stcp_create_channel(addr);

	stcp_send(channel, request, strlen(request), timeout_ms);
	stcp_receive(channel, buffer, sizeof(buffer), timeout_ms);
	puts(buffer);

	stcp_free_address(addr);
	stcp_free_channel(channel);
	stcp_terminate();
	return 0;
}
