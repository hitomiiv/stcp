// native.h
#ifndef SRC_NATIVE_H_
#define SRC_NATIVE_H_

#ifdef _WIN32
	#include <winsock2.h>
	#include <Ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
#endif

#ifdef _WIN32
	#define STCP_INVALID_SOCKET (~0ULL)
	#define STCP_SET_NON_BLOCKING(s, value) ioctlsocket(s, FIONBIO, value)
	#define STCP_SHUTDOWN_SOCKET(s) shutdown(s, SD_BOTH)
	#define STCP_CLOSE_SOCKET(s) closesocket(s)
#else
	#define STCP_INVALID_SOCKET (-1LL)
	#define STCP_SET_NON_BLOCKING(s, value) ioctl(s, FIONBIO, value)
	#define STCP_SHUTDOWN_SOCKET(s) shutdown(s, SHUT_RDWR)
	#define STCP_CLOSE_SOCKET(s) close(s)
#endif

#endif /* SRC_NATIVE_H_ */
