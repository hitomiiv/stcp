// socket.c
#include "socket.h"

#include <assert.h>
#include <stdlib.h>

#include "error.h"

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

typedef struct timeval timeval;
typedef struct sockaddr sockaddr;
typedef struct addrinfo addrinfo;

static timeval make_timeout(int timeout_milliseconds)
{
	assert(timeout_milliseconds >= 0);

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = timeout_milliseconds * 1000;
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
		assert(sockets[i] != INVALID_SOCKET);
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
	*s = socket(AF_INET, SOCK_STREAM, 0);
	if (*s == INVALID_SOCKET)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	// set to non-blocking
	unsigned long int mode = 1;
	int err = ioctlsocket(*s, FIONBIO, &mode);
	if (err != 0)
	{
		stcp_raise_error(err);
		return false;
	}

	return true;
}

bool stcp_init_udp_socket(socket_t* s)
{
	assert(s);

	// init
	*s = socket(AF_INET, SOCK_DGRAM, 0);
	if (*s == INVALID_SOCKET)
	{
		stcp_raise_error(stcp_get_last_error());
		return false;
	}

	// set to non-blocking
	unsigned long int mode = 1;
	int err = ioctlsocket(*s, FIONBIO, &mode);
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
	if (*s == INVALID_SOCKET)
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

		// this is correct behavior for a non-blocking socket
		if (err != STCP_WOULD_BLOCK)
		{
			stcp_raise_error(err);
			return false;
		}
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
	assert(timeout_milliseconds >= -1);

	// When timeout is infinite, always assume socket is writable
	if (timeout_milliseconds == -1)
		return true;

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
	assert(timeout_milliseconds >= -1);

	// When timeout is infinite, always assume socket is readable
	if (timeout_milliseconds == -1)
		return true;

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
		stcp_raise_error(STCP_CONNECTION_TIMED_OUT);
		return false;
	}
	else
	{
		return true;
	}
}

int stcp_socket_send(socket_t s, const char* buffer, int n)
{
	int bytes_sent = send(s, buffer, n, 0);
	if (bytes_sent == -1)
	{
		stcp_raise_error(stcp_get_last_error());
		return 0;
	}

	return bytes_sent;
}

int stcp_socket_receive(socket_t s, char* buffer, int n)
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

	if (*s != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(*s);
#else
		close(*s);
#endif
		*s = -1;
	}
}
