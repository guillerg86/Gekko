/****************************************************************
*	@File:		gekko.c
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/


#include "Ibex.h"
#include "Frame.h"
#include "Config.h"
#include "gekko.h"

/* GLOBALS */
int 	fd_socket_tumbdice, current_reconnect_attempt, error;
Ibex 	ibexList[IBEX_LIST_SIZE];
Config	config_tumbdice;
Clients_dozers clients;
Clients_dozers ventes_pending;
Ventas v[35];
int readers,ventes_readers;


static pthread_mutex_t mtx_alarm 	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_readers 	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_writers	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_conlist	= PTHREAD_MUTEX_INITIALIZER; 
static pthread_mutex_t mtx_pending	= PTHREAD_MUTEX_INITIALIZER; 
static pthread_mutex_t mtx_ventes_readers = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_ventes_writers = PTHREAD_MUTEX_INITIALIZER;


/**********************************************
* @Nombre: 	thread_manage_dozer
* @Def: 	Funcion encargada de gestionar a un Dozer.
* @Arg:	 	*arg: Recibe el socket del cliente
* @Ret:  	void
**********************************************/
void *thread_manage_dozer(void *arg) {
	char buffer[STRING_BUFFER_SIZE];
	Dozer auxDozer = new_Dozer();
	Dozer client = new_Dozer();
	
	int client_socket = (int) arg;
	char operator[14]; bzero(operator,14);
	// int error = FALSE;
	
	int readed_socket_bytes = 0;
	
	Frame frameTX = new_Frame();
	Frame frameRX = new_Frame();
	readed_socket_bytes = read(client_socket, &frameRX, sizeof(frameRX));
	
	if (readed_socket_bytes < 0 ) {
		print_message("[Gekko] - ERROR: No se pudo leer informacion del socket.");
		pthread_exit(NULL);
	} else {
		// Primero comprobamos que se reciba un frame de inicio de conexion
		if ( (frameRX.type == 'C' ) && strcmp(frameRX.data,"CONNEXIO") == 0 ) {
			strcpy(client.sOperador,frameRX.source);
			client.nFdSocket = client_socket;

			
			pthread_mutex_lock(&mtx_conlist);
			if (  LISTA_DOZER_BuscaDozer(&clients, frameRX.source ) != -1 ) {
				pthread_mutex_unlock(&mtx_conlist);
				
				frameTX = new_Frame_custom(FRAME_GEKKODOZER_ALREADYCONNECTED);
				write(client_socket, &frameTX, sizeof(frameTX));
				close(client_socket); 
				pthread_exit(NULL);
			} else {
				// Añadimos el dozer a la lista de clientes conectados
				LISTA_DOZER_Insereix(&clients, client);
				pthread_mutex_unlock(&mtx_conlist);
				
				frameTX = new_Frame_custom(FRAME_GEKKODOZER_OK);
				write(client_socket, &frameTX, sizeof(frameTX));
				
				bzero(buffer,STRING_BUFFER_SIZE);
				sprintf(buffer,"[Gekko] - INFO: Conexion aceptada para dozer con operador: %s - socket: %d\n",frameRX.source, client_socket);
				print_message(buffer);
			}
			
		// START PENDING NOTIFY 
			pthread_mutex_lock(&mtx_pending);
			LISTA_DOZER_VesInici(&ventes_pending);
			while(!LISTA_DOZER_Final(ventes_pending)){
				auxDozer = LISTA_DOZER_Consulta(ventes_pending);
				if(strcmp(auxDozer.sOperador, frameRX.source) == 0){
					frameTX = prepare_Frame("Gekko",'M', auxDozer.data_enviar);
					write(client_socket, &frameTX, sizeof(frameTX));
					LISTA_DOZER_Elimina(&ventes_pending);
				} else {
					LISTA_DOZER_Avanza(&ventes_pending);
				}
			}
		// END PENDING NOTIFY	
			pthread_mutex_unlock(&mtx_pending);
			while ( frameRX.type != 'Q' ) {
				// Escuchamos lo que tiene que decir el Dozer
				frameRX = new_Frame();
				readed_socket_bytes = read(client_socket, &frameRX, sizeof(frameRX));

				if (readed_socket_bytes < 0 ) {
					// Si leemos 0 bytes, el socket ha caido, cerramos socket y thread
					close(client_socket); 
					pthread_exit(NULL);
				}else{
					switch (frameRX.type) {
						case 'Q':
							// No hace nada porque es la condicion de salida del bucle.
							break;
						case 'X':
							//Show IBEX
							showIbex(client_socket);
							bzero(buffer,STRING_BUFFER_SIZE);
							sprintf(buffer,"[Gekko] - INFO: Recibida peticion de show Ibex del operador %s \n",frameRX.source);
							print_message(buffer);
							break;
						case 'B':
							//BUY
							pthread_mutex_lock(&mtx_ventes_writers);
							buyAccion(client_socket, frameRX);
							pthread_mutex_unlock(&mtx_ventes_writers);
							break;
						case 'S':
							//SELL
							pthread_mutex_lock(&mtx_ventes_writers);
							sellAccion(client_socket, frameRX);
							pthread_mutex_unlock(&mtx_ventes_writers);
							break;
						case 'D':
							//Esborrar
							pthread_mutex_lock(&mtx_ventes_writers);
							eraseAccion(client_socket, frameRX);
							pthread_mutex_unlock(&mtx_ventes_writers);
							break;
						default:
							break;
					}
				}
			}
			
			pthread_mutex_lock(&mtx_conlist);
			LISTA_DOZER_EliminaDozer(&clients, client.sOperador);	
			pthread_mutex_unlock(&mtx_conlist);	
			bzero(buffer,STRING_BUFFER_SIZE);
			sprintf(buffer,"[Gekko] - INFO: Conexion cerrada del operador: %s \n",client.sOperador);
			print_message(buffer);

		} else {
			frameTX = new_Frame_custom(FRAME_GEKKODOZER_ERROR);
			write(client_socket, &frameTX, sizeof(frameTX));
			close(client_socket);
			pthread_exit(NULL);
		}
	}
	
	close(client_socket);
	pthread_exit(NULL);
}

