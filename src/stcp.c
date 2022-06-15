// stcp.c

#include "stcp.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

// sometimes these get long
#define MALLOC(type) (type*) malloc(sizeof(type))
#define REALLOC(ptr, type) (type*) realloc(ptr, sizeof(type))

// ----- TCP/IP socket types -----
typedef struct stcp_channel
{
	socket_t socket;
} stcp_channel;

typedef struct stcp_server
{
	socket_t socket;
} stcp_server;


// ----- Initialization -----
static bool init = false;

bool stcp_initialize()
{
	if (!init)
	{
		// socket initialization
		stcp_socket_initialize_library();

		init = true;
	}

	return true;
}

void stcp_terminate()
{
	if (init)
	{
		// socket cleanup
		stcp_socket_terminate_library();

		init = false;
	}
}

// ----- Servers -----
stcp_server* stcp_open_server(const char* address, const char* protocol, int max_pending_channels)
{
	stcp_server* server = MALLOC(stcp_server);
	assert(server);
	assert(address);
	assert(max_pending_channels > 0);

	server->socket = stcp_socket_create();
	stcp_socket_bind(&server->socket, address, protocol);
	stcp_socket_listen(&server->socket, max_pending_channels);
	return server;
}

stcp_channel* stcp_accept(stcp_server* server, int timeout_milliseconds)
{
	assert(server);

	if (!stcp_socket_poll_read(&server->socket, timeout_milliseconds))
		return NULL;

	stcp_channel* channel = MALLOC(stcp_channel);
	assert(channel);
	channel->socket = stcp_socket_accept(&server->socket);
	return channel;
}

void stcp_close_server(stcp_server* server)
{
	if (server)
	{
		stcp_socket_close(&server->socket);
		free(server);
	}
}

// ----- Channels -----
stcp_channel* stcp_connect(const char* address, const char* protocol)
{
	stcp_channel* channel = (stcp_channel*) malloc(sizeof(stcp_channel));
	assert(channel);
	assert(address);
	assert(protocol);

	channel->socket = stcp_socket_create();
	stcp_socket_connect(&channel->socket, address, protocol);
	return channel;
}

bool stcp_send(stcp_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_write(&channel->socket, timeout_milliseconds))
		return false;

	int bytes_sent = 0;
	while (bytes_sent < length)
	{
		int ret = stcp_socket_write(&channel->socket,
				buffer + bytes_sent,
				length - bytes_sent);

		if (ret == 0)
			return false;

		bytes_sent += ret;
	}

	return true;
}

int stcp_receive(stcp_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_read(&channel->socket, timeout_milliseconds))
		return 0;

	return stcp_socket_read(&channel->socket, buffer, length);
}

bool stcp_stream_receive(stcp_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds)
{
	assert(channel);
	assert(stream_output);

	if (!stcp_socket_poll_read(&channel->socket, timeout_milliseconds))
		return false;

	char buffer[STCP_STREAM_BUFFER_SIZE];
	const int length = STCP_STREAM_BUFFER_SIZE;

	do
	{
		int bytes_received = stcp_socket_read(&channel->socket, buffer, length);

		if (bytes_received == 0)
			return false;

		if (!stream_output(buffer, bytes_received, user_data))
			return false;

	} while (stcp_socket_poll_read(&channel->socket, 0));

	return true;
}

void stcp_close_channel(stcp_channel* channel)
{
	if (channel)
	{
		stcp_socket_close(&channel->socket);
		free(channel);
	}
}
