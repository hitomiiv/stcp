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

static int ms_to_us(int ms)
{
	return ms * 1000;
}

static int ms_to_s(int ms)
{
	return ms / 1000;
}

static int s_to_ms(int s)
{
	return s * 1000;
}

bool stcp_open_socket(socket_t* s)
{
	assert(s);
	*s = -1;
	*s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return *s != -1;
}

bool stcp_poll_accept(const socket_t* s, int timeout_milliseconds)
{
	return stcp_poll_receive(s, timeout_milliseconds);
}

bool stcp_poll_send(const socket_t* s, int timeout_milliseconds)
{
	assert(s);
	assert(timeout_milliseconds >= -1);

	if (timeout_milliseconds == -1)
		return true;

	int seconds = ms_to_s(timeout_milliseconds);
	timeout_milliseconds -= s_to_ms(seconds);
	int microseconds = ms_to_us(timeout_milliseconds);

	fd_set set;
	FD_ZERO(&set);
	FD_SET(*s, &set);

	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = microseconds;

	return select(*s + 1, NULL, &set, NULL, &timeout) > 0;
}

bool stcp_poll_receive(const socket_t* s, int timeout_milliseconds)
{
	assert(s);
	assert(timeout_milliseconds >= -1);

	if (timeout_milliseconds == -1)
		return true;

	int seconds = ms_to_s(timeout_milliseconds);
	timeout_milliseconds -= s_to_ms(seconds);
	int microseconds = ms_to_us(timeout_milliseconds);

	fd_set set;
	FD_ZERO(&set);
	FD_SET(*s, &set);

	struct timeval timeout;
	timeout.tv_sec = seconds;
	timeout.tv_usec = microseconds;

	return select(*s + 1, &set, NULL, NULL, &timeout) > 0;
}

void stcp_close_socket(socket_t* s)
{
	assert(s);
	if (*s != -1)
	{
#ifdef _WIN32
		closesocket(*s);
#else
		close(*s);
#endif
		*s = -1;
	}
}