/**********************************************
* @Nombre: 	thread_update_ibex
* @Def: 	Funcion encargada de realizar las actualizaciones de los valores Ibex, se ejecuta en un thread.
* @Arg:	 	*arg: Recibe el mensaje que mostrara por pantalla cuando se inicie el Thread 
* @Ret:  	void
**********************************************/
void *thread_update_ibex(void *arg) {
	int result = 0;
	int boolContinue = TRUE;
	char *message = (char *) arg;

	print_message(message);
	print_message("\n");
	
	while ( boolContinue == TRUE ) {
		// Lock the mutex before updateIbex
		result = pthread_mutex_lock(&mtx_alarm);
		if ( result != 0 ) {
			print_message("[Gekko] - ERROR: No se pudo bloquear el mutex de alarma");
		} else {
			pthread_mutex_lock(&mtx_writers);
			result = updateIbexList();
			pthread_mutex_unlock(&mtx_writers);
			if ( result == UPDATE_IBEX_OK ) {
				alarm(config_tumbdice.update_interval);
			} else {
				alarm(TUMBDICE_RECONNECT_WAIT);
			}
		}		
	}
	pthread_exit(NULL);
}

/**********************************************
* @Nombre: 	main
* @Def: 	Funcion principal del programa. Lanza el thread_update_ibex y muestra shell al usuario.
* @Arg:	 	void
* @Ret:  	Out:	Retorna un Integer para indicar al operativo el resultado de la ejecucion.
**********************************************/
int main(void) {
	
	initGlobals();	// Iniciamos las globales
	//int close = 0;  
	int result = 0;
	pthread_t thUpdateIbex;
	
	// Load config file and ibex info
	read_file_config_tumbdice("config_tumblingdice.dat");
	read_file_dtbase_ibexinfo("ibex.dat");
	
	initVentas();	//Iniciar lista de ventas
	
	// Redefine IRQ to new RSI
	signal(SIGINT,irq_handler);
	signal(SIGTERM,irq_handler);
	signal(SIGALRM,irq_handler);
	
	// Lanzamos thread de seleccion de plato a cocinar
	result = pthread_create(&thUpdateIbex, NULL, thread_update_ibex, "[Gekko] - INFO: Thread para hablar con Tumbling Dice iniciado" );
	if ( 0 != result ) { 
		print_message("[Gekko] - ERROR: No se ha podido crear el Thread de comunicacion con Tumbling Dice.\n");
		print_message("[Gekko] - ERROR: Cerrando el programa! \n");
		closeGlobals();
		exit(EXIT_FAILURE);
	}
	
	
	listen_dozers();
	print_message("[Gekko] - INFO: Cerrando listener \n");
	
	disconnect_tumbdice();
	exitGekko();
		
	return EXIT_SUCCESS;
}



/**********************************************
* @Nombre: 	eraseAccion
* @Def: 	Funcion que nos permite el borrado de las acciones puestas a la venta.
* @Arg:	 	 int socket, Frame trama
* @Ret:  	void
**********************************************/

void eraseAccion(int client_socket, Frame frameRX){

	int i = 0;
	int j = 0;
	int l = 0;
	int found = 0;
	int foundOperator = -1;
	int error = 0;
	
	char auxNomAccion[5];
	char auxCantidad[8];
	int auxNumAcciones;
	
	elemento auxAccion;
	char buffer[STRING_BUFFER_SIZE];
	auxNumAcciones = -1;
	auxAccion.nNumAcciones = 0;
	
	
	i=0;
	
	//Estructura de frameRX.data --> ABC-100
	while(frameRX.data[j] != '-'){
		auxNomAccion[j] = frameRX.data[j];
		j++;
	}
	auxNomAccion[j] = '\0';
	j++;
	while(frameRX.data[j] != '\0'){
		auxCantidad[l] = frameRX.data[j];
		l++;
		j++;
	}
	auxCantidad[l] = '\0';
	l = 0;
	auxNumAcciones = atoi(auxCantidad);
	//Encontrar el ibex que se quiere borrar  en el array de ibex
	while(found == 0 && i<35){
			if(strcmp(v[i].nomAccion,auxNomAccion) !=0){
				i++;	
			}else{
				found = 1;
			}
	}
	//SI el ibex existe
	if(found == 1){
		
			//Buscar acciones en venta
			LISTA_VENTA_VesInici(&(v[i].listaVentas));
			
			while(!LISTA_VENTA_Final(v[i].listaVentas)){
				auxAccion = LISTA_VENTA_Consulta(v[i].listaVentas);
				//Mirar que el operador que tiene acciones en venta SI seas tu mismo
				if(strcmp(auxAccion.sOperador, frameRX.source) == 0){		
					
					foundOperator = 1;
					//Si la cantidad a borrar es igual a la que se tiene en venta
					if(auxAccion.nNumAcciones == auxNumAcciones){
						LISTA_VENTA_Elimina(&(v[i].listaVentas));
					}else{
						//Si la cantidad a borrar es menor a la que se dispone en la lista --> actualizar
						if ( auxNumAcciones < auxAccion.nNumAcciones ){
							LISTA_VENTA_Elimina(&(v[i].listaVentas));
							auxAccion.nNumAcciones = auxAccion.nNumAcciones - auxNumAcciones;
							if ( auxAccion.nNumAcciones > 0 ) { 
								LISTA_VENTA_Insereix(&(v[i].listaVentas),auxAccion); 
							}
						}else{
							error = -1;
						}
					}
					break;
				//El operador del nodo no eres tu --> avanza
				}else{
					LISTA_VENTA_Avanza(&(v[i].listaVentas));
				}
				
			}
			bzero(frameRX.data, sizeof(frameRX.data));
			if(error == -1){
				sprintf(frameRX.data, "ERROR: No tienes suficientes acciones a la venta. Como maximo puedes borrar %d", auxAccion.nNumAcciones);
				frameRX.type = 'E';
			}else if ( foundOperator == -1 ) {
				sprintf(frameRX.data, "ERROR: No tienes acciones %s puestas a la venta \n",auxNomAccion);
				frameRX.type = 'E';
			}else {
				bzero(buffer,STRING_BUFFER_SIZE);
				sprintf(buffer,"[Gekko] - INFO: El operador %s ha retirado %d acciones de %s \n",frameRX.source,auxAccion.nNumAcciones,auxNomAccion);
				print_message(buffer);
				
                sprintf(frameRX.data, "%s-%d", auxNomAccion, auxNumAcciones);
				frameRX.type = 'D';	
			}
	//Si el ibex no existe
	}else{
		strcpy(frameRX.data, "ERROR: el nombre del IBEX es incorrecto. No existe.");
		frameRX.type = 'E';
	}
	
	strcpy(frameRX.source, "Gekko");
	for (i = 5; i < FRAME_SOURCE_SIZE; i++) {
		frameRX.source[i] = '\0';
	}
	write(client_socket, &frameRX, sizeof(frameRX));
}

