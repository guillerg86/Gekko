/****************************************************************
*	@File:		Ibex.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#ifndef IBEX_H_
#define IBEX_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define IBEX_CODE_SIZE 5
#define IBEX_LIST_SIZE 35

typedef struct{
	char  code[5];
	float value;
	long long int quantity;
} Ibex;

Ibex 	new_Ibex	(void);

#endif
