#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/stcp.h"

void process_error(stcp_error e, void* user_data)
{
	(void) e;

	stcp_print_error();

	stcp_channel** channel = (stcp_channel**) user_data;
	stcp_close_channel(*channel);
	stcp_terminate();
	exit(-1);
}

bool print_buffer(const char* buffer, int length, void* user_data)
{
	(void) user_data;
	return fwrite(buffer, sizeof(char), length, stdout) == (size_t) length;
}

int main()
{
	stcp_channel* channel = NULL;
	stcp_set_error_callback(process_error, &channel);
	stcp_initialize();

	const char* request = "HEAD / HTTP/1.2\r\n\r\n";
	channel = stcp_open_channel("www.google.com", "http");
	stcp_send(channel, request, strlen(request), 500);
	stcp_stream_receive(channel, print_buffer, NULL, 500);

	stcp_close_channel(channel);
	stcp_terminate();
	return 0;
}