/**********************************************
* @Nombre: 	warnDozer
* @Def: 	Funcion encargada de avisar a un cliente Dozer de que se ha realizado la compra, o bien añadirlo a la lista de pendientes.
* @Arg:	 	*arg: Recibe el socket del cliente
* @Ret:  	void
**********************************************/
void warnDozer(char sOperador[14], char auxNomAccion[5], int auxNumAcciones, float totalPagar){
    Frame frameTX;
    int found = 0;
    Dozer auxDozer;
	Dozer auxDozerInsertar;
    int fdDozer = 0;
    char auxText[FRAME_DATA_SIZE];
    
	//Recibe por parametros janires, BBVA , 100, 941.5
	// Comprobamos si el Dozer sigue conectado
	pthread_mutex_lock(&mtx_conlist);
    LISTA_DOZER_VesInici(&clients);
    while (!LISTA_DOZER_Final(clients)) {
        auxDozer = LISTA_DOZER_Consulta(clients);
        if (!strcmp(auxDozer.sOperador, sOperador)) {
            fdDozer = auxDozer.nFdSocket;
			found = 1;
			break;
        }
        LISTA_DOZER_Avanza(&clients);
    }
	pthread_mutex_unlock(&mtx_conlist);
	
	// Si esta conectado le avisamos
    if (found == 1) {
		strcpy(frameTX.source, "Gekko");
		frameTX.type = 'M';
		bzero(auxText, sizeof(auxText));
		sprintf(auxText, "%s-%d-%f", auxNomAccion, auxNumAcciones, totalPagar);
		strcpy(frameTX.data, auxText);
        write(fdDozer, &frameTX, sizeof(frameTX));
    }else{
		// Si no esta conectado, guardamos en la lista de pendientes
		pthread_mutex_lock(&mtx_pending);
		
		LISTA_DOZER_VesInici(&ventes_pending);
		while (!LISTA_DOZER_Final(ventes_pending)) {
			LISTA_DOZER_Avanza(&ventes_pending);
		}
		
		stpcpy(auxDozerInsertar.sOperador, sOperador);
		auxDozerInsertar.nFdSocket = -1;
		bzero(auxText, sizeof(auxText));
		sprintf(auxText, "%s-%d-%f", auxNomAccion, auxNumAcciones, totalPagar);
		strcpy(auxDozerInsertar.data_enviar, auxText);
		LISTA_DOZER_Insereix(&ventes_pending, auxDozerInsertar);
		pthread_mutex_unlock(&mtx_pending);
    }
    
}




