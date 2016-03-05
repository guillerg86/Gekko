/****************************************************************
*	@File:		Config.c
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/
#include "Config.h"


/**********************************************
* @Nombre: 	new_Config
* @Def: 	Funcion que crea una structura Config inicializando los valores
* @Arg:	 	void
* @Ret:  	Out: Config: Devuelve una struct tipo Config limpia.
**********************************************/
Config 	new_Config	(void) {
	Config cfg;
	cfg.update_interval = 0;
	cfg.host_port = 0;
	bzero(cfg.host_ip,16);

	return cfg;
}

