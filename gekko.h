/****************************************************************
*	@File:		gekko.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#ifndef GEKKO_H_
#define GEKKO_H_

/* INCLUDES */ 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>
#include <pthread.h>

#include "lista_dozer.h"
#include "lista_a_la_venta.h"

#define TRUE 0
#define FALSE 1

/* FILE DESCRIPTORS */
#define FD_SCREEN 1
#define FD_KEYBOARD 0

/* CONSTANTS */
#define TUMBDICE_RECONNECT_WAIT 5
#define TUMBIDCE_RECONNECT_ATTEMPTS 5

#define UPDATE_IBEX_ERROR 10
#define UPDATE_IBEX_OK 20

#define STRING_BUFFER_SIZE 255

#define SERVER_PORT 8471
#define MAX_DOZERS 20




/* READ FILE FUNCTIONS */
void 	read_file_dtbase_ibexinfo(char file[]);
void 	read_file_config_tumbdice(char file[]);

void 	print_message(char *message);

/* SOCKET FUNCTIONS */
// Tumbling
int 	connect_tumbdice();
void 	disconnect_tumbdice();
int 	updateIbexList(void);
int		updateIbex(Frame);

// Dozers
void 	run_dozer_thread(int);
void	listen_dozers(void);
void 	disconnect_dozers();


void 	buyAccion(int,Frame);
void 	eraseAccion(int,Frame);
void 	sellAccion(int,Frame);
void 	showIbex(int);


/* INTERRUPTIONS FUNCTIONS */
void 	irq_handler(int);

/* CLEANERS */
void cleanIbex(void);
void initGlobals(void);
void closeGlobals(void);
void initVentas	(void);
void closeVentas(void);
void exitGekko(void);

#endif
