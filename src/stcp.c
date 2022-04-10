// stcp.c

#include "stcp.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <stdio.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

// ----- Initialization -----
bool stcp_initialize()
{
	stcp_set_error_callback(NULL, NULL);

#ifdef _WIN32
	WSADATA data;
	errno = WSAStartup(MAKEWORD(2, 2), &data);
	if (errno != 0)
	{
		stcp_raise_error(errno);
		return false;
	}
#endif

	return true;
}

void stcp_terminate()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

// ----- Servers -----
bool stcp_init_server(stcp_server* server,
		const char* address,
		const char* protocol,
		int max_pending_channels)
{
	assert(server);
	assert(address);
	assert(max_pending_channels > 0);

	if (!stcp_socket_init(&server->socket))
		return false;

	if (!stcp_socket_bind(server->socket, address, protocol))
	{
		stcp_close_server(server);
		return false;
	}

	if (!stcp_socket_listen(server->socket, max_pending_channels))
	{
		stcp_close_server(server);
		return false;
	}

	return true;
}

bool stcp_accept_channel(stcp_channel* channel, stcp_server server, int timeout_milliseconds)
{
	assert(channel);
	assert(timeout_milliseconds >= -1);

	return stcp_socket_poll_read(server.socket, timeout_milliseconds)
		&& stcp_socket_accept(&channel->socket, server.socket);
}

void stcp_close_server(stcp_server* server)
{
	assert(server);
	stcp_socket_close(&server->socket);
}

#include <stdio.h>

// ----- Channels -----
bool stcp_init_channel(stcp_channel* channel,
		const char* address,
		const char* protocol)
{
	assert(channel);
	assert(address);
	assert(protocol);

	if (!stcp_socket_init(&channel->socket))
		return false;

//	if (!stcp_socket_poll_write(channel->socket, timeout_milliseconds))
//	{
//		stcp_close_channel(channel);
//		return false;
//	}

	if (!stcp_socket_connect(channel->socket, address, protocol))
	{
		stcp_close_channel(channel);
		return false;
	}

	return true;
}

int stcp_send(stcp_channel channel, const char* buffer, int length, int timeout_milliseconds)
{
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	if (!stcp_socket_poll_write(channel.socket, timeout_milliseconds))
		return 0;

	return stcp_socket_send(channel.socket, buffer, length);
}

int stcp_receive(stcp_channel channel, char* buffer, int length, int timeout_milliseconds)
{
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	if (!stcp_socket_poll_read(channel.socket, timeout_milliseconds))
		return 0;

	return stcp_socket_receive(channel.socket, buffer, length);
}

void stcp_close_channel(stcp_channel* channel)
{
	assert(channel);
	stcp_socket_close(&channel->socket);
}