/**********************************************
* @Nombre: 	buyAccion
* @Def: 	Funcion encargada de realizar la compra de las acciones.
* @Arg:	 	int socket, Frame trama
* @Ret:  	void
**********************************************/
void buyAccion(int client_socket, Frame frameRX){
	int found=0;
	int i=0;
	elemento auxAccion;
	int j=0;
	int l=0;
	int t =0;
	float totalPagar;
	float moneyDozer;
	
	//MIrar!!!
	char auxNomAccion[5];
	char auxCantidad[8];
	char auxDinero[10];
	int auxNumAcciones;
	int auxNumSum = 0;
	float auxCantidadDinero;
	
	strcpy( frameRX.source, "Gekko");
	for (i = 5; i < FRAME_SOURCE_SIZE; i++) {
		 frameRX.source[i] = '\0';
	}
	 frameRX.type = 'B';	
	i=0;
	//Estructura de  frameRX. data --> ABC-100[1200.00]
	while( frameRX. data[j] != '-'){
		auxNomAccion[j] =  frameRX. data[j];
		j++;
	}
	auxNomAccion[j] = '\0';
	j++;
	while( frameRX. data[j] != '['){
		auxCantidad[l] =  frameRX. data[j];
		l++;
		j++;
	}
	auxCantidad[l] = '\0';
	j++;
	l = 0;
	auxNumAcciones = atoi(auxCantidad);
	while( frameRX. data[j] != ']'){
		auxDinero[l] =  frameRX. data[j];
		l++;
		j++;
	}
	auxDinero[l] = '\0';
	auxCantidadDinero = atof(auxDinero);
		
	//Buscar el valor de la accion que quiero comprar en el array de ibex
	for(t = 0; t < 35; t++){
			if(strcmp(ibexList[t].code,auxNomAccion) ==0){
				found =1;
				break;
			}
	}
	//Si no lo encuentra muestra mensaje de error
	if(found == 0){
		sprintf( frameRX.data, "Error: Ese IBEX no existe.");
	}else{
		totalPagar = ibexList[t].value * auxNumAcciones;
		//Si el dinero que cuestan las acciones que quiero comprar es menor que el capital que tengo -->compra

		if(totalPagar <= auxCantidadDinero){
		
			found = 1;
			//Encontrar el ibex que quiere comprar en el array de ventas
			for(i = 0; i < 35; i++){
				if(strcmp(v[i].nomAccion,auxNomAccion) ==0){
					found = 0;
					break;
				}
			}

			
			
			
			//Si el ibex que se quiere comprar se encuentra en la lista de ibex a la venta 
			if(found == 0){
			
				//Buscar acciones en venta
				LISTA_VENTA_VesInici(&(v[i].listaVentas));
				
				while( !LISTA_VENTA_Final(v[i].listaVentas) ){
					auxAccion = LISTA_VENTA_Consulta(v[i].listaVentas);
					//Mirar que el operador que tiene acciones en venta no seas tu mismo
					if(strcmp(auxAccion.sOperador,  frameRX.source) != 0){		
						
						//si el primero nodo encontrado tiene menos o igual a la cantidad de acciones que quiero, elimino el nodo de la
						//lista y aviso a su dozer de que se lo he comprado
						if (auxAccion.nNumAcciones > auxNumAcciones ){
							warnDozer(auxAccion.sOperador, auxNomAccion, auxNumAcciones,totalPagar);	
							auxAccion.nNumAcciones = auxAccion.nNumAcciones - auxNumAcciones;
							//elimina nodo
							LISTA_VENTA_Elimina(&(v[i].listaVentas));
							//Volverlo a insertar actualiado
							LISTA_VENTA_Insereix(&(v[i].listaVentas),auxAccion);
							found = 2;
							break;
						//si no quedan más acciones en la lista de dozer, entonces hay que comprarle a gekko
						}else if(auxAccion.nNumAcciones == auxNumAcciones){
								
								LISTA_VENTA_Elimina(&(v[i].listaVentas));
								warnDozer(auxAccion.sOperador, auxNomAccion, auxAccion.nNumAcciones,totalPagar); 
								found = 2;
								break;				 	
						}else if (auxAccion.nNumAcciones < auxNumAcciones){
							LISTA_VENTA_Elimina(&(v[i].listaVentas));
							moneyDozer = auxAccion.nNumAcciones * ibexList[t].value;
							warnDozer(auxAccion.sOperador, auxNomAccion, auxAccion.nNumAcciones,moneyDozer);
							auxNumSum = auxNumSum + auxAccion.nNumAcciones;
							//Decrementar el numero de acciones para saber cuales quedan por comprar
							auxNumAcciones = auxNumAcciones - auxAccion.nNumAcciones;
						}
						//LISTA_VENTA_Avanza(&(v[i].listaVentas));	
					} else {
						LISTA_VENTA_Avanza(&(v[i].listaVentas));	
					}
					
				}
					//Si el ibex que se quiere comprar NO se encuentra en la lista de ibex a la venta O no hay sufiente cantidad en la
					//lista y hay que coger de los del gekko.
				
				if(found == 2){
					sprintf(frameRX.data, "%s-%d-%.2f", auxNomAccion,auxNumAcciones, totalPagar);

				}else {
					if(ibexList[t].quantity - auxNumAcciones > 0){
						ibexList[t].quantity = ibexList[t].quantity - auxNumAcciones;
						auxNumSum = auxNumSum + auxNumAcciones;
						sprintf(frameRX.data, "%s-%d-%.2f", auxNomAccion,auxNumSum, totalPagar);
					}else{
						frameRX.type = 'E';
						sprintf(frameRX.data, "ERROR: No hay suficientes acciones a la venta.");
					}
				
				}
			}else{
				frameRX.type = 'E';
				sprintf(frameRX.data, "ERROR: Ese IBEX no existe.");
			}
			
		
		}else{
			frameRX.type = 'E';
			sprintf(frameRX.data, "Error: No tienes suficiente dinero para comprar.");
		}	
	}
	write(client_socket, &frameRX, sizeof(frameRX));
}

