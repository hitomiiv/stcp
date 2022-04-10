// stcp.h
#ifndef STCP_H_
#define STCP_H_

#include "error.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

// ----- TCP/IP socket types -----
typedef struct stcp_channel
{
	socket_t socket;
} stcp_channel;

typedef struct stcp_server
{
	socket_t socket;
} stcp_server;


// ----- Initialization -----
// Initializes the library. This must be called before any other function.
// Returns true if successful
bool stcp_initialize();

// Terminates the library
void stcp_terminate();


// ----- Servers -----
// Create a TCP/IP server with the given address and max allowed pending channels.
// Returns true if successful
bool stcp_init_server(stcp_server* server,
		const char* address,
		const char* protocol,
		int max_pending_channels);

// Accepts a pending channel using a timeout (use timeout of -1 to block).
// Returns true if successful
bool stcp_accept_channel(stcp_channel* channel,
		stcp_server server,
		int timeout_milliseconds);

// Frees a server's resources in memory
void stcp_close_server(stcp_server* server);


// ----- Channels -----
// Creates a TCP/IP channel (client) connected to the given server address.
// Returns true if successful
bool stcp_init_channel(stcp_channel* channel,
		const char* address,
		const char* protocol);

// Sends data through a channel using a timeout (use timeout of -1 to block).
// Returns the number of bytes sent
int stcp_send(stcp_channel channel,
		const char* buffer,
		int length,
		int timeout_milliseconds);

// Receives data through a channel using a timeout (use timeout of -1 to block).
// Returns the number of bytes received
int stcp_receive(stcp_channel channel,
		char* buffer,
		int length,
		int timeout_milliseconds);

// Frees a channel's resources in memory
void stcp_close_channel(stcp_channel* channel);


#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
