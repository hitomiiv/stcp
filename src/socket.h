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

#include "native/native_types.h"

// Library startup / cleanup
void stcp_socket_initialize_library();
void stcp_socket_terminate_library();

// Socket creation
socket_t stcp_socket_create();
socket_t stcp_socket_accept(const socket_t* server);

// Connection management
void stcp_socket_connect(const socket_t* s, const char* address, const char* protocol);
void stcp_socket_bind(const socket_t* s, const char* address, const char* protocol);
void stcp_socket_listen(const socket_t* s, int max_pending_channels);
void stcp_socket_shutdown(const socket_t* s);

// Returns true if all sockets are ready to transfer data
// Only raises STCP_CONNECTION_TIMED_OUT if timeout_milliseconds != 0
bool stcp_socket_poll_write(const socket_t* socket, int timeout_milliseconds);
bool stcp_socket_poll_write_n(const socket_t* sockets, int n, int timeout_milliseconds);
bool stcp_socket_poll_read(const socket_t* s, int timeout_milliseconds);
bool stcp_socket_poll_read_n(const socket_t* sockets, int n, int timeout_milliseconds);

// Returns the number of bytes transferred
int stcp_socket_write(const socket_t* s, const char* buffer, int n);
int stcp_socket_read(const socket_t* s, char* buffer, int n);

// Frees a socket's resources
void stcp_socket_close(socket_t* s);


#ifdef __cplusplus
}
#endif

#endif /* SOCKET_H_ */