/**********************************************
* @Nombre: 	sellAccion
* @Def: 	Funcion encarga de poner las acciones a la venta.
* @Arg:	 	int socket, Frame trama
* @Ret:  	void
**********************************************/
void sellAccion(int client_socket, Frame frameRX){
	int i;
	int j;
	int l;
	int k;
	int found;
	int ok;
	char buffer[STRING_BUFFER_SIZE];
	char auxNomAccion[5];
	char auxCantidad[8];
	int auxNumAcciones;
	elemento auxAccion;
	elemento prueba;
	
	//Inicializar variables
	i = 0;
	j = 0;
	k = 0;
	l = 0;
	found = 0;
	bzero(auxNomAccion,5);
	bzero(auxCantidad,8);
	
	while(frameRX.data[j] != '-'){
		auxNomAccion[j] = frameRX.data[j];
		j++;
	}
	auxNomAccion[j] = '\0';
	j++;
	while(frameRX.data[j] != '\0'){
		auxCantidad[l] = frameRX.data[j];
		l++;
		j++;
	}
	auxCantidad[l] = '\0';
	l = 0;
	auxNumAcciones = atoi(auxCantidad);
	
	//Buscar Ibex en la lista de ibex
	for(i = 0; i < 35; i++){
			if(strcmp(ibexList[i].code,auxNomAccion) ==0){
				found = 1;
				break;
			}
	}
	
	if(found == 1){
		strcpy(auxAccion.sOperador, frameRX.source);
		auxAccion.nNumAcciones = auxNumAcciones;
	
		ok = LISTA_VENTA_BuscaOperador(&(v[i].listaVentas),frameRX.source);
		
		if(ok == 0){
			while(!LISTA_VENTA_Final(v[i].listaVentas)){
				LISTA_VENTA_Avanza(&(v[i].listaVentas));
			}
			LISTA_VENTA_Insereix(&(v[i].listaVentas),auxAccion);
		}else{
			prueba = LISTA_VENTA_Consulta(v[i].listaVentas);
			prueba.nNumAcciones = prueba.nNumAcciones + auxNumAcciones;
			LISTA_VENTA_ActualizaOperador(&(v[i].listaVentas),prueba);
		}
		
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - INFO: El operador %s pone a la venta %d acciones de %s \n",frameRX.source, auxNumAcciones, auxNomAccion);
		print_message(buffer);

		sprintf(frameRX.data, "%s-%d", auxNomAccion, auxNumAcciones);
		
		for (k = (int)strlen(frameRX.data); k < FRAME_DATA_SIZE; k++) {
            frameRX.data[k] = '\0';
        }
		frameRX.type = 'S';	
	}else{
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - ERROR: El operador %s queria poner a la venta %d acciones de %s \n",frameRX.source, auxNumAcciones, auxNomAccion);
		print_message(buffer);
		sprintf(frameRX.data, "ERROR: Ese IBEX no existe.");
		frameRX.type = 'E';	
	}
	
	//Preparar trama para enviar
	strcpy(frameRX.source, "Gekko");
	for (i = 5; i < FRAME_SOURCE_SIZE; i++) {
		frameRX.source[i] = '\0';
	}
	
	write(client_socket, &frameRX, sizeof(frameRX));
}

/**********************************************
* @Nombre: 	showIbex
* @Def: 	Envia al usuario la informacion de los Ibex
* @Arg:	 	void
* @Ret:  	Out: int fd_socket_tumbdice: Devuelve el file descriptor del socket
**********************************************/
void showIbex(int client_socket){
	Frame frameTX;
	int j;
	char dataAux[FRAME_DATA_SIZE];
	long sumatorio = 0;
	elemento auxAccion;
	// START Readers-writers
	pthread_mutex_lock(&mtx_readers);
	readers++;
	if ( readers == 1) {
		pthread_mutex_lock(&mtx_writers);
	}
	pthread_mutex_unlock(&mtx_readers);
	// Leemos los IBEX
	for ( j = 0; j <35 ;j++){
		sumatorio = 0;
		pthread_mutex_lock(&mtx_ventes_readers);
		ventes_readers++;
		if (readers == 1) {
			pthread_mutex_lock(&mtx_ventes_writers);
		}
		pthread_mutex_unlock(&mtx_ventes_readers);
		
		LISTA_VENTA_VesInici(&(v[j].listaVentas));
		while(!LISTA_VENTA_Final(v[j].listaVentas)){
			auxAccion = LISTA_VENTA_Consulta(v[j].listaVentas);
			sumatorio = sumatorio + auxAccion.nNumAcciones;
			LISTA_VENTA_Avanza(&(v[j].listaVentas));
		}
		
		pthread_mutex_lock(&mtx_ventes_readers);
		ventes_readers--;
		if ( ventes_readers == 0 ) {
			pthread_mutex_unlock(&mtx_ventes_writers);
		}
		pthread_mutex_unlock(&mtx_ventes_readers);
		bzero(dataAux, sizeof(dataAux));
		sprintf(dataAux, "%s\t%.2f\t%lld", ibexList[j].code, ibexList[j].value, ibexList[j].quantity + sumatorio);
		
		frameTX = prepare_Frame("Gekko", 'X', dataAux);
		write(client_socket,&frameTX, sizeof(frameTX));
		
		
	}
	// END Readers-writers
	pthread_mutex_lock(&mtx_readers);
	readers--;
	if (readers == 0) {
		pthread_mutex_unlock(&mtx_writers);
	}
	pthread_mutex_unlock(&mtx_readers);

}



