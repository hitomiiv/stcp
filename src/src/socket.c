// socket.c
#include "socket.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "error.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
	#define STCP_INVALID_SOCKET INVALID_SOCKET
	#define SET_NON_BLOCKING(socket, value) ioctlsocket(*s, FIONBIO, value);
#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#define STCP_INVALID_SOCKET -1ULL
	#define SET_NON_BLOCKING(socket, value) ioctl(*s, FIONBIO, value);
#endif

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

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

static fd_set make_socket_set(socket_t* sockets, int n)
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
static bool init_address(sockaddr* address,
		const char* name,
		const char* protocol)
{
	assert(address);
	assert(name);

	addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

	addrinfo* info = NULL;
	int err = getaddrinfo(name, protocol, &hints, &info);
	if (err != 0)
	{
		stcp_raise_error(err);
		return false;
	}

	*address = *info->ai_addr;

	freeaddrinfo(info);
	return true;
}

bool stcp_socket_init(socket_t* s)
{
	assert(s);

	// init
	*s = STCP_INVALID_SOCKET;
	*s = socket(AF_INET, SOCK_STREAM, 0);
	if (*s == STCP_INVALID_SOCKET)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	// set to non-blocking
	unsigned long int mode = 1;
	int err = SET_NON_BLOCKING(*s, &mode);
	if (err != 0)
	{
		stcp_raise_error(err);
		return false;
	}

	return true;
}

bool stcp_socket_bind(socket_t s, const char* address, const char* protocol)
{
	sockaddr addr;
	if (!init_address(&addr, address, protocol))
		return false;

	if (0 != bind(s, &addr, sizeof(addr)))
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	return true;
}

bool stcp_socket_listen(socket_t s, int max_pending_channels)
{
	if (0 != listen(s, max_pending_channels))
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	return true;
}

bool stcp_socket_accept(socket_t* s, socket_t server)
{
	assert(s);

	*s = accept(server, NULL, NULL);
	if (*s == STCP_INVALID_SOCKET)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	return true;
}

bool stcp_socket_connect(socket_t s, const char* address, const char* protocol)
{
	sockaddr addr;
	if (!init_address(&addr, address, protocol))
		return false;

	if (0 != connect(s, &addr, sizeof(addr)))
	{
		stcp_error err = stcp_get_last_error();

		// Ignore these errors:
		// This is windows/linux's way of saying the
		// non-blocking socket is connecting asynchronously
		if (err != STCP_WOULD_BLOCK)
			return true;

		stcp_raise_error(err);
		return false;
	}

	return true;
}

bool stcp_socket_poll_write(socket_t socket, int timeout_milliseconds)
{
	return stcp_socket_poll_write_n(&socket, 1, timeout_milliseconds);
}

bool stcp_socket_poll_write_n(socket_t* sockets, int n, int timeout_milliseconds)
{
	assert(sockets);
	assert(n > 0);

	fd_set socket_set = make_socket_set(sockets, n);
	timeval timeout = make_timeout(timeout_milliseconds);

	int sockets_ready = select(FD_SETSIZE, NULL, &socket_set, NULL, &timeout);
	if (sockets_ready == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}
	else if (sockets_ready != n)
	{
		if (timeout_milliseconds != 0)
			stcp_raise_error(STCP_CONNECTION_TIMED_OUT);

		return false;
	}
	else
	{
		return true;
	}
}

bool stcp_socket_poll_read(socket_t socket, int timeout_milliseconds)
{
	return stcp_socket_poll_read_n(&socket, 1, timeout_milliseconds);
}

bool stcp_socket_poll_read_n(socket_t* sockets, int n, int timeout_milliseconds)
{
	assert(sockets);
	assert(n > 0);

	fd_set socket_set = make_socket_set(sockets, n);
	timeval timeout = make_timeout(timeout_milliseconds);

	int sockets_ready = select(FD_SETSIZE, &socket_set, NULL, NULL, &timeout);
	if (sockets_ready == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}
	else if (sockets_ready != n)
	{
		if (timeout_milliseconds != 0)
			stcp_raise_error(STCP_CONNECTION_TIMED_OUT);

		return false;
	}
	else
	{
		return true;
	}
}

int stcp_socket_write(socket_t s, const char* buffer, int n)
{
	int bytes_sent = send(s, buffer, n, 0);
	if (bytes_sent == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return 0;
	}

	return bytes_sent;
}

int stcp_socket_read(socket_t s, char* buffer, int n)
{
	int bytes_received = recv(s, buffer, n, 0);
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
#ifdef _WIN32
		closesocket(*s);
#else
		close(*s);
#endif
		*s = -1;
	}
}
