# stcp
This is a simple, lightweight, and cross-platform TCP/IP library written in C

There are two basic structures, `stcp_server` and `stcp_channel`, which represent a TCP server and client, respectively. Both of these need an address and protocol to create. The protocol can alternatively indicate the port (i.e. "80"). Only ipv4 and hostnames are supported. Servers can accept pending channels, and channels can send and receive data. 

## Compilation
A Makefile is provided for unix environments. Use `make [static|shared]` to compile. By default, `make` compiles a static library.

For Windows you will have to write your own Makefile or use an IDE. Make sure to link against `ws2_32`, `ssl`, and `crypto`.

## Error handling
The user can handle errors one of two ways:
1. Return values: A function has failed if it returns false or NULL. For a more detailed error, use `stcp_get_last_error()`
2. Error callback: An error callback function takes two parameters: an `stcp_error` for the error code and a `void*` for optional user data. It is set using `stcp_set_error_callback(cb, user_data)`.

Use `stcp_error_to_string()` to convert an `stcp_error` into a human-readable string.

## Usage
First, call `stcp_initialize()` to initialize the library. 

To host your own server, use `stcp_open_server()` with the desired parameters. Then, accept a client with `stcp_accept_channel()`. A client will now be able to connect, and both the server and client may  then send and receive data.

To connect to a pre-existing server, wse `stcp_open_channel()`.

Use `stcp_send()` and `stcp_receive()` to transfer some amount of data.
If there is more data to be read than your buffer can hold, you may need to call `stcp_receive()` multiple times to process everything. Alternatively, you can use `stcp_stream_receive()` with a callback function and user data pointer. This is the equivalent of wrapping `stcp_receive()` in a while loop until everything is received. To modify the buffer size in which `stcp_stream_receive()` loads  data, redefine `STCP_STREAM_BUFFER_SIZE` before you include `"stcp.h"`.

Remember to use `stcp_close_channel()`, `stcp_close_server()`, and `stcp_terminate()` to prevent any memory leaks.

## Example

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stcp.h"

void process_error(stcp_error e, void* user_data)
{
	stcp_channel** channel = (stcp_channel**) user_data;

	perror(stcp_error_to_string(e));
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
```