/* SOCKET FUNCTIONS */
/**********************************************
* @Nombre: 	connect_tumbdice
* @Def: 	Crea el socket, configura la conexion y realiza la conexion al servidor Tumbling Dice
* @Arg:	 	void
* @Ret:  	Out: int fd_socket_tumbdice: Devuelve el file descriptor del socket
**********************************************/
int connect_tumbdice() {
	char buffer[STRING_BUFFER_SIZE];
	struct sockaddr_in server;
	
	// Creating socket
	fd_socket_tumbdice = socket(AF_INET, SOCK_STREAM, 0);
	if ( fd_socket_tumbdice < 0 ) {
		print_message("[Gekko] - ERROR: No se ha podido crear el socket de conexion\n");
		return -1;
	}
	
	bzero(&server,sizeof(&server));
	server.sin_addr.s_addr 	= inet_addr(config_tumbdice.host_ip);
	server.sin_family 		= AF_INET;
	server.sin_port			= htons(config_tumbdice.host_port);
	
	int connectionResult = connect(fd_socket_tumbdice , (struct sockaddr *)&server , sizeof(server));
	if (  connectionResult < 0 ) {
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - ERROR: No se ha podido conectar con %s:%d \n",config_tumbdice.host_ip,config_tumbdice.host_port);
		close(fd_socket_tumbdice); error = 1;
		print_message(buffer);
		return -1;
	}
	
	// Prepare Frame and Send
	Frame frameTX = new_Frame_custom(FRAME_GEKKOTUMB_CONNEXIO);
	write(fd_socket_tumbdice, &frameTX, sizeof(frameTX));
	
	// Read answer Frame
	Frame frameRX = new_Frame();
	read(fd_socket_tumbdice, &frameRX, sizeof(frameRX));
	
	if ( frameRX.type != 'O' ) {
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - ERROR: Tumbling Dice ha respondido con trama de [%c - %s] con el host %s:%d \n",frameRX.type,frameRX.data,config_tumbdice.host_ip,config_tumbdice.host_port);
		print_message(buffer);
		return -1;
	}
	
	bzero(buffer,STRING_BUFFER_SIZE);
	sprintf(buffer,"[Gekko] - INFO: Conexion realizada correctamente con el host %s:%d \n",config_tumbdice.host_ip,config_tumbdice.host_port);
	print_message(buffer);
	
	error = 0;
	return fd_socket_tumbdice;
}

/**********************************************
* @Nombre: 	updateIbexList
* @Def: 	Crea la trama de peticion de datos para Tumbling Dice y la envia. Recupera el listado que TD le responde.
* @Arg:	 	void
* @Ret:  	Out: int: Nos permite saber si la actualizacion se ha realizado correctamente.
**********************************************/
int updateIbexList(void) {
	int readed_socket_bytes = 0;
	char buffer[STRING_BUFFER_SIZE];
	
	Frame frameTX = new_Frame_custom(FRAME_GEKKOTUMB_PETICIO);
	readed_socket_bytes = write(fd_socket_tumbdice, &frameTX, sizeof(frameTX));
	
	if ( error == 1 ) {
		
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - INFO: Reconnect attempt %d/%d \n",(current_reconnect_attempt+1),TUMBIDCE_RECONNECT_ATTEMPTS);
		print_message(buffer);
		
		if ( connect_tumbdice() < 0 ) {
			if ( (current_reconnect_attempt + 1) >= TUMBIDCE_RECONNECT_ATTEMPTS ) {
				bzero(buffer,STRING_BUFFER_SIZE);
				sprintf(buffer,"[Gekko] - ERROR: Despues de %d/%d intentos se cierra Gekko server.\n",(current_reconnect_attempt+1),TUMBIDCE_RECONNECT_ATTEMPTS);
				print_message(buffer);
				raise(SIGINT);
				exit(EXIT_FAILURE);
			}
			current_reconnect_attempt++;
		}
		return UPDATE_IBEX_ERROR;
	}
	
	// Reset counter
	current_reconnect_attempt = 0;
	Frame frameRX;
	
	unsigned int i = 0;
	for ( i=0; i<IBEX_LIST_SIZE; i++) {
		// Getting the response frame
		frameRX = new_Frame();
		readed_socket_bytes = read(fd_socket_tumbdice, &frameRX, sizeof(frameRX));
		
		if ( readed_socket_bytes < 1 ) {
			print_message("[Gekko] - ERROR: No se ha recibido respuesta o bien el socket se ha caido.\n");
			close(fd_socket_tumbdice); error = 1;
			return UPDATE_IBEX_ERROR;
		} else if ( frameRX.type == 'E' ) {
			print_message("[Gekko] - ERROR: Tumbling Dice ha respondido con trama de error.\n");
			return UPDATE_IBEX_ERROR;
		} else {
			if ( updateIbex(frameRX) == UPDATE_IBEX_ERROR ) { 
				print_message("[Gekko] - ERROR: Fallo al actualizar el listado de Ibex\n");
				return UPDATE_IBEX_ERROR; 
			}
		}
	}
	print_message("[Gekko] - INFO: Actualizacion de Ibex correcta\n");
	return UPDATE_IBEX_OK;
}
/**********************************************
* @Nombre: 	updateIbex
* @Def: 	Funcion encargada de recibir los datos de un único Ibex de los que ha enviado TD, encontrarlo en el array y actualizarlo.
* @Arg:	 	void
* @Ret:  	Out: int : Nos permite saber si la actualizacion se ha realizado correctamente.
**********************************************/
int updateIbex(Frame frameRX) {
	int i = 0; int j = 0;
	int found = FALSE;
	char operator = ' ';
	float value = 0;
	char aux_ibex_code[IBEX_CODE_SIZE];
	char aux_value[FRAME_DATA_SIZE];
	
	
	// Clean auxiliar string
	bzero(aux_ibex_code,IBEX_CODE_SIZE);
	bzero(aux_value,IBEX_CODE_SIZE);

	while ( frameRX.data[i] != '-' && frameRX.data[i] != '+'  ) {
		aux_ibex_code[i] = frameRX.data[i];
		i++;
	}
	operator = frameRX.data[i++];
	while (frameRX.data[i] != '\0' ) {
		aux_value[j] = frameRX.data[i];
		j++; i++;
	}
	
	for ( i=0; i<IBEX_LIST_SIZE; i++) {
		if ( strcmp(aux_ibex_code,ibexList[i].code) == 0 ) {
			found = TRUE;
			break;
		}
	}
	
	if ( found == TRUE ) {

		value = atof(aux_value);
		if ( operator == '+' ) {
			ibexList[i].value = ibexList[i].value + value;
		} else if (operator == '-') {
			ibexList[i].value = ibexList[i].value - value;
			if ( ibexList[i].value < 0 ) {
				ibexList[i].value = 0;
			}
		}
		return UPDATE_IBEX_OK;
	}
	
	return UPDATE_IBEX_ERROR;
}
/**********************************************
* @Nombre: 	disconnect_tumbdice
* @Def: 	Funcion encargada de desconectar el programa del servidor TD.
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void disconnect_tumbdice() {
	Frame frameTX = new_Frame_custom(FRAME_GEKKOTUMB_DESCONNEXIO);
	write(fd_socket_tumbdice,&frameTX, sizeof(frameTX));
	close(fd_socket_tumbdice);
}

/* PRINT FUNCTIONS */
/**********************************************
* @Nombre: 	print_message
* @Def: 	Debido a que no esta permitido el uso de printf, hemos de crear nuestra funcion "printf"
* @Arg:	 	In: char[] message: Recibe una String 
* @Ret:  	void
**********************************************/
void print_message(char *message) {
	write(FD_SCREEN, message, strlen(message));
}

