// stcp.c

#include "stcp.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

// ----- TCP/IP socket types -----
typedef struct stcp_channel
{
	socket_t socket;
} stcp_channel;

typedef struct stcp_encrypted_channel
{
	socket_t socket;
} stcp_encrypted_channel;

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
	#ifdef _WIN32
		WSADATA data;
		errno = WSAStartup(MAKEWORD(2, 2), &data);
		if (errno != 0)
		{
			stcp_raise_error(errno);
			return false;
		}
	#endif

		init = true;
	}

	return true;
}

void stcp_terminate()
{
	if (init)
	{
	#ifdef _WIN32
		WSACleanup();
	#endif
		init = false;
	}
}

// ----- Servers -----
stcp_server* stcp_open_server(const char* address, const char* protocol, int max_pending_channels)
{
	stcp_server* server = (stcp_server*) malloc(sizeof(stcp_server));
	assert(server);
	assert(address);
	assert(max_pending_channels > 0);

	if (!stcp_socket_init(&server->socket))
	{
		stcp_close_server(server);
		return NULL;
	}

	if (!stcp_socket_bind(server->socket, address, protocol))
	{
		stcp_close_server(server);
		return NULL;
	}

	if (!stcp_socket_listen(server->socket, max_pending_channels))
	{
		stcp_close_server(server);
		return NULL;
	}

	return server;
}

stcp_channel* stcp_accept_channel(const stcp_server* server, int timeout_milliseconds)
{
	assert(server);
	assert(timeout_milliseconds >= -1);

	if (!stcp_socket_poll_read(server->socket, timeout_milliseconds))
		return NULL; // timed out

	stcp_channel* channel = (stcp_channel*) malloc(sizeof(stcp_channel));
	assert(channel);

	if (!stcp_socket_accept(&channel->socket, server->socket))
	{
		stcp_close_channel(channel);
		return NULL;
	}

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
stcp_channel* stcp_open_channel(const char* address, const char* protocol)
{
	stcp_channel* channel = (stcp_channel*) malloc(sizeof(stcp_channel));
	assert(channel);
	assert(address);
	assert(protocol);

	if (!stcp_socket_init(&channel->socket))
	{
		stcp_close_channel(channel);
		return NULL;
	}

	if (!stcp_socket_connect(channel->socket, address, protocol))
	{
		stcp_close_channel(channel);
		return NULL;
	}

	return channel;
}

bool stcp_send(const stcp_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_write(channel->socket, timeout_milliseconds))
		return false;

	int bytes_sent = 0;
	while (bytes_sent < length)
	{
		int ret = stcp_socket_write(channel->socket,
				buffer + bytes_sent,
				length - bytes_sent);

		if (ret == 0)
			return false;

		bytes_sent += ret;
	}

	return true;
}

int stcp_receive(const stcp_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_read(channel->socket, timeout_milliseconds))
		return 0;

	int bytes_received = 0;
	do
	{
		int ret = stcp_socket_read(channel->socket,
				buffer + bytes_received,
				length - bytes_received);

		if (ret == 0)
			break;

		bytes_received += ret;
	} while (bytes_received < length && stcp_socket_poll_read(channel->socket, 0));

	return bytes_received;
}

bool stcp_stream_receive(const stcp_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds)
{
	assert(channel);
	assert(stream_output);

	if (!stcp_socket_poll_read(channel->socket, timeout_milliseconds))
		return false;

	char buffer[STCP_STREAM_BUFFER_SIZE];
	const int length = STCP_STREAM_BUFFER_SIZE;

	do
	{
		int bytes_received = stcp_socket_read(channel->socket, buffer, length);

		if (bytes_received == 0)
			return false;

		if (!stream_output(buffer, bytes_received, user_data))
			return false;

	} while (stcp_socket_poll_read(channel->socket, 0));

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
