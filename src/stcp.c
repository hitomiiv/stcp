// stcp.c

#include "stcp.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

// sometimes these get long
#define MALLOC(type) (type*) malloc(sizeof(type))
#define REALLOC(ptr, type) (type*) realloc(ptr, sizeof(type))

// ----- TCP/IP socket types -----
typedef struct stcp_channel
{
	socket_t socket;
} stcp_channel;

typedef struct stcp_server
{
	socket_t socket;
} stcp_server;

typedef struct stcp_encrypted_channel
{
	stcp_channel channel;
	SSL* ssl;
} stcp_encrypted_channel;


// ----- Initialization -----
static bool init = false;
static SSL_CTX* ssl_ctx = NULL;

bool stcp_initialize()
{
	if (!init)
	{
		// socket initialization
		stcp_socket_initialize_library();

		// OpenSSL initialization
		SSL_library_init();
		SSL_load_error_strings();

		ssl_ctx = SSL_CTX_new(TLS_method());
		if (!ssl_ctx)
		{
			stcp_raise_error(STCP_SSL_ERROR);
			return false;
		}

		init = true;
	}

	return true;
}

void stcp_terminate()
{
	if (init)
	{
		// OpenSSL cleanup
		SSL_CTX_free(ssl_ctx);
		ssl_ctx = NULL;

		ERR_free_strings();

		// socket cleanup
		stcp_socket_terminate_library();

		init = false;
	}
}

// ----- Type conversion -----
static stcp_encrypted_channel* stcp_encrypt(stcp_channel* channel)
{
	assert(channel);

	stcp_encrypted_channel* echannel = REALLOC(channel, stcp_encrypted_channel);
	assert(echannel);

	echannel->ssl = SSL_new(ssl_ctx);
	if (!echannel->ssl)
	{
		stcp_raise_error(STCP_SSL_ERROR);
		stcp_close_encrypted_channel(echannel);
		return NULL;
	}

	int ret = SSL_set_fd(echannel->ssl, echannel->channel.socket);
	if (ret <= 0)
	{
		stcp_raise_ssl_error(SSL_get_error(echannel->ssl, ret));
		stcp_close_encrypted_channel(echannel);
		return NULL;
	}

	return echannel;
}

// ----- Servers -----
stcp_server* stcp_open_server(const char* address, const char* protocol, int max_pending_channels)
{
	stcp_server* server = MALLOC(stcp_server);
	assert(server);
	assert(address);
	assert(max_pending_channels > 0);

	server->socket = stcp_socket_create();
	stcp_socket_bind(&server->socket, address, protocol);
	stcp_socket_listen(&server->socket, max_pending_channels);
	return server;
}

stcp_channel* stcp_accept(stcp_server* server, int timeout_milliseconds)
{
	assert(server);

	if (!stcp_socket_poll_read(&server->socket, timeout_milliseconds))
		return NULL;

	stcp_channel* channel = MALLOC(stcp_channel);
	assert(channel);
	channel->socket = stcp_socket_accept(&server->socket);
	return channel;
}

stcp_encrypted_channel* stcp_eaccept(stcp_server* server,
		int timeout_milliseconds)
{
	assert(server);

	stcp_channel* channel = stcp_accept(server, timeout_milliseconds);
	if (!channel)
		return NULL;

	stcp_encrypted_channel* echannel = stcp_encrypt(channel);
	if (!echannel)
		return NULL;

	SSL_set_accept_state(echannel->ssl);

	int ret = SSL_accept(echannel->ssl);
	if (ret <= 0)
	{
		stcp_raise_ssl_error(SSL_get_error(echannel->ssl, ret));
		stcp_close_encrypted_channel(echannel);
		return NULL;
	}

	return echannel;
}

void stcp_close_server(stcp_server* server)
{
	if (server)
	{
		stcp_socket_close(&server->socket);
		free(server);
	}
}

// ----- Channels -----
stcp_channel* stcp_connect(const char* address, const char* protocol)
{
	stcp_channel* channel = (stcp_channel*) malloc(sizeof(stcp_channel));
	assert(channel);
	assert(address);
	assert(protocol);

	channel->socket = stcp_socket_create();
	stcp_socket_connect(&channel->socket, address, protocol);
	return channel;
}

