# stcp
This is a simple, lightweight, and cross-platform TCP/IP library written in C

When compiling on Windows, link against `ws2_32`

First initialize the library with `stcp_initialize();`. There are two basic structures, `stcp_server` and `stcp_channel`, which represent a TCP server and client, respectively. Both of these need an address and protocol to create. The protocol can alternatively indicate the port (i.e. "80"). Only ipv4 and hostnames are supported. Servers can accept pending channels, and channels can send/receive data. Note that for send/receive, a timeout of 0 only checks for write/readability. However, a timeout of -1 blocks the caller until the task is completed.

The user can handle errors with `stcp_set_error_callback(cb, user_data)`. The callback function takes two parameters: an `stcp_error` for the error code and a `void*` for the user data.

Here's a complete example:

```c
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
```


