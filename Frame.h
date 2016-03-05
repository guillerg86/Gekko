/****************************************************************
*	@File:		Frame.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/
#ifndef FRAME_H_
#define FRAME_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define FRAME_SOURCE_SIZE 14
#define FRAME_DATA_SIZE 100
#define FRAME_GEKKOTUMB_DESCONNEXIO 0
#define FRAME_GEKKOTUMB_CONNEXIO 1
#define FRAME_GEKKOTUMB_PETICIO 2

#define FRAME_GEKKODOZER_DESCONNEXIO 100
#define FRAME_GEKKODOZER_CONNEXIO 101
#define FRAME_GEKKODOZER_ERROR 102 
#define FRAME_GEKKODOZER_OK 103
#define FRAME_GEKKODOZER_ALREADYCONNECTED 104
#define FRAME_GEKKODOZER_SELLOK 105
#define FRAME_GEKKODOZER_BUYOK 106
#define FRAME_GEKKODOZER_SELLERROR 107
#define FRAME_GEKKODOZER_BUYERROR 108

typedef struct {
	char source[14];
	char type;
	char data[100];
} Frame;

Frame 	new_Frame	(void);
Frame	new_Frame_custom(int);
Frame	prepare_Frame(char nom[], char, char data[]);

#endif

