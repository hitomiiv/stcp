// stcp.h
#ifndef STCP_H_
#define STCP_H_

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

typedef struct stcp_address stcp_address;
typedef struct stcp_channel stcp_channel;
typedef struct stcp_server stcp_server;

// range: 4, 9, 13, 14, 22, 24, 35-71
typedef enum stcp_error
{
	STCP_UNKNOWN_ERROR                 = 0,
	STCP_FUNCTION_INTERRUPTED          = 4,
	STCP_INVALID_FILE_HANDLE           = 9,
	STCP_PERMISSION_DENIED             = 13,
	STCP_INVALID_POINTER               = 14,
	STCP_INVALID_ARGUMENT              = 22,
	STCP_TOO_MANY_SOCKETS              = 24,
	STCP_RESOURCE_UNAVAILABLE          = 35,
	STCP_BLOCKED_BY_CALLER             = 36,
	STCP_BLOCKED_BY_CALLEE             = 37,
	STCP_INVALID_SOCKET                = 38,
	STCP_INVALID_DESTINATION           = 39,
	STCP_MESSAGE_TOO_LONG              = 40,
	STCP_INVALID_PROTOCOL              = 41,
	STCP_INVALID_PROTOCOL_OPTION       = 42,
	STCP_PROTOCOL_NOT_SUPPORTED        = 43,
	STCP_SOCKET_NOT_SUPPORTED          = 44,
	STCP_OPERATION_NOT_SUPPORTED       = 45,
	STCP_PROTOCOL_FAMILY_NOT_SUPPORTED = 46,
	STCP_ADDRESS_FAMILY_NOT_SUPPORTED  = 47,
	STCP_ADDRESS_IN_USE                = 48,
	STCP_ADDRESS_NOT_AVAILABLE         = 49,
	STCP_NETWORK_DOWN                  = 50,
	STCP_NETWORK_UNREACHABLE           = 51,
	STCP_NETWORK_RESET                 = 52,
	STCP_CONNECTION_ABORTED            = 53,
	STCP_CONNECTION_RESET              = 54,
	STCP_NO_BUFFER_SPACE               = 55,
	STCP_SOCKET_ALREADY_CONNECTED      = 56,
	STCP_SOCKET_NOT_CONNECTED          = 57,
	STCP_SOCKET_SHUTDOWN               = 58,
	STCP_TOO_MANY_REFERENCES           = 59,
	STCP_CONNECTION_TIMED_OUT          = 60,
	STCP_CONNECTION_REFUSED            = 61,
	STCP_INVALID_NAME                  = 62,
	STCP_NAME_TOO_LONG                 = 63,
	STCP_HOST_DOWN                     = 64,
	STCP_HOST_UNREACHABLE              = 65,
	STCP_DIRECTORY_NOT_EMPTY           = 66,
	STCP_TOO_MANY_PROCESSES            = 67,
	STCP_TOO_MANY_USERS                = 68,
	STCP_OUT_OF_DISK_SPACE             = 69,
	STCP_OLD_HANDLE                    = 70,
	STCP_RESOURCE_NOT_LOCAL            = 71,
} stcp_error;

typedef void (*stcp_error_callback_fn)(stcp_error /*error*/, void* /*user data*/);


// ----- Initialization and Error Handling -----
// Initializes the library. This must be called before any other function.
// Returns false if an error occurred, otherwise true
bool stcp_initialize();

// Terminates the library
void stcp_terminate();

// Sets the error callback and user data pointer
// error_callback must be a function pointer of type void(stcp_error, void*)
void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data);


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
// Returns the number of bytes sent
int stcp_send(const stcp_channel* channel, const char* buffer, int length, int timeout_milliseconds);

// Receives data through a channel using a timeout (-1 indicates no timeout).
// Returns the number of bytes received
int stcp_receive(const stcp_channel* channel, char* buffer, int length, int timeout_milliseconds);

// Frees a channel's resources in memory
void stcp_free_channel(stcp_channel* channel);


#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
