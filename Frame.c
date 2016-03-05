/****************************************************************
*	@File:		Frame.c
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/
#include "Frame.h"

/**********************************************
* @Nombre: 	new_Frame
* @Def: 	Funcion que simula un constructor de POO , creando y iniciando las variables.
* @Arg:	 	void
* @Ret:  	Out: Frame: Devuelve una trama limpia.
**********************************************/
Frame 	new_Frame	(void) {
	Frame frame;
	bzero(frame.source,FRAME_SOURCE_SIZE);
	bzero(frame.data, FRAME_DATA_SIZE);
	frame.type = '\0';

	return frame;
}
/**********************************************
* @Nombre: 	new_Frame_custom
* @Def: 	Funcion que simula ser un constructor, permitiendo crear diferentes tramas.
* @Arg:	 	In: int type: Tipo de trama que queremos crear.
* @Ret:  	Out: Frame: Devuelve la trama creada y con los datos del tipo solicitado
**********************************************/
Frame	new_Frame_custom(int type) {
	Frame frame = new_Frame();
	
	
	switch (type) {
		case FRAME_GEKKOTUMB_CONNEXIO:
			strcpy(frame.source,"Gekko");
			strcpy(frame.data,"CONNEXIO");
			frame.type = 'C';
		break;
		case FRAME_GEKKOTUMB_PETICIO:
			strcpy(frame.source,"Gekko");
			strcpy(frame.data,"PETICIO");
			frame.type = 'P';		
		break;
		case FRAME_GEKKOTUMB_DESCONNEXIO:
			strcpy(frame.source,"Gekko");
			strcpy(frame.data,"DESCONNEXIO");
			frame.type = 'Q';		
		break;
		case FRAME_GEKKODOZER_ERROR:
			strcpy(frame.source,"Gekko");
			strcpy(frame.data,"ERROR");
			frame.type = 'E';
		break;
		case FRAME_GEKKODOZER_CONNEXIO:
			strcpy(frame.data,"CONNEXIO");
			frame.type = 'C';
		break;
		case FRAME_GEKKODOZER_DESCONNEXIO:
			strcpy(frame.data,"DESCONNEXIO");
			frame.type = 'Q';
		break;
		case FRAME_GEKKODOZER_OK: 
			strcpy(frame.data,"CONNEXIO OK");
			strcpy(frame.source,"Gekko");
			frame.type = 'O';
		break;
		case FRAME_GEKKODOZER_ALREADYCONNECTED:
			strcpy(frame.data,"JA CONNECTAT EN UN ALTRE TERMINAL");
			strcpy(frame.source,"Gekko");
			frame.type = 'A';
		break;
		case FRAME_GEKKODOZER_SELLOK:
			strcpy(frame.data,"OK");
			strcpy(frame.source,"Gekko");
			frame.type = 'S';
		break;
		
		default:
		break;
	}
	
	return frame;
}
/**********************************************
* @Nombre: 	prepare_Frame
* @Def: 	
* @Arg:	 	void
* @Ret:  	Out: Frame: Devuelve una trama limpia.
**********************************************/
Frame prepare_Frame(char nom[], char tipo, char datos[]){
	Frame frame = new_Frame();
	int i;
	strcpy(frame.source, nom);
	for (i = strlen(nom); i < FRAME_SOURCE_SIZE; i++){
		frame.source[i] = '\0';
	}
	frame.type = tipo;
	strcpy(frame.data, datos);
	for (i= strlen(datos); i<FRAME_DATA_SIZE; i++) {
		frame.data[i] = '\0';
	}
	return frame;
}

