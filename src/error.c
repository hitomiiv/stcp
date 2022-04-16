// error.c
#include "error.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <errno.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <openssl/err.h>

static stcp_error_callback_fn _error_callback = NULL;
static void* _user_data = NULL;

void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data)
{
	_error_callback = error_callback;
	_user_data = user_data;
}

stcp_error stcp_get_last_error()
{
#ifdef _WIN32
	return (stcp_error) WSAGetLastError();
#else
	return (stcp_error) errno;
#endif
}

void stcp_raise_error(stcp_error err)
{
	if (_error_callback)
		_error_callback(err, _user_data);
}

const char* stcp_error_to_string(stcp_error err)
{
	if (err == STCP_NO_ERROR)
		return "No error";
	else if (err == STCP_SSL_ERROR)
		return "OpenSSL error";

#ifdef _WIN32
	else if (err == 4
		  || err == 9
		  || err == 13
		  || err == 14
		  || err == 22
		  || err == 24)
		return strerror(err);
	else if (35 <= err && err <= 71)
		return strerror(err + 50);
	else
		return "Unknown Windows error";
#else
	return strerror(err);
#endif
}

void stcp_print_error(stcp_error e)
{
	if (e == STCP_NO_ERROR)
		return;

	fprintf(stderr, "STCP error: (%d) %s\n", e, stcp_error_to_string(e));

	if (e == STCP_SSL_ERROR)
		ERR_print_errors_fp(stderr);
}
