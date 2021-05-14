# stcp
This is a simple, lightweight, and cross-platform TCP/IP library written in C

First initialize the library with `stcp_initialize();`
There are two basic structures, `stcp_server` and `stcp_channel`, which represent a TCP server and client, respectively.
Both of these need an `stcp_address` and a port to create. Only ipv4 and hostnames are supported.
Servers can accept pending channels, and channels can send/receive data. 
Note that for send/receive, a timeout of 0 does not block the caller. However, a timeout of -1 blocks the caller until the task is completed.

Here's an example. In a real program, remember to check the return values:
```c
#include <stdio.h>
#include <string.h>

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
```
