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

bool stcp_initialize()
{
#ifdef _WIN32
	WSADATA data;
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (err != 0)
		return false;
#endif
	return true;
}

void stcp_terminate()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

int stcp_get_last_error()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

const char* stcp_resolve_error(int error)
{
#ifdef _WIN32
	switch (error)
	{
	case WSAEINTR:
		return "Interrupted function call";
	case WSAEBADF:
		return "WSAEBADF";
	case WSAEACCES:
		return "WSAEACCES";
	case WSAEFAULT:
		return "Bad address";
	case WSAEINVAL:
		return "Invalid argument";
	case WSAEMFILE:
		return "Too many open files";
	case WSAEWOULDBLOCK:
		return "Operation would block";
	case WSAEINPROGRESS:
		return "Operation now in progress";
	case WSAEALREADY:
		return "Operation already in progress";
	case WSAENOTSOCK:
		return "Socket operation on non-socket";
	case WSAEDESTADDRREQ:
		return "Destination address required";
	case WSAEMSGSIZE:
		return "Message too long";
	case WSAEPROTOTYPE:
		return "Protocol wrong type for socket";
	case WSAENOPROTOOPT:
		return "Bad protocol option";
	case WSAEPROTONOSUPPORT:
		return "Protocol not supported";
	case WSAESOCKTNOSUPPORT:
		return "Socket type not supported";
	case WSAEOPNOTSUPP:
		return "Operation not supported";
	case WSAEPFNOSUPPORT:
		return "Protocol family not supported";
	case WSAEAFNOSUPPORT:
		return "Address family not supported by protocol family";
	case WSAEADDRINUSE:
		return "Address already in use";
	case WSAEADDRNOTAVAIL:
		return "Cannot assign requested address";
	case WSAENETDOWN:
		return "Network is down";
	case WSAENETUNREACH:
		return "Network is unreachable";
	case WSAENETRESET:
		return "Network dropped connection on reset";
	case WSAECONNABORTED:
		return "Software caused connection abort";
	case WSAECONNRESET:
		return "Connection reset by peer";
	case WSAENOBUFS:
		return "No buffer space available";
	case WSAEISCONN:
		return "Socket is already connected";
	case WSAENOTCONN:
		return "Socket is not connected";
	case WSAESHUTDOWN:
		return "Cannot send after socket shutdown";
	case WSAETOOMANYREFS:
		return "WSAETOOMANYREFS";
	case WSAETIMEDOUT:
		return "Connection timed out";
	case WSAECONNREFUSED:
		return "Connection refused";
	case WSAELOOP:
		return "WSAELOOP";
	case WSAENAMETOOLONG:
		return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN:
		return "Host is down";
	case WSAEHOSTUNREACH:
		return "No route to host";
	case WSAENOTEMPTY:
		return "WSAENOTEMPTY";
	case WSAEPROCLIM:
		return "Too many processes";
	case WSAEUSERS:
		return "WSAEUSERS";
	case WSAEDQUOT:
		return "WSAEDQUOT";
	case WSAESTALE:
		return "WSAESTALE";
	case WSAEREMOTE:
		return "WSAEREMOTE";
	case WSASYSNOTREADY:
		return "Network subsystem is unavailable";
	case WSAVERNOTSUPPORTED:
		return "WINSOCK.DLL version out of range";
	case WSANOTINITIALISED:
		return "Successful WSAStartup() not yet performed";
	case WSAEDISCON:
		return "WSAEDISCON";
	case WSAENOMORE:
		return "WSAENOMORE";
	case WSAECANCELLED:
		return "WSAECANCELLED";
	case WSAEINVALIDPROCTABLE:
		return "WSAEINVALIDPROCTABLE";
	case WSAEINVALIDPROVIDER:
		return "WSAEINVALIDPROVIDER";
	case WSAEPROVIDERFAILEDINIT:
		return "WSAEPROVIDERFAILEDINIT";
	case WSASYSCALLFAILURE:
		return "WSASYSCALLFAILURE";
	case WSASERVICE_NOT_FOUND:
		return "WSASERVICE_NOT_FOUND";
	case WSATYPE_NOT_FOUND:
		return "WSATYPE_NOT_FOUND";
	case WSA_E_NO_MORE:
		return "WSA_E_NO_MORE";
	case WSA_E_CANCELLED:
		return "WSA_E_CANCELLED";
	case WSAEREFUSED:
		return "WSAEREFUSED";
	case WSAHOST_NOT_FOUND:
		return "Host not found";
	case WSATRY_AGAIN:
		return "Non-authoritative host not found";
	case WSANO_RECOVERY:
		return "This is a non-recoverable error";
	case WSANO_DATA:
		return "Valid name, no data record of requested type";
	default:
		return "Unknown winsock error";
	}
#else
	return strerror(errno);
#endif
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

//char* stcp_resolve_socket(const socket_t* s)
//{
//	assert(s);
//	const struct in_addr* addr = (const struct in_addr*) s;
//	char* converted = inet_ntoa(*addr);
//	if (!converted)
//		return NULL;
//	char* ipv4 = malloc(strlen(converted) + 1);
//	assert(ipv4);
//	strcpy(ipv4, converted);
//	return ipv4;
//}

stcp_address* stcp_create_address_ipv4(const char* ipv4, int port)
{
	assert(ipv4);
	assert(port > 0);

	struct sockaddr_in addr;
	addr.sin_addr.s_addr = inet_addr(ipv4);
	if (addr.sin_addr.s_addr == INADDR_NONE)
		return NULL;

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
		return NULL;

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
		return NULL;
	}

	if (bind(server->socket, &server_address->impl, sizeof(server_address->impl)) != 0)
	{
		stcp_free_server(server);
		return NULL;
	}

	if (listen(server->socket, max_pending_channels) != 0)
	{
		stcp_free_server(server);
		return NULL;
	}

	return server;
}

stcp_channel* stcp_accept_channel(const stcp_server* server, int timeout_milliseconds)
{
	assert(server);
	assert(timeout_milliseconds >= -1);

	if (!stcp_poll_accept(&server->socket, timeout_milliseconds))
		return NULL;

	socket_t s = accept(server->socket, NULL, NULL);
	if (s == -1)
		return NULL;

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
		return NULL;
	}

	if (connect(channel->socket, &server_address->impl, sizeof(server_address->impl)) != 0)
	{
		stcp_free_channel(channel);
		return NULL;
	}

	return channel;
}

bool stcp_send(const stcp_channel* channel, const char* buffer, int length, int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	if (!stcp_poll_send(&channel->socket, timeout_milliseconds))
		return NULL;

	return send(channel->socket, buffer, length, 0) >= 0;
}

/*
 * Receives data through a socket
 */
bool stcp_receive(const stcp_channel* channel, char* buffer, int length, int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);
	assert(timeout_milliseconds >= -1);

	if (!stcp_poll_receive(&channel->socket, timeout_milliseconds))
		return NULL;

	length = recv(channel->socket, buffer, length - 1, 0);
	if (length <= 0)
		return false;

	buffer[length - 1] = 0;
	return true;
}

void stcp_free_channel(stcp_channel* channel)
{
	if (channel)
	{
		stcp_close_socket(&channel->socket);
		free(channel);
	}
}
