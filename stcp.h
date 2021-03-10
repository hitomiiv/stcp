// stcp.h
#ifndef STCP_H_
#define STCP_H_

#define STCP_SUCCESS 0
#define STCP_FAILURE -1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stcp_address stcp_address;
typedef struct stcp_socket stcp_socket;

// Initialize the library
// Returns SUCCESS or FAILURE
int stcp_initialize();

// Terminate the library
void stcp_terminate();

// Return the last error code
int stcp_get_last_error();

// Convert an error to a string
const char* stcp_error_to_string(int error);

// Returns a string containing the ipv4 of the hostname
// This string must be freed later with free()
char* stcp_resolve_hostname(const char* hostname);

// Returns a string containing the ipv4 of the socket
// This string must be freed later with free()
char* stcp_resolve_socket(const stcp_socket* s);

// Returns a dynamically allocated address or NULL
// The address must be freed later with free_address()
stcp_address* stcp_create_address_ipv4(const char* ipv4, int port);

// Returns a dynamically allocated address or NULL
// The address must be freed later with free_address()
stcp_address* stcp_create_address_hostname(const char* hostname, int port);

// Frees an address
void stcp_free_address(stcp_address* address);

// Returns a new tcp socket or NULL
// The socket must be freed later with free_socket()
stcp_socket* stcp_create_socket();

// Returns a new client socket of the server or NULL
// The server must first be set to listening mode
// The socket must be freed later with free_socket()
stcp_socket* stcp_accept(const stcp_socket* server);

// Frees a socket
void stcp_free_socket(stcp_socket* s);

// Sets a socket to listen for queue_length new connections at server_address
// Returns SUCCESS or FAILURE
int stcp_listen(const stcp_socket* server, const stcp_address* server_address, int queue_length);

// Connects to a server address
// Returns SUCCESS or FAILURE
int stcp_connect(const stcp_socket* client, const stcp_address* server_address);

// Sends data through a socket
// Returns SUCCESS or FAILURE
int stcp_send(const stcp_socket* s, const char* buffer, int length);

// Receives data through a socket
// Returns SUCCESS or FAILURE
// If the operation succeeded, buffer must be freed
int stcp_receive(const stcp_socket* s, char** buffer, int* length);

// Sets a socket to blocking or non-blocking mode
// When sockets are set to non-blocking, sockets must first be polled. Otherwise, functions will return FAILURE
// Returns SUCCESS or FAILURE
int stcp_set_blocking(stcp_socket* s, int value);

// Returns true if there is at least one socket to accept
int stcp_poll_accept(const stcp_socket* s, int timeout_seconds, int timeout_microseconds);

// Returns true if data may be sent through the socket
int stcp_poll_send(const stcp_socket* s, int timeout_seconds, int timeout_microseconds);

// Returns true if data may be received through the socket
int stcp_poll_receive(const stcp_socket* s, int timeout_seconds, int timeout_microseconds);

#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
