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

// initialization and error handling
bool stcp_initialize();
void stcp_terminate();
int stcp_get_last_error();
const char* stcp_resolve_error(int error);

// addresses
stcp_address* stcp_create_address_ipv4(const char* ipv4, int port);
stcp_address* stcp_create_address_hostname(const char* hostname, int port);
void stcp_free_address(stcp_address* address);

// servers
stcp_server* stcp_create_server(const stcp_address* server_address, int max_pending_clients);
stcp_channel* stcp_accept_channel(const stcp_server* server, int timeout_milliseconds);
void stcp_free_server(stcp_server* server);

// channels
stcp_channel* stcp_create_channel(const stcp_address* server_address);
bool stcp_send(const stcp_channel* channel, const char* buffer, int length, int timeout_milliseconds);
bool stcp_receive(const stcp_channel* channel, char* buffer, int length, int timeout_milliseconds);
void stcp_free_channel(stcp_channel* channel);

#ifdef __cplusplus
}
#endif

#endif /* STCP_H_ */
