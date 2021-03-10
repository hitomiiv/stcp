// stcp.c
#include "stcp.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define HOSTNAME_SEARCH_DEPTH 100
#define RECV_BUFFER_SIZE 2048

#define POLL_READ 0
#define POLL_WRITE 1
#define POLL_EXCEPT 2

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

typedef struct stcp_socket
{
#ifdef _WIN32
	unsigned long long int impl;
#else
	long long int impl;
#endif
} stcp_socket;

int stcp_initialize()
{
#ifdef _WIN32
	WSADATA data;
	int err = WSAStartup(MAKEWORD(2, 2), &data);
	if (err != 0)
		return STCP_FAILURE;
#endif
	return STCP_SUCCESS;
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

const char* stcp_error_to_string(int error)
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

char* stcp_resolve_hostname(const char* hostname)
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

char* stcp_resolve_socket(const stcp_socket* s)
{
	assert(s);

	char* converted = inet_ntoa(*(struct in_addr*) &s);
	if (!converted)
		return NULL;

	char* ipv4 = malloc(strlen(converted) + 1);
	assert(ipv4);
	strcpy(ipv4, converted);
	return ipv4;
}

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
	assert(address);
	free(address);
}

stcp_socket* stcp_create_socket()
{
	stcp_socket* s = malloc(sizeof(stcp_socket));
	assert(s);
	s->impl = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s->impl == -1)
	{
		free(s);
		s = NULL;
	}
	return s;
}

stcp_socket* stcp_accept(const stcp_socket* server)
{
	assert(server);
	int client_socket = accept(server->impl, NULL, NULL);
	if (client_socket == -1)
		return NULL;
	stcp_socket* s = malloc(sizeof(stcp_socket));
	assert(s);
	s->impl = client_socket;
	return s;
}

void stcp_free_socket(stcp_socket* s)
{
	assert(s);
	if (s->impl == -1)
		return;

#ifdef _WIN32
	closesocket(s->impl);
#else
	close(s->impl);
#endif

	free(s);
}

int stcp_listen(const stcp_socket* server, const stcp_address* server_address, int queue_length)
{
	assert(server);
	assert(server_address);
	assert(queue_length > 0);

	int err = bind(server->impl, &server_address->impl, sizeof(server_address->impl));
	if (err != 0)
		return STCP_FAILURE;

	return listen(server->impl, queue_length);
}

int stcp_connect(const stcp_socket* client, const stcp_address* server_address)
{
	assert(client);
	assert(server_address);
	return connect(client->impl, &server_address->impl, sizeof(server_address->impl));
}

int stcp_send(const stcp_socket* s, const char* buffer, int length)
{
	assert(s);
	assert(buffer);
	assert(length > 0);
	return send(s->impl, buffer, length, 0) < 0;
}

int stcp_receive(const stcp_socket* s, char** buffer, int* length)
{
	assert(s);
	assert(buffer);
	assert(length);

	*buffer = malloc(RECV_BUFFER_SIZE);
	assert(*buffer);

	*length = recv(s->impl, *buffer, RECV_BUFFER_SIZE - 1, 0);
	if (*length == -1)
	{
		free(*buffer);
		*buffer = NULL;
		return STCP_FAILURE;
	}

	(*buffer)[*length] = '\0';
	*buffer = realloc(*buffer, *length + 1);

	return STCP_SUCCESS;
}

int stcp_set_blocking(stcp_socket* s, int value)
{
	assert(s);
	assert(value == 0 || value == 1);
	value = !value;
	return ioctlsocket(s->impl, FIONBIO, (u_long*) &value);
}

int stcp_poll(const stcp_socket* s, int poll_type, int timeout_seconds, int timeout_microseconds)
{
	assert(s);
	assert(poll_type >= 0);
	assert(poll_type <= 2);
	assert(timeout_seconds >= 0);
	assert(timeout_microseconds >= 0);

	struct fd_set set;
	FD_ZERO(&set);
	FD_SET(s->impl, &set);

	struct timeval timeout;
	timeout.tv_sec = timeout_seconds;
	timeout.tv_usec = timeout_microseconds;

	int ret = 0;
	if (poll_type == POLL_READ)
		ret = select(0, &set, NULL, NULL, &timeout);
	else if (poll_type == POLL_WRITE)
		ret = select(0, NULL, &set, NULL, &timeout);
	else if (poll_type == POLL_EXCEPT)
		ret = select(0, NULL, NULL, &set, &timeout);

	return ret > 0;
}

int stcp_poll_accept(const stcp_socket* s, int timeout_seconds, int timeout_microseconds)
{
	return stcp_poll_receive(s, timeout_seconds, timeout_microseconds);
}

int stcp_poll_send(const stcp_socket* s, int timeout_seconds, int timeout_microseconds)
{
	assert(s);
	assert(timeout_seconds >= 0);
	assert(timeout_microseconds >= 0);
	return stcp_poll(s, POLL_WRITE, timeout_seconds, timeout_microseconds);
}

int stcp_poll_receive(const stcp_socket* s, int timeout_seconds, int timeout_microseconds)
{
	assert(s);
	assert(timeout_seconds >= 0);
	assert(timeout_microseconds >= 0);
	return stcp_poll(s, POLL_READ, timeout_seconds, timeout_microseconds);
}
