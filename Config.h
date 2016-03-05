/****************************************************************
*	@File:		Config.h
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	int update_interval;
	int host_port;
	char host_ip[16];
} Config;

Config 	new_Config	(void);

#endif
