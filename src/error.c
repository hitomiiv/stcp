// error.c
#include "error.h"

#include <winsock2.h>
#include <errno.h>

static stcp_error_callback_fn _error_callback;
static void* _user_data;

void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data)
{
	_error_callback = error_callback;
	_user_data = user_data;
}

stcp_error stcp_get_last_error()
{
#ifdef _WIN32
	int error = WSAGetLastError() - 10000;
#else
	int error = errno;
#endif
	if (error == 4
			|| error == 9
			|| error == 13
			|| error == 14
			|| error == 22
			|| error == 24
			|| (35 <= error && error <= 71))
	{
		return (stcp_error) error;
	}
	else
	{
		return STCP_UNKNOWN_ERROR;
	}
}

void stcp_raise_error(stcp_error err)
{
	if (_error_callback)
		_error_callback(err, _user_data);
}
