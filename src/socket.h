// socket.h
#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdbool.h>

#ifdef _WIN32
	typedef unsigned long long int socket_t;
#else
	typedef long long int socket_t;
#endif

// this stuff all returns false if an error occurred
bool stcp_open_socket(socket_t* s);
bool stcp_poll_accept(const socket_t* s, int timeout_milliseconds);
bool stcp_poll_send(const socket_t* s, int timeout_milliseconds);
bool stcp_poll_receive(const socket_t* s, int timeout_milliseconds);
void stcp_close_socket(socket_t* s);

#endif /* SOCKET_H_ */
