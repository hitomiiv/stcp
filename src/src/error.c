// error.c
#include "error.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <errno.h>
#include <stddef.h>

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
	int error = WSAGetLastError() - 10000;
#else
	int error = errno - 50;
#endif
	if (error == 4
		|| error == 9
		|| error == 13
		|| error == 14
		|| error == 22
		|| error == 24
		|| (35 <= error && error <= 71))
		return (stcp_error) error;
	else
		return STCP_UNKNOWN_ERROR;
}

void stcp_raise_error(stcp_error err)
{
	if (_error_callback)
		_error_callback(err, _user_data);
}

const char* stcp_error_to_string(stcp_error err)
{
	// index 0 is error 35
	const char* error_table[] =
	{
		"STCP_WOULD_BLOCK",
		"STCP_BLOCKED_BY_CALLER",
		"STCP_BLOCKED_BY_CALLEE",
		"STCP_INVALID_SOCKET",
		"STCP_INVALID_DESTINATION",
		"STCP_MESSAGE_TOO_LONG",
		"STCP_INVALID_PROTOCOL",
		"STCP_INVALID_PROTOCOL_OPTION",
		"STCP_PROTOCOL_NOT_SUPPORTED",
		"STCP_SOCKET_NOT_SUPPORTED",
		"STCP_OPERATION_NOT_SUPPORTED",
		"STCP_PROTOCOL_FAMILY_NOT_SUPPORTED",
		"STCP_ADDRESS_FAMILY_NOT_SUPPORTED",
		"STCP_ADDRESS_IN_USE",
		"STCP_ADDRESS_NOT_AVAILABLE",
		"STCP_NETWORK_DOWN",
		"STCP_NETWORK_UNREACHABLE",
		"STCP_NETWORK_RESET",
		"STCP_CONNECTION_ABORTED",
		"STCP_CONNECTION_RESET",
		"STCP_NO_BUFFER_SPACE",
		"STCP_SOCKET_ALREADY_CONNECTED",
		"STCP_SOCKET_NOT_CONNECTED",
		"STCP_SOCKET_SHUTDOWN",
		"STCP_TOO_MANY_REFERENCES",
		"STCP_CONNECTION_TIMED_OUT",
		"STCP_CONNECTION_REFUSED",
		"STCP_INVALID_NAME",
		"STCP_NAME_TOO_LONG",
		"STCP_HOST_DOWN",
		"STCP_HOST_UNREACHABLE",
		"STCP_DIRECTORY_NOT_EMPTY",
		"STCP_TOO_MANY_PROCESSES",
		"STCP_TOO_MANY_USERS",
		"STCP_OUT_OF_DISK_SPACE",
		"STCP_OLD_HANDLE",
		"STCP_RESOURCE_NOT_LOCAL",
	};

	// values: 4, 9, 13, 14, 22, 24, 35-71
	switch (err)
	{
	case 4:  return "STCP_FUNCTION_INTERRUPTED"; break;
	case 9:  return "STCP_INVALID_FILE_HANDLE"; break;
	case 13: return "STCP_PERMISSION_DENIED"; break;
	case 14: return "STCP_INVALID_POINTER"; break;
	case 22: return "STCP_INVALID_ARGUMENT"; break;
	case 24: return "STCP_TOO_MANY_SOCKETS"; break;
	default:
		if (35 <= err && err <= 71)
		{
			return error_table[err - 35];
		}
		else
		{
			return "STCP_UNKNOWN_ERROR";
		}
	}
}
