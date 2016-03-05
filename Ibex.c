/****************************************************************
*	@File:		Ibex.c
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#include "Ibex.h"


/**********************************************
* @Nombre: 	new_Ibex
* @Def: 	Funcion que simula un constructor de POO , creando y iniciando las variables.
* @Arg:	 	void
* @Ret:  	Out: Ibex: Devuelve una struct Ibex limpia.
**********************************************/
Ibex 	new_Ibex	(void) {
	Ibex ibex;
	ibex.value 		= 0;
	ibex.quantity	= 0;
	bzero(ibex.code,IBEX_CODE_SIZE);
	return ibex;
}

