#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stcp.h"

stcp_channel channel;

void error_callback(stcp_error e, void* user_data)
{
	(void) user_data;
	fprintf(stderr, "STCP returned with error code %d\n", e);
	stcp_close_channel(&channel);
	stcp_terminate();
	exit(-1);
}

int main()
{
	stcp_set_error_callback(error_callback, NULL);

	stcp_initialize();
	stcp_init_channel(&channel, "www.google.com", "http");

	const char* request = "HEAD / HTTP/1.2\r\n\r\n";
	stcp_send(channel, request, strlen(request), 500);

	char response[1024];
	int length = stcp_receive(channel, response, 1024, 500);
	fwrite(response, 1, length, stdout);

	stcp_close_channel(&channel);
	stcp_terminate();
	return 0;
}
