// socket.h
#ifndef SOCKET_H_
#define SOCKET_H_

/*
 * These functions provide a simple wrapper over
 * windows and unix sockets.
 *
 * The purpose is to unify errors and error handling,
 * remove unnecessary magic numbers, and simplify
 * the interface with clear naming
 */

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#ifdef _WIN32
	typedef unsigned long long int socket_t;
#else
	typedef long long int socket_t;
#endif

// Returns true if successful
bool stcp_socket_init(socket_t* s);
bool stcp_socket_bind(socket_t s, const char* address, const char* protocol);
bool stcp_socket_listen(socket_t s, int max_pending_channels);
bool stcp_socket_accept(socket_t* s, socket_t server);
bool stcp_socket_connect(socket_t s, const char* address, const char* protocol);

// Returns true if all sockets are ready to transfer data
// Only raises STCP_CONNECTION_TIMED_OUT if timeout_milliseconds != 0
bool stcp_socket_poll_write(socket_t socket, int timeout_milliseconds);
bool stcp_socket_poll_write_n(socket_t* sockets, int n, int timeout_milliseconds);
bool stcp_socket_poll_read(socket_t s, int timeout_milliseconds);
bool stcp_socket_poll_read_n(socket_t* sockets, int n, int timeout_milliseconds);

// Returns the number of bytes transferred
int stcp_socket_write(socket_t s, const char* buffer, int n);
int stcp_socket_read(socket_t s, char* buffer, int n);

// Frees a socket's resources
void stcp_socket_close(socket_t* s);


#ifdef __cplusplus
}
#endif

#endif /* SOCKET_H_ */