/* READ FILE FUNCTIONS */
/**********************************************
* @Nombre: 	read_file_config_tumbdice
* @Def: 	Funcion encargada de leer el fichero con los datos ip, puerto e intervalos de actualizacion de TD.
* @Arg:	 	In: char[] file: Recibe el nombre del fichero donde recoger la información de la configuración.
* @Ret:  	void
**********************************************/
void read_file_config_tumbdice(char file[]) {
	char c;
    int nOK;
    int i;
    int j;
	int fd;
    char tmp[10];
	config_tumbdice = new_Config();
	
	fd = open(file, O_RDONLY);
	if(fd == -1){
		print_message("\nError cargando el fichero de configuracion\n");
		exit(EXIT_FAILURE);
	}else{	   
		nOK = read(fd,&c,1);
		while(nOK!=0){
			j=0;
			while(c!='\n'){
				tmp[j]=c;
				j++;
				nOK = read(fd,&c,1);
			}
			tmp[j] = '\0';
			config_tumbdice.update_interval = atoi(tmp);
			nOK = read(fd,&c,1);
			i=0;
			while(c!='\n'){
				config_tumbdice.host_ip[i]=c;
				i++;
				nOK = read(fd,&c,1);
			}
			config_tumbdice.host_ip[i]='\0'; 	    
			j=0;
			nOK = read(fd,&c,1);
			while(c!='\n'){
				tmp[j]=c;
				j++;
				nOK = read(fd,&c,1);
			}
			tmp[j] = '\0';
			
			config_tumbdice.host_port = atoi(tmp);
			if(c =='\n') break;
		}	
			
		close(fd);
	}
}
/**********************************************
* @Nombre: 	read_file_dtbase_ibexinfo
* @Def: 	Funcion encargada de leer el fichero con los datos de los Ibex. Sobre estos datos recibiremos actualizaciones
* @Arg:	 	In: char[] file: Recibe el nombre del fichero donde recoger la información de los Ibex
* @Ret:  	void
**********************************************/
void read_file_dtbase_ibexinfo(char file[]) {
   	char c;
    int nOK;
    int i;
	int s;
	int k;
    int j;
    char tmp[255];
	int fd;

    
       
	fd = open(file, O_RDONLY);
	if(fd == -1){
		char frase[] = "Errors\n";
		write(1,frase, strlen(frase));
		exit(0);
	}else{	   
    nOK = read(fd,&c,1);
    for(s = 0; s < IBEX_LIST_SIZE; s++){
		i=0;
		while(c!='\t' ){    
			ibexList[s].code[i]=c;
			i++;
            nOK = read(fd,&c,1);
        }
        ibexList[s].code[i]='\0';
		j=0;
        nOK = read(fd,&c,1);
        while(c!='\t'){
            tmp[j]=c;
            j++;
            nOK = read(fd,&c,1);
        }
		tmp[j] = '\0';
		ibexList[s].value = atof(tmp);
		k=0;
		nOK = read(fd,&c,1);
		while(c!='\n'){
            tmp[k]=c;
            k++;
            nOK = read(fd,&c,1);
        }
		tmp[k] = '\0';
		ibexList[s].quantity = atoll(tmp);
		nOK = read(fd,&c,1);
	}
   	}	
		
  	close(fd);
}


/* INTERRUPTIONS FUNCTIONS */
/**********************************************
* @Nombre: 	irq_handler
* @Def: 	Funcion encargada de leer el fichero con los datos ip, puerto e intervalos de actualizacion de TD.
* @Arg:	 	In: int irq_signal: Identificador de la signal que se ha provocado
* @Ret:  	void
**********************************************/
void irq_handler(int irq_signal) {
	int result = 0;
	if ( irq_signal == SIGINT || irq_signal == SIGTERM ) {
		print_message("\n\n");
		exitGekko();
	} else if (irq_signal == SIGALRM) {
		// Cambiar estado del mutex
		result = pthread_mutex_unlock(&mtx_alarm);
		if ( result != 0 ) {
			print_message("[Gekko] - ERROR: No se pudo desbloquear el mutex de actualizar Ibex.\n");
		}
	}
}

