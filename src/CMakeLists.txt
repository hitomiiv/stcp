cmake_minimum_required(VERSION 3.12)

add_library(stcp SHARED error.c error.h socket.c socket.h stcp.c stcp.h)

if(WIN32)
	target_link_libraries(stcp PUBLIC ws2_32)
endif()