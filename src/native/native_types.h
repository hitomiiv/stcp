// native_types.h
#ifndef SRC_NATIVE_TYPES_H_
#define SRC_NATIVE_TYPES_H_

#ifdef _WIN32
	typedef unsigned long long int socket_t;
#else
	typedef long long int socket_t;
#endif

#endif /* SRC_NATIVE_TYPES_H_ */
