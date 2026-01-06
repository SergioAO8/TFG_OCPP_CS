#
#	FILE
#	    makefile - makefile del nucli del sistema.
#	PROJECT
#	    TFG - Implementació d'un Sistema de Control per Punts de Càrrega de Vehicles Elèctrics.
#	DESCRIPTION
#	    Makefile per compilar el nucli del sistema.
#	AUTHOR
#	    Sergio Abate
#	OPERATING SYSTEM
#	    Linux

# variables
SRCS = $(wildcard *.c)
SRCS_JSON_CODEC = $(wildcard ../json_codec/*.c)
SRCS_LIB_WS = $(wildcard ../lib_ws/*.c)
SRCS_OCPP_REQUESTS = $(wildcard ../ocpp_requests/*.c)
OBJS = $(SRCS:.c=.o)
OBJS_JSON_CODEC = $(SRCS_JSON_CODEC:.c=.o)
OBJS_LIB_WS = $(SRCS_LIB_WS:.c=.o)
OBJS_OCPP_REQUESTS = $(SRCS_OCPP_REQUESTS:.c=.o)
DEPS = $(SRCS:.c=.d)
DEPS_JSON_CODEC = $(SRCS_JSON_CODEC:.c=.d)
DEPS_LIB_WS = $(SRCS_LIB_WS:.c=.d)
DEPS_OCPP_REQUESTS = $(SRCS_OCPP_REQUESTS:.c=.d)

# compilador i linker
CC = gcc

# flag per al preprocessador de cc durant la creació dels fitxers objecte
CPPFLAGS = -I. -I../json_codec -I../ocpp_requests -I/usr/include/cjson -I/usr/local/include/wsserver -pthread -O2 -Wall -Wno-unused-function -MMD -MP
# CPPFLAGS = -I../lib -I/usr/include/cjson -g -O2 -Wall -pedantic -MMD -MP

# flag pel linker ld durant la creació del programa executable
LDFLAGS = -lcjson -llist -lsqlite3 ../lib_ws/libws.a

# creació de l'executable
ocpp_cs: $(OBJS) $(OBJS_JSON_CODEC) $(OBJS_LIB_WS) $(OBJS_OCPP_REQUESTS)
	$(CC) $^ -o $@ $(LDFLAGS)

# creació dels fitxers objecte
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) $(OBJS) $(OBJS_JSON_CODEC) $(OBJS_LIB_WS) $(OBJS_OCPP_REQUESTS) $(DEPS) $(DEPS_JSON_CODEC) $(DEPS_LIB_WS) $(DEPS_OCPP_REQUESTS) server

-include $(DEPS) $(DEPS_JSON_CODEC) $(DEPS_LIB_WS) $(DEPS_OCPP_REQUESTS)
