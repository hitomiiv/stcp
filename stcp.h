// stcp.h
#ifndef STCP_H_
#define STCP_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#define STCP_FAILURE -1
#define STCP_TIMED_OUT -2

typedef struct stcp_address stcp_address;
typedef struct stcp_channel stcp_channel;
typedef struct stcp_server stcp_server;


// ----- Initialization and Error Handling -----
// Initializes the library.
// Returns false if an error occurred, otherwise true
bool stcp_initialize();

// Terminates the library
void stcp_terminate();

// Returns the last library error
int stcp_get_last_error();

// Returns a string of the specified library error
const char* stcp_resolve_error(int error);


// ----- Addresses -----
// Creates an address with the specified ipv4 and port.
// Returns NULL if an error occurred
stcp_address* stcp_create_address_ipv4(const char* ipv4, int port);

// Creates an address with the specified hostname and port.
// Returns NULL if an error occurred
stcp_address* stcp_create_address_hostname(const char* hostname, int port);

// Frees an address's resources in memory
void stcp_free_address(stcp_address* address);


// ----- Servers -----
// Create a TCP/IP server with the given address and max allowed pending channels.
// Returns NULL if an error occurred
stcp_server* stcp_create_server(const stcp_address* server_address, int max_pending_channels);

// Accepts a pending channel using a timeout (-1 indicates no timeout).
// Returns NULL if an error occurred, or the request timed out
stcp_channel* stcp_accept_channel(const stcp_server* server, int timeout_milliseconds);

// Frees a server's resources in memory
void stcp_free_server(stcp_server* server);


// ----- Channels -----
// Creates a TCP/IP channel (client) connected to the given server address.
// Returns NULL if an error occurred
stcp_channel* stcp_create_channel(const stcp_address* server_address);

// Sends data through a channel using a timeout (-1 indicates no timeout).
// Returns the number of bytes sent, STCP_FAILURE, or STCP_TIMED_OUT
int stcp_send(const stcp_channel* channel, const char* buffer, int length, int timeout_milliseconds);

// Receives data through a channel using a timeout (-1 indicates no timeout).
// Returns the number of bytes received, STCP_FAILURE, or STCP_TIMED_OUT
int stcp_receive(const stcp_channel* channel, char* buffer, int length, int timeout_milliseconds);

// Frees a channel's resources in memory
void stcp_free_channel(stcp_channel* channel);


#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
