// error.h
#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _WIN32
#include <errno.h>
#endif

// Portable error codes
typedef enum stcp_error
{
	// SSL errors
    STCP_SSL_WANT_CLIENT_HELLO_CB = -11,
    STCP_SSL_WANT_ASYNC_JOB       = -10,
    STCP_SSL_WANT_ASYNC           = -9,
    STCP_SSL_WANT_ACCEPT          = -8,
    STCP_SSL_WANT_CONNECT         = -7,
    STCP_SSL_ZERO_RETURN          = -6,
    STCP_SSL_SYSCALL              = -5,
    STCP_SSL_WANT_X509_LOOKUP     = -4,
    STCP_SSL_WANT_WRITE           = -3,
    STCP_SSL_WANT_READ            = -2,
    STCP_SSL_ERROR                = -1,

	STCP_NO_ERROR			      = 0,

#ifdef _WIN32 // Winsock errors. Range: 10000 + 4, 9, 13, 14, 22, 24, 35-71
	STCP_EINTR                    = 10004,
	STCP_EBADF                    = 10009,
	STCP_EACCES                   = 10013,
	STCP_EFAULT                   = 10014,
	STCP_EINVAL                   = 10022,
	STCP_EMFILE                   = 10024,
	STCP_EWOULDBLOCK	          = 10035,
	STCP_EINPROGRESS	          = 10036,
	STCP_EALREADY	              = 10037,
	STCP_ENOTSOCK	              = 10038,
	STCP_EDESTADDRREQ	          = 10039,
	STCP_EMSGSIZE	              = 10040,
	STCP_EPROTOTYPE	              = 10041,
	STCP_ENOPROTOOPT	          = 10042,
	STCP_EPROTONOSUPPORT	      = 10043,
	STCP_ESOCKTNOSUPPORT	      = 10044,
	STCP_EOPNOTSUPP	              = 10045,
	STCP_EPFNOSUPPORT	          = 10046,
	STCP_EAFNOSUPPORT	          = 10047,
	STCP_EADDRINUSE	              = 10048,
	STCP_EADDRNOTAVAIL	          = 10049,
	STCP_ENETDOWN	              = 10050,
	STCP_ENETUNREACH	          = 10051,
	STCP_ENETRESET	              = 10052,
	STCP_ECONNABORTED	          = 10053,
	STCP_ECONNRESET	              = 10054,
	STCP_ENOBUFS		          = 10055,
	STCP_EISCONN		          = 10056,
	STCP_ENOTCONN	              = 10057,
	STCP_ESHUTDOWN	              = 10058,
	STCP_ETOOMANYREFS	          = 10059,
	STCP_ETIMEDOUT	              = 10060,
	STCP_ECONNREFUSED	          = 10061,
	STCP_ELOOP		              = 10062,
	STCP_ENAMETOOLONG	          = 10063,
	STCP_EHOSTDOWN	              = 10064,
	STCP_EHOSTUNREACH	          = 10065,
	STCP_ENOTEMPTY	              = 10066,
	STCP_EPROCLIM	              = 10067,
	STCP_EUSERS		              = 10068,
	STCP_EDQUOT		              = 10069,
	STCP_ESTALE		              = 10070,
	STCP_EREMOTE		          = 10071,
#else
	STCP_EINTR                    = EINTR,
	STCP_EBADF                    = EBADF,
	STCP_EACCES                   = EACCES,
	STCP_EFAULT                   = EFAULT,
	STCP_EINVAL                   = EINVAL,
	STCP_EMFILE                   = EMFILE,
	STCP_EWOULDBLOCK	          = EWOULDBLOCK,
	STCP_EINPROGRESS	          = EINPROGRESS,
	STCP_EALREADY	              = EALREADY,
	STCP_ENOTSOCK	              = ENOTSOCK,
	STCP_EDESTADDRREQ	          = EDESTADDRREQ,
	STCP_EMSGSIZE	              = EMSGSIZE,
	STCP_EPROTOTYPE	              = EPROTOTYPE,
	STCP_ENOPROTOOPT	          = ENOPROTOOPT,
	STCP_EPROTONOSUPPORT	      = EPROTONOSUPPORT,
	STCP_ESOCKTNOSUPPORT	      = ESOCKTNOSUPPORT,
	STCP_EOPNOTSUPP	              = EOPNOTSUPP,
	STCP_EPFNOSUPPORT	          = EPFNOSUPPORT,
	STCP_EAFNOSUPPORT	          = EAFNOSUPPORT,
	STCP_EADDRINUSE	              = EADDRINUSE,
	STCP_EADDRNOTAVAIL	          = EADDRNOTAVAIL,
	STCP_ENETDOWN	              = ENETDOWN,
	STCP_ENETUNREACH	          = ENETUNREACH,
	STCP_ENETRESET	              = ENETRESET,
	STCP_ECONNABORTED	          = ECONNABORTED,
	STCP_ECONNRESET	              = ECONNRESET,
	STCP_ENOBUFS		          = ENOBUFS,
	STCP_EISCONN		          = EISCONN,
	STCP_ENOTCONN	              = ENOTCONN,
	STCP_ESHUTDOWN	              = ESHUTDOWN,
	STCP_ETOOMANYREFS	          = ETOOMANYREFS,
	STCP_ETIMEDOUT	              = ETIMEDOUT,
	STCP_ECONNREFUSED	          = ECONNREFUSED,
	STCP_ELOOP		              = ELOOP,
	STCP_ENAMETOOLONG	          = ENAMETOOLONG,
	STCP_EHOSTDOWN	              = EHOSTDOWN,
	STCP_EHOSTUNREACH	          = EHOSTUNREACH,
	STCP_ENOTEMPTY	              = ENOTEMPTY,
	STCP_EPROCLIM	              = 83, // No idea why EPROCLIM gives errors on WSL Ubuntu 20.04
	STCP_EUSERS		              = EUSERS,
	STCP_EDQUOT		              = EDQUOT,
	STCP_ESTALE		              = ESTALE,
	STCP_EREMOTE		          = EREMOTE,
#endif
} stcp_error;

// Function pointer to void (stcp_error e, void* user_data)
typedef void (*stcp_error_callback_fn)(stcp_error, void*);

// Sets the error callback and optional user data pointer
void stcp_set_error_callback(stcp_error_callback_fn error_callback, void* user_data);

// Grabs the last network error
stcp_error stcp_get_last_error();

// Forwards an error to the error callback
void stcp_raise_error(stcp_error err);
void stcp_raise_ssl_error(int err);

// Convert an error to a human readable string
const char* stcp_error_to_string(stcp_error err);

// Print any and all errors
void stcp_print_error();

// Non-recoverable errors. Bypasses the debug callback
void stcp_fail(stcp_error err, const char* file, int line);
#define STCP_FAIL(err) stcp_fail(err, __FILE__, __LINE__)
#define STCP_FAIL_LAST_ERROR() stcp_fail(stcp_get_last_error(), __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* SRC_ERROR_H_ */
