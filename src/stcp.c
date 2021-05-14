// stcp.c
#include "stcp.h"

#include "socket.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

typedef struct stcp_address
{
	struct sockaddr impl;
} stcp_address;

typedef struct stcp_channel
{
	socket_t socket;
} stcp_channel;

typedef struct stcp_server
{
	socket_t socket;
} stcp_server;

static stcp_error stcp_get_last_error();
static void stcp_propagate_error(stcp_error err);
static char* stcp_resolve_hostname(const char* hostname);

stcp_error_callback_fn _error_callback;
void* _error_callback_user_data;

static stcp_error stcp_get_last_error()
{
#ifdef _WIN32
	int error = WSAGetLastError() - 10000;
#else
	int error = errno;
#endif
	if (error == 4
			|| error == 9
			|| error == 13
			|| error == 14
			|| error == 22
			|| error == 24
			|| (35 <= error && error <= 71))
	{
		return (stcp_error) error;
	}
	else
	{
		return STCP_UNKNOWN_ERROR;
	}
}

static void stcp_propagate_error(stcp_error err)
{
	if (_error_callback)
		_error_callback(err, _error_callback_user_data);
}

static char* stcp_resolve_hostname(const char* hostname)
{
	assert(hostname);

	struct hostent* host = NULL;
	host = gethostbyname(hostname);
	if (!host)
		return NULL;

	struct in_addr** addr_list = (struct in_addr**) host->h_addr_list;
	assert(addr_list);
	assert(*addr_list);

	char* converted = inet_ntoa(**addr_list);
	assert(converted);
	char* ipv4 = malloc(strlen(converted) + 1);
	assert(ipv4);
	strcpy(ipv4, converted);
	return ipv4;
}

bool stcp_initialize()
{
	_error_callback = NULL;
	_error_callback_user_data = NULL;
#ifdef _WIN32
	WSADATA data;
	errno = WSAStartup(MAKEWORD(2, 2), &data);
	if (errno != 0)
	{
		perror("Winsock failed to start");
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

void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data)
{
	assert(error_callback);
	_error_callback = error_callback;
	_error_callback_user_data = user_data;
}

stcp_address* stcp_create_address_ipv4(const char* ipv4, int port)
{
	assert(ipv4);
	assert(port > 0);

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ipv4);
	if (addr.sin_addr.s_addr == INADDR_NONE)
	{
		stcp_propagate_error(STCP_HOST_UNREACHABLE);
		return NULL;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	stcp_address* address = malloc(sizeof(stcp_address));
	assert(address);
	address->impl = *(struct sockaddr*) &addr;
	return address;
}

stcp_address* stcp_create_address_hostname(const char* hostname, int port)
{
	assert(hostname);
	assert(port > 0);

	char* ipv4 = stcp_resolve_hostname(hostname);
	if (!ipv4)
	{
		stcp_propagate_error(STCP_HOST_UNREACHABLE);
		return NULL;
	}

	stcp_address* address = stcp_create_address_ipv4(ipv4, port);
	free(ipv4);
	return address;
}

void stcp_free_address(stcp_address* address)
{
	if (address)
		free(address);
}

stcp_server* stcp_create_server(const stcp_address* server_address, int max_pending_channels)
{
	assert(server_address);
	assert(max_pending_channels > 0);

	stcp_server* server = malloc(sizeof(stcp_server));
	assert(server);

	if (!stcp_open_socket(&server->socket))
	{
		stcp_free_server(server);
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	if (bind(server->socket, &server_address->impl, sizeof(server_address->impl)) != 0)
	{
		stcp_free_server(server);
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	if (listen(server->socket, max_pending_channels) != 0)
	{
		stcp_free_server(server);
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	return server;
}

stcp_channel* stcp_accept_channel(const stcp_server* server, int timeout_milliseconds)
{
	assert(server);
	assert(timeout_milliseconds >= -1);

	int ret = stcp_poll_read(&server->socket, timeout_milliseconds);
	if (ret == 0)
	{
		stcp_propagate_error(STCP_CONNECTION_TIMED_OUT);
		return NULL;
	}
	else if (ret == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	socket_t s = accept(server->socket, NULL, NULL);
	if (s == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	stcp_channel* channel = malloc(sizeof(stcp_channel));
	assert(channel);
	channel->socket = s;
	return channel;
}

void stcp_free_server(stcp_server* server)
{
	if (server)
	{
		stcp_close_socket(&server->socket);
		free(server);
	}
}

stcp_channel* stcp_create_channel(const stcp_address* server_address)
{
	assert(server_address);

	stcp_channel* channel = malloc(sizeof(stcp_channel));
	assert(channel);

	if (!stcp_open_socket(&channel->socket))
	{
		stcp_free_channel(channel);
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	if (connect(channel->socket, &server_address->impl, sizeof(server_address->impl)) != 0)
	{
		stcp_free_channel(channel);
		stcp_propagate_error(stcp_get_last_error());
		return NULL;
	}

	return channel;
}

int stcp_send(const stcp_channel* channel, const char* buffer, int length, int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	int ret = stcp_poll_write(&channel->socket, timeout_milliseconds);
	if (ret == 0)
	{
		stcp_propagate_error(STCP_CONNECTION_TIMED_OUT);
		return 0;
	}
	else if (ret == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return 0;
	}

	ret = send(channel->socket, buffer, length, 0);
	if (ret == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return 0;
	}

	return ret;
}

int stcp_receive(const stcp_channel* channel, char* buffer, int length, int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	int ret = stcp_poll_read(&channel->socket, timeout_milliseconds);
	if (ret == 0)
	{
		stcp_propagate_error(STCP_CONNECTION_TIMED_OUT);
		return 0;
	}
	else if (ret == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return 0;
	}

	ret = recv(channel->socket, buffer, length, 0);
	if (ret == -1)
	{
		stcp_propagate_error(stcp_get_last_error());
		return 0;
	}

	return ret;
}

void stcp_free_channel(stcp_channel* channel)
{
	if (channel)
	{
		stcp_close_socket(&channel->socket);
		free(channel);
	}
}
