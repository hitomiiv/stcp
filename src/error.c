// error.c
#include "error.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <openssl/ssl.h>
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

void stcp_raise_ssl_error(int ssl_error)
{
	stcp_raise_error(-ssl_error);
}

const char* stcp_error_to_string(stcp_error err)
{
	switch(err)
	{
	case STCP_SSL_ERROR:
		return "SSL error";
	case STCP_SSL_SYSCALL:
		return "SSL syscall";
	case STCP_SSL_WANT_CLIENT_HELLO_CB:
		return "SSL want client hello cb";
	case STCP_SSL_WANT_ASYNC_JOB:
		return "SSL want async job";
	case STCP_SSL_WANT_ASYNC:
		return "SSL want async";
	case STCP_SSL_WANT_ACCEPT:
		return "SSL want accept";
	case STCP_SSL_WANT_CONNECT:
		return "SSL want connect";
	case STCP_SSL_ZERO_RETURN:
		return "SSL zero return";
	case STCP_SSL_WANT_X509_LOOKUP:
		return "SSL want x509 lookup";
	case STCP_SSL_WANT_WRITE:
		return "SSL want write";
	case STCP_SSL_WANT_READ:
		return "SSL want read";

	case STCP_NO_ERROR:
		return "No error";
	case STCP_EINTR:
		return strerror(EINTR);
	case STCP_EBADF:
		return strerror(EBADF);
	case STCP_EACCES:
		return strerror(EACCES);
	case STCP_EFAULT:
		return strerror(EFAULT);
	case STCP_EINVAL:
		return strerror(EINVAL);
	case STCP_EMFILE:
		return strerror(EMFILE);
	case STCP_EWOULDBLOCK:
		return strerror(EWOULDBLOCK);
	case STCP_EINPROGRESS:
		return strerror(EINPROGRESS);
	case STCP_EALREADY:
		return strerror(EALREADY);
	case STCP_ENOTSOCK:
		return strerror(ENOTSOCK);
	case STCP_EDESTADDRREQ:
		return strerror(EDESTADDRREQ);
	case STCP_EMSGSIZE:
		return strerror(EMSGSIZE);
	case STCP_EPROTOTYPE:
		return strerror(EPROTOTYPE);
	case STCP_ENOPROTOOPT:
		return strerror(ENOPROTOOPT);
	case STCP_EPROTONOSUPPORT:
		return strerror(EPROTONOSUPPORT);
	case STCP_ESOCKTNOSUPPORT:
		return "Socket not supported";
	case STCP_EOPNOTSUPP:
		return strerror(EOPNOTSUPP);
	case STCP_EPFNOSUPPORT:
		return "Protocol family not supported";
	case STCP_EAFNOSUPPORT:
		return strerror(EAFNOSUPPORT);
	case STCP_EADDRINUSE:
		return strerror(EADDRINUSE);
	case STCP_EADDRNOTAVAIL:
		return strerror(EADDRNOTAVAIL);
	case STCP_ENETDOWN:
		return strerror(ENETDOWN);
	case STCP_ENETUNREACH:
		return strerror(ENETUNREACH);
	case STCP_ENETRESET:
		return strerror(ENETRESET);
	case STCP_ECONNABORTED:
		return strerror(ECONNABORTED);
	case STCP_ECONNRESET:
		return strerror(ECONNRESET);
	case STCP_ENOBUFS:
		return strerror(ENOBUFS);
	case STCP_EISCONN:
		return strerror(EISCONN);
	case STCP_ENOTCONN:
		return strerror(ENOTCONN);
	case STCP_ESHUTDOWN:
		return "ESHUTDOWN";
	case STCP_ETOOMANYREFS:
		return "ETOOMANYREFS";
	case STCP_ETIMEDOUT:
		return "Connection timed out";
	case STCP_ECONNREFUSED:
		return strerror(ECONNREFUSED);
	case STCP_ELOOP:
		return strerror(ELOOP);
	case STCP_ENAMETOOLONG:
		return strerror(ENAMETOOLONG);
	case STCP_EHOSTDOWN:
		return "Host is down";
	case STCP_EHOSTUNREACH:
		return strerror(EHOSTUNREACH);
	case STCP_ENOTEMPTY:
		return strerror(ENOTEMPTY);
	case STCP_EPROCLIM:
		return "Too many processes";
	case STCP_EUSERS:
		return "Too many users";
	case STCP_EDQUOT:
		return "EDQUOT";
	case STCP_ESTALE:
		return "ESTALE";
	case STCP_EREMOTE:
		return "EREMOTE";
	default:
		return "Unknown STCP error";
	}
}

void stcp_print_error(stcp_error e)
{
	fprintf(stderr, "STCP error: %s\n", stcp_error_to_string(e));
	if (e == STCP_SSL_ERROR || e == STCP_SSL_SYSCALL)
	{
		int sys_err = ERR_get_error();
		if (sys_err != 0)
		{
			fprintf(stderr, "SSL error: %s\n", ERR_error_string(sys_err, NULL));
			fprintf(stderr, "SSL reason: %s\n", ERR_reason_error_string(sys_err));
		}

		ERR_print_errors_fp(stderr);
		fflush(stderr);
	}
}

void stcp_fail(stcp_error err, const char* file, int line)
{
	stcp_print_error(err);
	fprintf(stderr, "File: %s\nLine: %d\n", file, line);
	fflush(stderr);
	exit(-1);
}