/**********************************************
* @Nombre: 	cleanIbex
* @Def: 	Funcion encargada de limpiar el listado de Ibex cargados en memoria.
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void	cleanIbex	(void) {
	int i = 0;
	for ( i=0; i<IBEX_LIST_SIZE; i++) {
		ibexList[i] = new_Ibex();
	}
}
/**********************************************
* @Nombre: 	initGlobals
* @Def: 	Funcion encargada de iniciar las variables globales a valores
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void	initGlobals	(void) {
	error = 1;
	readers = 0; ventes_readers = 0;
	fd_socket_tumbdice = -1;
	current_reconnect_attempt = 0;
	cleanIbex();
	LISTA_DOZER_Crea(&clients);
	LISTA_DOZER_Crea(&ventes_pending);
}
/**********************************************
* @Nombre: 	initVentas
* @Def: 	Funcion encargada de iniciar las variables globales a valores
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void	initVentas	(void) {
	int i;
	//Inicializar lista de ventas
	for (i = 0;i <35; i++){
		strcpy(v[i].nomAccion, ibexList[i].code);
		LISTA_VENTA_Crea(&(v[i].listaVentas));
	}
}
/**********************************************
* @Nombre: 	closeGlobals
* @Def: 	Funcion encargada de cerrar file descriptors, eliminar mutex, semaforos
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void 	closeGlobals() {
	pthread_mutex_destroy(&mtx_alarm);
	pthread_mutex_destroy(&mtx_readers);
	pthread_mutex_destroy(&mtx_writers);
	pthread_mutex_destroy(&mtx_conlist);
	pthread_mutex_destroy(&mtx_ventes_readers);
	pthread_mutex_destroy(&mtx_ventes_writers);
	pthread_mutex_destroy(&mtx_pending);
}

/**********************************************
* @Nombre: 	exitGekko
* @Def: 	Funcion encargada de cerrar el programa correctamente.
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void 	exitGekko	(void) {
	disconnect_tumbdice();
	disconnect_dozers();
	LISTA_DOZER_Destrueix(&clients);
	LISTA_DOZER_Destrueix(&ventes_pending);
	closeGlobals();
	closeVentas();
	print_message("[Gekko] - INFO: Cerrando el programa \n");
	exit(EXIT_SUCCESS);
}
/**********************************************
* @Nombre: 	closeVentas
* @Def: 	Funcion encargada liberar la lista dinamica de acciones puestas a la venta
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void closeVentas() {
	int i;
	for (i = 0;i <35; i++){
		strcpy(v[i].nomAccion, ibexList[i].code);
		LISTA_VENTA_Destrueix(&(v[i].listaVentas));
	}
}
/**********************************************
* @Nombre: 	disconnect_dozers
* @Def: 	Funcion encargada de desconectar a todos los clientes Dozer del programa cuando Gekko cierra.
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void 	disconnect_dozers() {
	Dozer auxDozer;
	Frame frameTX;
	char buffer[STRING_BUFFER_SIZE];
	
	LISTA_DOZER_VesInici(&clients);
	while (!LISTA_DOZER_Final(clients) ) {
		auxDozer = LISTA_DOZER_Consulta(clients);
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Gekko] - Enviando mensaje de desconexion al cliente: %s \n",auxDozer.sOperador);
		print_message(buffer);
		frameTX = new_Frame_custom(FRAME_GEKKODOZER_DESCONNEXIO);
		strcpy(frameTX.source,"Gekko");
		write(auxDozer.nFdSocket,&frameTX,sizeof(frameTX));
		LISTA_DOZER_Avanza(&clients);
	}
}
/**********************************************
* @Nombre: 	listen_dozers
* @Def: 	Funcion encarga de iniciar el servidor de escucha de dozers
* @Arg:	 	void
* @Ret:  	void
**********************************************/
void	listen_dozers(void) {
	
	int server_socket = 0;
	int client_socket = 0;
	int client_length = sizeof(client_socket);
	int result = 0;
	struct sockaddr_in server, client;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if ( server_socket < 0 ) {
		print_message("[Gekko] - ERROR: Problemas al iniciar el servidor para Dozers.\n");
		raise(SIGINT);
	}
	
	// Prepare server info
	bzero((char *) &server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(SERVER_PORT);
	memset(server.sin_zero,0,8);
	result = bind(server_socket,(struct sockaddr *) &server, sizeof(server));
	if (  result < 0 ) {
		print_message("[Gekko] - ERROR: Problemas con el binding. Revisa que no haya otro programa usando el puerto\n");
		raise(SIGINT);
	}
	result = listen(server_socket,MAX_DOZERS);
	if ( result < 0 ) {
		print_message("[Gekko] - ERROR: Problemas al realizar el listen\n");
		raise(SIGINT);
	}
	print_message("[Gekko] - INFO: Servidor de Dozers - started\n");
	while( 1 == 1 ) {
		print_message("[Gekko] - INFO: Listo para conectar con un nuevo Dozer\n");
		client_socket = accept(server_socket, (struct sockaddr *) &client, &client_length);
		if (client_socket < 0) {
			print_message("[Gekko] - ERROR: Problemas estableciendo conexion con cliente Dozer\n");
			close(server_socket); 
			close(client_socket);
		} else {
			run_dozer_thread(client_socket);
		}
	}
	
}
/**********************************************
* @Nombre: 	run_dozer_thread
* @Def: 	Funcion encargada lanzar el Thread del Dozer
* @Arg:	 	int socket_cliente
* @Ret:  	void
**********************************************/
void run_dozer_thread(int client_socket) {
	pthread_t thread_dozer_id;
	pthread_create(&thread_dozer_id, NULL, thread_manage_dozer, (void *)client_socket);
}


