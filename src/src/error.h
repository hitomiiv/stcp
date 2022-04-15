// error.h
#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

// values: 4, 9, 13, 14, 22, 24, 35-71
typedef enum stcp_error
{
	STCP_UNKNOWN_ERROR                 = 0,

	STCP_FUNCTION_INTERRUPTED          = 4,
	STCP_IN_PROGRESS				   = 8,
	STCP_INVALID_FILE_HANDLE           = 9,
	STCP_PERMISSION_DENIED             = 13,
	STCP_INVALID_POINTER               = 14,
	STCP_INVALID_ARGUMENT              = 22,
	STCP_TOO_MANY_SOCKETS              = 24,

	STCP_WOULD_BLOCK                   = 35,
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

// Function pointer to void (stcp_error e, void* user_data)
typedef void (*stcp_error_callback_fn)(stcp_error, void*);

// Sets the error callback and optional user data pointer
void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data);

// Grabs the last network error
stcp_error stcp_get_last_error();

// Forwards an error to the error callback
void stcp_raise_error(stcp_error err);

// Convert an error to a human readable string
const char* stcp_error_to_string(stcp_error err);

#ifdef __cplusplus
}
#endif

#endif /* SRC_ERROR_H_ */
