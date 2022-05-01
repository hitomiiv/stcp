// socket.c
#include "socket.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "native/native.h"
#include "error.h"

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

static bool init = false;

void stcp_socket_initialize_library()
{
	if (!init)
	{
#ifdef _WIN32
		WSADATA data;
		int err = WSAStartup(MAKEWORD(2, 2), &data);
		if (err != 0)
		{
			STCP_FAIL(err);
		}
#endif
		init = true;
	}
}

void stcp_socket_terminate_library()
{
	if (init)
	{
#ifdef _WIN32
		int err = WSACleanup();
		if (err != 0)
		{
			STCP_FAIL(err);
		}
#endif
		init = false;
	}
}

static timeval make_timeout(int timeout_milliseconds)
{
	timeval timeout;

	if (timeout_milliseconds < 0)
	{
		timeout.tv_sec = LONG_MAX;
		timeout.tv_usec = LONG_MAX;
	}
	else
	{
		timeout.tv_sec = timeout_milliseconds / 1000;
		timeout_milliseconds -= timeout.tv_sec * 1000;
		timeout.tv_usec = timeout_milliseconds * 1000;
	}

	return timeout;
}

static fd_set make_socket_set(const socket_t* sockets, int n)
{
	assert(sockets);
	assert(n < FD_SETSIZE);

	fd_set socket_set;
	FD_ZERO(&socket_set);
	for (int i = 0; i < n; ++i)
	{
		assert(sockets[i] != STCP_INVALID_SOCKET);
		FD_SET(sockets[i], &socket_set);
	}

	return socket_set;
}

// private function to resolve ips and hostnames
static sockaddr init_address(const char* name, const char* protocol)
{
	assert(name || protocol);

	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

	addrinfo* info = NULL;
	sockaddr ret;
	if (0 == getaddrinfo(name, protocol, &hints, &info))
	{
		ret = *info->ai_addr;
		freeaddrinfo(info);
	}
	else
	{
		freeaddrinfo(info);
		STCP_FAIL_LAST_ERROR();
	}

	return ret;
}

socket_t stcp_socket_create()
{
	socket_t s = socket(AF_INET, SOCK_STREAM, 0);
	if (s != STCP_INVALID_SOCKET)
	{
		unsigned long int mode = 1;
		if (0 != STCP_SET_NON_BLOCKING(s, &mode))
			STCP_FAIL_LAST_ERROR();
	}
	else
	{
		STCP_FAIL_LAST_ERROR();
	}

	return s;
}

socket_t stcp_socket_accept(const socket_t* server)
{
	assert(server);

	socket_t s = accept(*server, NULL, NULL);
	if (s != STCP_INVALID_SOCKET)
	{
		unsigned long int mode = 1;
		if (0 != STCP_SET_NON_BLOCKING(s, &mode))
			STCP_FAIL_LAST_ERROR();
	}
	else
	{
		STCP_FAIL_LAST_ERROR();
	}

	return s;
}

void stcp_socket_connect(const socket_t* s, const char* address, const char* protocol)
{
	assert(s);

	sockaddr addr = init_address(address, protocol);
	if (0 != connect(*s, &addr, sizeof(addr)))
	{
		stcp_error err = stcp_get_last_error();

		// Ignore these errors:
		// This is windows/linux's way of saying the
		// non-blocking socket is connecting asynchronously
		if (err != STCP_EWOULDBLOCK && err != STCP_EINPROGRESS)
			STCP_FAIL_LAST_ERROR();
	}
	else
	{
		STCP_FAIL_LAST_ERROR();
	}
}

void stcp_socket_bind(const socket_t* s, const char* address, const char* protocol)
{
	assert(s);

	sockaddr addr = init_address(address, protocol);
	if (0 != bind(*s, &addr, sizeof(addr)))
		STCP_FAIL_LAST_ERROR();
}

void stcp_socket_listen(const socket_t* s, int max_pending_channels)
{
	assert(s);
	assert(max_pending_channels > 0);

	if (0 != listen(*s, max_pending_channels))
		STCP_FAIL_LAST_ERROR();
}

void stcp_socket_shutdown(const socket_t* s)
{
	assert(s);
	if (0 != STCP_SHUTDOWN_SOCKET(*s))
		STCP_FAIL_LAST_ERROR();
}

bool stcp_socket_poll_write(const socket_t* socket, int timeout_milliseconds)
{
	return stcp_socket_poll_write_n(socket, 1, timeout_milliseconds);
}

bool stcp_socket_poll_write_n(const socket_t* sockets, int n, int timeout_milliseconds)
{
	assert(sockets);
	assert(n > 0);

	fd_set socket_set = make_socket_set(sockets, n);
	timeval timeout = make_timeout(timeout_milliseconds);

	int sockets_ready = select(FD_SETSIZE, NULL, &socket_set, NULL, &timeout);
	if (sockets_ready == n)
	{
		return true;
	}
	else if (sockets_ready == -1)
	{
		STCP_FAIL_LAST_ERROR();
		return false;
	}
	else
	{
		if (timeout_milliseconds != 0)
			stcp_raise_error(STCP_ETIMEDOUT);

		return false;
	}
}

bool stcp_socket_poll_read(const socket_t* socket, int timeout_milliseconds)
{
	return stcp_socket_poll_read_n(socket, 1, timeout_milliseconds);
}

bool stcp_socket_poll_read_n(const socket_t* sockets, int n, int timeout_milliseconds)
{
	assert(sockets);
	assert(n > 0);

	fd_set socket_set = make_socket_set(sockets, n);
	timeval timeout = make_timeout(timeout_milliseconds);

	int sockets_ready = select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout);
	if (sockets_ready == n)
	{
		return true;
	}
	else if (sockets_ready == -1)
	{
		STCP_FAIL_LAST_ERROR();
		return false;
	}
	else
	{
		if (timeout_milliseconds != 0)
			stcp_raise_error(STCP_ETIMEDOUT);

		return false;
	}
}

int stcp_socket_write(const socket_t* s, const char* buffer, int n)
{
	assert(s);

	int bytes_sent = send(*s, buffer, n, 0);
	if (bytes_sent == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return 0;
	}

	return bytes_sent;
}

int stcp_socket_read(const socket_t* s, char* buffer, int n)
{
	int bytes_received = recv(*s, buffer, n, 0);
	if (bytes_received == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return 0;
	}

	return bytes_received;
}

void stcp_socket_close(socket_t* s)
{
	assert(s);

	if (*s != STCP_INVALID_SOCKET)
	{
		STCP_CLOSE_SOCKET(*s);
		*s = -1;
	}
}
