/****************************************************************
*	@File:		Operator.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/
#ifndef OPERATOR_H_
#define OPERATOR_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define OPERATOR_NAME_SIZE 20

typedef struct{
	char  name[20];
	float money;
	lista myIbexList;
} Operator;

Operator	new_Operator(void);


#endif