/****************************************************************
*	@File:		Share.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#ifndef SHARE_H_
#define SHARE_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	char *command;
	char *sharecode;
	char *quantity;
} Share;

Share 	new_Share	(void);

#endif
