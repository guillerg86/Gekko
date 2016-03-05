/****************************************************************
*	@File:		dozer.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/
#ifndef DOZER_H_
#define DOZER_H_


#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <pthread.h>

#include "lista.h"
#include "Share.h"

#define TRUE 0
#define FALSE 1

#define FD_SCREEN 1
#define FD_KEYBOARD 0

#define OPERATOR_NAME_SIZE 20
#define STRING_BUFFER_SIZE 255

typedef struct{
	char name[20];
	float money;
	lista acciones;
} Operator;

void irq_handler(int);
void print_message(char *);
void initGlobals(void);
void closeGlobals(void);
void disconnect_gekko(void);
int connect_gekko(void);
void *thread_listen_gekko(void *);

void string_to_lowercase(char *, int);
void string_to_uppercase(char *, int);
void closeDozer(void);
int read_file_config_gekko(char file[]);
int read_file_dtbase_stock(char file[]);

int desglosa(Share *, char[]);
void shell_show_me_the_money(void);
void shell_show_stock(void);
int menu(void);
void write_file_stock(void);

void print_ibex_list(Frame);

void eraseAccion(Frame);
void buyAccion(Frame);
void sellAccion(Frame);
void warnDozer(Frame);

#endif