stcp_encrypted_channel* stcp_econnect(const char* address,
		const char* protocol,
		int timeout_milliseconds)
{
	assert(address);
	assert(protocol);

	stcp_channel* channel = stcp_connect(address, protocol);
	if (!channel)
		return NULL;

	stcp_encrypted_channel* echannel = stcp_encrypt(channel);

	SSL_set_connect_state(echannel->ssl);

	if (!stcp_socket_poll_read(&echannel->channel.socket, timeout_milliseconds))
		return false;

	int ret = SSL_do_handshake(echannel->ssl);
	if (ret <= 0)
	{
		stcp_raise_ssl_error(SSL_get_error(echannel->ssl, ret));
		stcp_close_encrypted_channel(echannel);
		return NULL;
	}

	return echannel;
}

bool stcp_send(stcp_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_write(&channel->socket, timeout_milliseconds))
		return false;

	int bytes_sent = 0;
	while (bytes_sent < length)
	{
		int ret = stcp_socket_write(&channel->socket,
				buffer + bytes_sent,
				length - bytes_sent);

		if (ret == 0)
			return false;

		bytes_sent += ret;
	}

	return true;
}

bool stcp_esend(stcp_encrypted_channel* channel,
		const char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_write(&channel->channel.socket, timeout_milliseconds))
		return false;

	int ret = SSL_write(channel->ssl, buffer, length);
	if (ret <= 0)
	{
		stcp_raise_ssl_error(SSL_get_error(channel->ssl, ret));
		return false;
	}

	return true;
}

int stcp_receive(stcp_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_read(&channel->socket, timeout_milliseconds))
		return 0;

	return stcp_socket_read(&channel->socket, buffer, length);
}

int stcp_ereceive(stcp_encrypted_channel* channel,
		char* buffer,
		int length,
		int timeout_milliseconds)
{
	assert(channel);
	assert(buffer);
	assert(length > 0);

	if (!stcp_socket_poll_read(&channel->channel.socket, timeout_milliseconds))
		return 0;

	int ret = SSL_read(channel->ssl, buffer, length);
	if (ret <= 0)
	{
		stcp_raise_ssl_error(SSL_get_error(channel->ssl, ret));
		return 0;
	}

	return ret;
}

bool stcp_stream_receive(stcp_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds)
{
	assert(channel);
	assert(stream_output);

	if (!stcp_socket_poll_read(&channel->socket, timeout_milliseconds))
		return false;

	char buffer[STCP_STREAM_BUFFER_SIZE];
	const int length = STCP_STREAM_BUFFER_SIZE;

	do
	{
		int bytes_received = stcp_socket_read(&channel->socket, buffer, length);

		if (bytes_received == 0)
			return false;

		if (!stream_output(buffer, bytes_received, user_data))
			return false;

	} while (stcp_socket_poll_read(&channel->socket, 0));

	return true;
}

bool stcp_stream_ereceive(stcp_encrypted_channel* channel,
		stream_output_fn stream_output,
		void* user_data,
		int timeout_milliseconds)
{
	assert(channel);
	assert(stream_output);

	if (!stcp_socket_poll_read(&channel->channel.socket, timeout_milliseconds))
		return false;

	char buffer[STCP_STREAM_BUFFER_SIZE];
	const int length = STCP_STREAM_BUFFER_SIZE;

	do
	{
		int ret = SSL_read(channel->ssl, buffer, length);
		if (ret <= 0)
		{
			stcp_raise_ssl_error(SSL_get_error(channel->ssl, ret));
			return false;
		}

		if (!stream_output(buffer, ret, user_data))
			return false;

	} while (stcp_socket_poll_read(&channel->channel.socket, 0));

	return true;
}

void stcp_close_channel(stcp_channel* channel)
{
	if (channel)
	{
		stcp_socket_close(&channel->socket);
		free(channel);
	}
}

void stcp_close_encrypted_channel(stcp_encrypted_channel* echannel)
{
	if (echannel)
	{
		SSL_free(echannel->ssl);
		stcp_socket_close(&echannel->channel.socket);
		free(echannel);
	}
}
