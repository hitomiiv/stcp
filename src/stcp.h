// stcp.h
#ifndef STCP_H_
#define STCP_H_

#include "error.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef STCP_STREAM_BUFFER_SIZE
#define STCP_STREAM_BUFFER_SIZE 2048
#endif

// ----- TCP/IP socket types -----
typedef struct stcp_channel stcp_channel;
typedef struct stcp_server stcp_server;
typedef struct stcp_encrypted_channel stcp_encrypted_channel;

// ----- Callback function pointers -----
// Process the stream buffer
// Return true if successful
typedef bool (*stream_output_fn)(const char* buffer, int length, void* user_data);

// ----- Initialization -----
// Initializes the library. This must be called before any other function.
// Returns true if successful
bool stcp_initialize();

// Terminates the library
void stcp_terminate();


// ----- Servers -----
// Create a TCP/IP server with the given address and max allowed pending channels.
stcp_server* stcp_open_server(const char* address,
		const char* protocol,
		int max_pending_channels);

// Accepts a pending channel using a timeout (use a negative timeout to block).
stcp_channel* stcp_accept(stcp_server* server,
		int timeout_milliseconds);

stcp_encrypted_channel* stcp_eaccept(stcp_server* server,
		int timeout_milliseconds);

// Frees a server's resources in memory
void stcp_close_server(stcp_server* server);


// ----- Channels -----
// Creates a TCP/IP channel (client) connected to the given server address.
// Returns true if successful
stcp_channel* stcp_connect(const char* address,
		const char* protocol);

stcp_encrypted_channel* stcp_econnect(const char* address,
		const char* protocol,
		int timeout_milliseconds);

// Sends data through a channel until the buffer is empty, or an error occurs
// Returns true if successful
bool stcp_send(stcp_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds);

bool stcp_esend(stcp_encrypted_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds);

// Receives data through a channel until the buffer is full or there is no more data
// This may not read all of the incoming data. To process all data, use stcp_stream_receive()
// Returns the new buffer length
int stcp_receive(stcp_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds);

int stcp_ereceive(stcp_encrypted_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds);

// Receive channel data using a callback
// Returns true if successful
bool stcp_stream_receive(stcp_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds);

bool stcp_stream_ereceive(stcp_encrypted_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds);

// Frees a channel's resources in memory
void stcp_close_channel(stcp_channel* channel);
void stcp_close_encrypted_channel(stcp_encrypted_channel* echannel);

#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
