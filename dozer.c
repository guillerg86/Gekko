/****************************************************************
*	@File:		dozer.c
*	@Author:	Itziar Sanchez 		LS26023
*	@Author: 	Guille Rodríguez 	LS26151
*	@Date:		2014/2015
****************************************************************/

#include "Frame.h"
#include "Config.h"
#include "dozer.h"


/* GLOBALS */

Operator op;
Config config_gekko;
int fd_socket_gekko;
int boolContinue;
int gekkoContinue;

static pthread_mutex_t mtx_shell 	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_stockdata= PTHREAD_MUTEX_INITIALIZER;	

/*********************************************************************************************************
 *
 *   @Nombre: buyAccion
 *   @Def: Función que se encarga de actualizar el saldo y la lista de acciones que posee el cliente. 
 *		   Cuando compra acciones debe restar de su saldo el coste de la compra de dichas acciones y añadir
 *		   a su lista de acciones el nombre del Ibex y la cantidad de la operacion efectuada.
 *   @Arg: frameRX: Recibe la trama que le envia el gekko con los datos de la compra.
 *   @Ret: void
 *
 *********************************************************************************************************/
void buyAccion(Frame frameRX){
    char *sIbex,*sAuxNumAcciones,*sAuxCosteAcciones;
    char sAuxText[100];
    int i = 0;
	int j = 0;	
    float fCantidadCoste;
    elemento auxElemento;
	int nCantidadAcciones;
	int found = 0;
		
        //Recibimos ABC-100-1200.00
        sIbex = (char*)malloc(sizeof(char));
        while (frameRX.data[i] != '-') {
            sIbex[i] = frameRX.data[i];
            i++;
            sIbex = (char*)realloc(sIbex, sizeof(char) * (i+1));
        }
        sIbex[i] = '\0';
        i++;
		j= 0;
		sAuxNumAcciones = (char*)malloc(sizeof(char));
        while (frameRX.data[i] != '-') {
            sAuxNumAcciones[j] = frameRX.data[i];
            i++;
            j++;
            sAuxNumAcciones = (char*)realloc(sAuxNumAcciones, sizeof(char) * (j+1));
        }
        sAuxNumAcciones[j] = '\0';
        i++; 
		j= 0;
		nCantidadAcciones = atoi(sAuxNumAcciones);
		sAuxCosteAcciones = (char*)malloc(sizeof(char));
		while (frameRX.data[i] != '\0') {
            sAuxCosteAcciones[j] = frameRX.data[i];
            i++;
            j++;
            sAuxCosteAcciones = (char*)realloc(sAuxCosteAcciones, sizeof(char) * (j+1));
        }
        sAuxCosteAcciones[j] = '\0';
		fCantidadCoste = atof(sAuxCosteAcciones);
		
		//Busca el la lista del cliente a ver si tenia acciones de las que quiere comprar
        LISTA_VesInici(&op.acciones);
        while (!LISTA_Final(op.acciones)) {
            auxElemento = LISTA_Consulta(op.acciones);
            if(strcmp(auxElemento.ibex,sIbex) == 0){
				found = 1;
                break;
            }else{
				LISTA_Avanza(&op.acciones);
			}
        }
		if(found == 0){
			strcpy(auxElemento.ibex,sIbex);
			auxElemento.num = nCantidadAcciones;
		}else{
			auxElemento.num = auxElemento.num + nCantidadAcciones;
			LISTA_Elimina(&op.acciones);
		}
		LISTA_Insereix(&op.acciones, auxElemento);
		
		//Resta lo que le ha costado la venta al saldo		
		op.money = op.money - fCantidadCoste;
		
		bzero(sAuxText, sizeof(sAuxText));
        sprintf(sAuxText, "\nCompra realizada. Coste: %.2f €\n", fCantidadCoste);
        print_message(sAuxText);
		
		free(sIbex);
		free(sAuxNumAcciones);
		free(sAuxCosteAcciones);
    
}
/*********************************************************************************************************
 *
 *   @Nombre: sellAccion
 *   @Def: Función que se encarga de actualizar la lista de acciones que posee el cliente. 
 *		   Cuando vende acciones debe borrar o actualizar las cantidades de acciones que posee el cliente 
 *		   en su lista.
 *   @Arg: frameRX: Recibe la trama que le envia el gekko con los datos de la compra.
 *   @Ret: void
 *
 *********************************************************************************************************/
void sellAccion(Frame frameRX){
	char *sIbex, *sAuxNumAcciones;
	int i;
	int j;
	int nCantidadAcciones;
	elemento auxElemento;
		
	//Inicializar variables
	i = 0;
	j = 0;
	nCantidadAcciones = -1;
		//Recibe ABC-100
		//Lee el nombre del Ibex
		sIbex = (char*)malloc(sizeof(char));
		while (frameRX.data[i] != '-') {
			sIbex[i] = frameRX.data[i];
			i++;
			sIbex = (char*)realloc(sIbex, sizeof(char) * (i+1));
		}
		sIbex[i] = '\0';
		i++;
		
		//Lee el numero de acciones que quiere vender
		
		sAuxNumAcciones = (char*)malloc(sizeof(char));
		while (frameRX.data[i] != '\0') {
			sAuxNumAcciones[j] = frameRX.data[i];
			i++;
			j++;
			sAuxNumAcciones = (char*)realloc(sAuxNumAcciones, sizeof(char) * (j+1));
		}
		sAuxNumAcciones[j] = '\0';
		nCantidadAcciones = atoi(sAuxNumAcciones);
		
		//Busca el ibex que quiere vender, en la lista de los ibex que dispone
		
		LISTA_VesInici(&op.acciones);
		while (!LISTA_Final(op.acciones)) {
			auxElemento = LISTA_Consulta(op.acciones);
			if(strcmp(auxElemento.ibex,sIbex) == 0){
				auxElemento.num = auxElemento.num - nCantidadAcciones;
				LISTA_Elimina(&op.acciones);
				
				if(auxElemento.num != 0) {
					LISTA_Insereix(&op.acciones,auxElemento);
				}
				break;
			}
			LISTA_Avanza(&op.acciones);
		}
		free(sIbex);
		free(sAuxNumAcciones);

		print_message("\n[Dozer] - INFO: OK. Accions posades a la venda.\n");

}
/*********************************************************************************************************
 *
 *   @Nombre: warnDozer
 *   @Def: Función que se encarga de actualizar el saldo del cliente al que pertenecen las acciones compradas y 
 *		   de avisar al dueño de las acciones cuantas de las acciones que ha puesto a la venta han sido compradas.
 *   @Arg: frameRX: Recibe la trama que le envia el gekko con los datos de la compra para poder avisar al dueño
 *		   de dichas acciones.
 *   @Ret: void
 *
 *********************************************************************************************************/
void warnDozer(Frame frameRX){

    char *sIbex,*sAuxNumAcciones,*sAuxCosteAcciones;
    char sAuxText[100];
    int i = 0;
	int j = 0;	
    float fCantidadCoste;
	int nCantidadAcciones;
	
	//Recibimos ABC-100-1200.00
    sIbex = (char*)malloc(sizeof(char));
    while (frameRX.data[i] != '-') {
        sIbex[i] = frameRX.data[i];
        i++;
        sIbex = (char*)realloc(sIbex, sizeof(char) * (i+1));
    }
    sIbex[i] = '\0';
    i++;
	j = 0;
	sAuxNumAcciones = (char*)malloc(sizeof(char));
    while (frameRX.data[i] != '-') {
        sAuxNumAcciones[j] = frameRX.data[i];
        i++;
        j++;
        sAuxNumAcciones = (char*)realloc(sAuxNumAcciones, sizeof(char) * (j+1));
    }
    sAuxNumAcciones[j] = '\0';
    i++; 
	j=0;
	nCantidadAcciones = atoi(sAuxNumAcciones);
	sAuxCosteAcciones = (char*)malloc(sizeof(char));
	while (frameRX.data[i] != '-') {
        sAuxCosteAcciones[j] = frameRX.data[i];
        i++;
        j++;
        sAuxCosteAcciones = (char*)realloc(sAuxCosteAcciones, sizeof(char) * (j+1));
    }
    sAuxCosteAcciones[j] = '\0';
	fCantidadCoste = atof(sAuxCosteAcciones);
	
	op.money = op.money + fCantidadCoste;
	
	sprintf(sAuxText, "\n[GEKKO]: Venta realizada. %s %d acciones. %.2f€.\n%s>",sIbex,nCantidadAcciones, fCantidadCoste, op.name);
	print_message(sAuxText);
	free(sIbex);
	free(sAuxNumAcciones);
	free(sAuxCosteAcciones);	
}
/*********************************************************************************************************
 *
 *   @Nombre: eraseAccion
 *   @Def: Función que se encarga de actualizar la lista de acciones que posee el cliente y volver a añadir 
 *		   la cantidad de acciones que había puesto a la venta. 
 *   @Arg: frameRX: Recibe la trama que le envia el gekko con los datos de las acciones que había puesto 
 *		   a la venta.
 *   @Ret: void
 *
 *********************************************************************************************************/
void eraseAccion(Frame frameRX) {
	char buffer[STRING_BUFFER_SIZE];
	char *sIbex, *sAuxNumAcciones;
	int i;
	int j;
	int nCantidadAcciones;
	elemento auxElemento;
	elemento newIbex;
	int found = -1;
	int size = strlen(frameRX.data);
	//Inicializar variables
	i = 0;
	j = 0;
	nCantidadAcciones = -1;
	
	sIbex = (char*)malloc(sizeof(char));
	while (frameRX.data[i] != '-' && i < size ) {
		sIbex[i] = frameRX.data[i];
		i++;
		sIbex = (char*)realloc(sIbex, sizeof(char) * (i+1));
	}
	sIbex[i] = '\0';
	i++;
	
	//Lee el numero de acciones que ha retirado	
	sAuxNumAcciones = (char*)malloc(sizeof(char));
	while (frameRX.data[i] != '\0' && i < size ) {
		sAuxNumAcciones[j] = frameRX.data[i];
		i++;
		j++;
		sAuxNumAcciones = (char*)realloc(sAuxNumAcciones, sizeof(char) * (j+1));
	}
	sAuxNumAcciones[j] = '\0';
	nCantidadAcciones = atoi(sAuxNumAcciones);
	
	
	// Separados los datos, procedemos a modificar la cantidad de las acciones!
	LISTA_VesInici(&op.acciones);
	while (!LISTA_Final(op.acciones)) {
		
		auxElemento = LISTA_Consulta(op.acciones);
		if(strcmp(auxElemento.ibex,sIbex) == 0){
			found = 1;
			// Actualizamos las acciones
			auxElemento.num = auxElemento.num + nCantidadAcciones;
			LISTA_Elimina(&op.acciones);
			LISTA_Insereix(&op.acciones,auxElemento);
			break;
		}
		LISTA_Avanza(&op.acciones);
	}
	
	if ( found == -1 ) {
		bzero(newIbex.ibex,5);
		strcpy(newIbex.ibex,sIbex);
		newIbex.num = nCantidadAcciones;
		LISTA_Insereix(&op.acciones,newIbex);
	}
	
	bzero(buffer,STRING_BUFFER_SIZE);
	sprintf(buffer,"\nRetiradas %d acciones de %s \n",nCantidadAcciones, sIbex);
	print_message(buffer);
	
	free(sIbex);
	free(sAuxNumAcciones);
}
/*********************************************************************************************************
 *
 *   @Nombre: thread_listen_gekko
 *   @Def: Funcion encargada de gestionar lo que recibe del Gekko.
 *   @Arg: *arg: Recibe un 0
 *   @Ret: void
 *
 *********************************************************************************************************/
void *thread_listen_gekko(void *arg) {
	int readed_bytes; 
	readed_bytes = (int)arg;	// Para quitar el warning del ARG sense us
	readed_bytes = 0;
	Frame frameRX = new_Frame();
	char buffer[STRING_BUFFER_SIZE];
	
	while ( frameRX.type != 'Q' ) {
		// Escuchamos lo que tiene que decir el Gekko
		frameRX = new_Frame();
		readed_bytes = read(fd_socket_gekko, &frameRX, sizeof(frameRX));

		if (readed_bytes < 0 ) {
			// Si leemos 0 bytes, el socket ha caido, cerramos socket y thread
			print_message("[Dozer] - ERROR: La conexion con Gekko se ha cerrado.\n");
			raise(SIGINT);
		}else{
			switch (frameRX.type) {
				case 'Q':
					print_message("\n[Dozer] - INFO: El servidor Gekko ha enviado señal de que cierra. Este terminal se cerrara\n");
					raise(SIGINT);
					break;
				case 'X':
					print_ibex_list(frameRX);
					pthread_mutex_unlock(&mtx_shell);
					break;
				case 'B':
					//BUY
					buyAccion(frameRX);
					pthread_mutex_unlock(&mtx_shell);
					break;
				case 'S':
					//SELL
					sellAccion(frameRX);
					pthread_mutex_unlock(&mtx_shell);
					break;
				case 'D':
					pthread_mutex_lock(&mtx_stockdata);
					eraseAccion(frameRX);
					pthread_mutex_unlock(&mtx_stockdata);
					pthread_mutex_unlock(&mtx_shell);
					break;
				case 'M':
					warnDozer(frameRX);
				break;
				case 'E':
					bzero(buffer,STRING_BUFFER_SIZE);
					sprintf(buffer,"\n[Dozer] - %s \n",frameRX.data);
					print_message(buffer);
					pthread_mutex_unlock(&mtx_shell);
				break;
				default:
					pthread_mutex_unlock(&mtx_shell);
					break;
			}
		}
	}
	pthread_exit(NULL);
}
/******************************************************************************************************
*
* @Nombre: 	main
* @Def: 	Funcion principal del programa. Lanza el thread_listen_gekko y cierra el Dozer.
* @Arg:	 	void
* @Ret:  	Out:	Retorna un Integer para indicar al operativo el resultado de la ejecucion.
*
*******************************************************************************************************/
int main(void) {
	
	char buffer[STRING_BUFFER_SIZE];
	
	initGlobals();
	pthread_t thread_gekko_id;
	
	// Load data files
	read_file_config_gekko("config.dat");
	read_file_dtbase_stock("stock.dat");
	
	// Redefine IRQ to new RSI
	signal(SIGINT,irq_handler);
	signal(SIGTERM,irq_handler);
	
	
	if ( connect_gekko() == TRUE ) {
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"\n Benvingut al Dozer! \n\tIP-SERVER: %s:%d\n",config_gekko.host_ip,config_gekko.host_port);
		print_message(buffer);
		
		pthread_create(&thread_gekko_id, NULL, thread_listen_gekko, 0);
		
		// Execute the shell
		while ( boolContinue == TRUE && gekkoContinue == TRUE) {
			boolContinue = menu();
		}
	
	}
	
	closeDozer();
	return EXIT_SUCCESS;
}
/*******************************************************************************************************
*
* @Nombre: 	connect_gekko
* @Def: 	Crea el socket, configura la conexion y realiza la conexion con el Gekko
* @Arg:	 	void
* @Ret:  	Out: int fd_socket_gekko: Devuelve el file descriptor del socket
*
********************************************************************************************************/
int connect_gekko() {
	struct sockaddr_in server; 
	int result = 0;
	int readed_bytes = 0;
	Frame frameTX = new_Frame_custom(FRAME_GEKKODOZER_CONNEXIO);
	Frame frameRX = new_Frame();
	strcpy(frameTX.source,op.name);
	
    server.sin_addr.s_addr = inet_addr(config_gekko.host_ip);
	server.sin_port = htons(config_gekko.host_port);
	server.sin_family = AF_INET;
	memset(server.sin_zero,0,8);

	fd_socket_gekko = socket(AF_INET, SOCK_STREAM, 0);
	if ( fd_socket_gekko < 0 ) {
		print_message("[Dozer] - ERROR: No se ha podido crear la conexion (socket)\n");
		return -1;
	}

	result = connect(fd_socket_gekko, (struct sockaddr*)&server, sizeof(server) );
    if ( result < 0) {
        print_message("[Dozer] - ERROR: No se ha podido crear la conexion (connect). Revise config.dat (ip & port)\n");
		close(fd_socket_gekko);
		fd_socket_gekko = -1;
        return -1;
    }
	
	write(fd_socket_gekko, &frameTX, sizeof(frameTX));
	readed_bytes = read(fd_socket_gekko,&frameRX, sizeof(frameRX));
	if (frameRX.type == 'A') {
		print_message("[Dozer] - ERROR: Este operador ya esta conectado en otro terminal\n");
		return FALSE;
	} else if (frameRX.type != 'O') {
		print_message("[Dozer] - ERROR: Error al conectar con el servidor Gekko\n");
		return FALSE;
	}
	
	return TRUE;
	
}
/*********************************************************************************************************
*
* @Nombre: 	disconnect_tumbdice
* @Def: 	Funcion encargada de desconectar el programa del servidor TD.
* @Arg:	 	void
* @Ret:  	void
*
***********************************************************************************************************/
void disconnect_gekko() {
	Frame frameTX = new_Frame_custom(FRAME_GEKKODOZER_DESCONNEXIO);
	strcpy(frameTX.source,op.name);
	
	write(fd_socket_gekko,&frameTX,sizeof(frameTX));
	close(fd_socket_gekko);
}
/***********************************************************************************************************
*
* @Nombre: 	menu
* @Def: 	Funcion encargada de mostrar la shell y leer el comando que el usuario introduce
* @Arg:	 	void
* @Ret:  	Out: int: Devuelve valor TRUE o FALSE dependiendo si es un comando normal o si es un comando EXIT.
*
************************************************************************************************************/
int menu(void) {
	int nbytes = 0;
	int i = 0;
	int found = -1;
	int size = 0;
	char sText[FRAME_DATA_SIZE];
	char buffer[STRING_BUFFER_SIZE];
	char dinero[20];
	Frame frameTX;
	elemento auxElemento;
	int ok;
	int num;
	Share share;
		
	bzero(buffer,STRING_BUFFER_SIZE);
	pthread_mutex_lock(&mtx_shell);
	sprintf(buffer,"\n\n%s>",op.name);
	print_message(buffer);
	nbytes = read(0,buffer,STRING_BUFFER_SIZE); buffer[nbytes-1]='\0';
	
	string_to_lowercase(buffer,nbytes);
	
	
	if ( strcmp(buffer,"exit") == 0 ) {
		pthread_mutex_unlock(&mtx_shell);
		return FALSE;
	} else if ( strcmp(buffer,"show stock") == 0) {
		
		pthread_mutex_lock(&mtx_stockdata);
		shell_show_stock();
		pthread_mutex_unlock(&mtx_stockdata);
		pthread_mutex_unlock(&mtx_shell);
		
	} else if ( strcmp(buffer,"show me the money") == 0)  {
		
		pthread_mutex_lock(&mtx_stockdata);
		shell_show_me_the_money();
		pthread_mutex_unlock(&mtx_stockdata);
		pthread_mutex_unlock(&mtx_shell);
		
	} else if ( strcmp(buffer,"show ibex") == 0) {
		frameTX = prepare_Frame(op.name,'X',"PETICIO IBEX35");
		write(fd_socket_gekko, &frameTX, sizeof(frameTX));
	} else {
		// Separamos buffer
		
		ok = desglosa(&share,buffer);
		
		if(ok == 1){
			i=0;
			bzero(sText, sizeof(sText));
			strcpy(sText, share.sharecode);
			strcat(sText,"-");
			strcat(sText, share.quantity);
			for (i = strlen(sText); i < FRAME_DATA_SIZE; i++) {
				sText[i] = '\0';
			}
			if ( strcmp(share.command,"buy") == 0) {
				num = atoi(share.quantity);
				if(num <= 0){
					print_message("\nComanda incorrecta\n");
					pthread_mutex_unlock(&mtx_shell);
				}else{
					strcat(sText, "[");
					pthread_mutex_lock(&mtx_stockdata);
					sprintf(dinero,"%.2f", op.money);
					pthread_mutex_unlock(&mtx_stockdata);
					strcat(sText,dinero);
					strcat(sText, "]");
					for (i = strlen(sText); i < FRAME_DATA_SIZE; i++) {
						sText[i] = '\0';
					}
					frameTX = prepare_Frame(op.name,'B',sText);
					write(fd_socket_gekko, &frameTX, sizeof(frameTX));
				}
			} else if ( strcmp(share.command,"sell") == 0) {
				
				//Buscar el ibex en la lista de ibex que disponemos. Si existe en nuestra lista, se puede vender.
				pthread_mutex_lock(&mtx_stockdata);
				found = LISTA_BuscaIbex(&op.acciones,share.sharecode);
				pthread_mutex_unlock(&mtx_stockdata);
				if(found == 0){
					print_message("\n[Dozer] - ERROR: No dispone de acciones del Ibex que quiere vender.\n");
					pthread_mutex_unlock(&mtx_shell);
				}else{
					pthread_mutex_lock(&mtx_stockdata);
					auxElemento = LISTA_Consulta(op.acciones);
					pthread_mutex_unlock(&mtx_stockdata);
					if(atoi(share.quantity) > auxElemento.num){
						print_message("\n[Dozer] - ERROR: No dispone de suficientes acciones para ese IBEX en concreto.\n");
						pthread_mutex_unlock(&mtx_shell);
					}else{
						size = strlen(share.quantity);
						for(i = 0; i < size; i++){
							if(share.quantity[i] >= '0' && share.quantity[i] <= '9'){
								found = 1;
							}else{
								found = 0;
								break;
							}
						}
						num = atoi(share.quantity);
						if(num <= 0){
							print_message("\nComanda incorrecta\n");
							pthread_mutex_unlock(&mtx_shell);
						}else{
							if(found == 1){
								for (i = strlen(sText); i < FRAME_DATA_SIZE; i++) {
									sText[i] = '\0';
								}
								frameTX =  prepare_Frame(op.name,'S',sText);
								write(fd_socket_gekko, &frameTX, sizeof(frameTX));
							}else{
								print_message("\nComanda incorrecta\n");
								pthread_mutex_unlock(&mtx_shell);
							}
						}
					}
				}
			
			} else if ( strcmp(share.command,"erase") == 0) {
				size = strlen(share.quantity);
					for(i = 0; i < size; i++){
					
						if(share.quantity[i] >= '0' && share.quantity[i] <= '9'){
							found = 1;
						}else{
							found = 0;
							break;
						}
					}
					num = atoi(share.quantity);
					if(num <= 0){
						print_message("\nComanda incorrecta\n");
						pthread_mutex_unlock(&mtx_shell);
					}else{
						if(found == 1){
							for (i = strlen(sText); i < FRAME_DATA_SIZE; i++) {
								sText[i] = '\0';
							}
							frameTX =  prepare_Frame(op.name,'D',sText);
							write(fd_socket_gekko, &frameTX, sizeof(frameTX));
						}else{
							print_message("\nComanda incorrecta\n");
							pthread_mutex_unlock(&mtx_shell);
						}
					}
				
			} else {
				pthread_mutex_unlock(&mtx_shell);
				print_message("\nComanda incorrecta\n");
			}
			
		}else{
			pthread_mutex_unlock(&mtx_shell);
			print_message("\nComanda incorrecta\n");
		}
	}
	return TRUE;
}
/* SHELL FUNCTIONS */
/*********************************************************************************************************
 *
 *   @Nombre: desglosa
 *   @Def: Función que se encarga de desglosar el comando que introduce el cliente cuando va a comprar, vender
 *	       o borrar.
 *   @Arg: *share: Estructura de la accion. Guarda el tipo de acción, el IBEX y la cantidad.
 *		   buffer: comando que introduce el cliente. Ej: buy BBVA 100.
 *   @Ret: Retorna -1 si encuentra un error en el comando. Retorna 1 si todo va bien.
 *
 *********************************************************************************************************/
int desglosa(Share *share, char buffer[]){
	int i;
	int j;
	int espacios = 0;
	
	
		i = 0;
        j = 0;
		while(buffer[i] == ' '){
				espacios++;
				i++;
		}
		if(espacios == 0){
			share->command = (char*)malloc(sizeof(char));
			while (buffer[i] != ' ' && buffer[i] != '\0') {
				share->command[j] = buffer[i];
				i++;
				j++;
				share->command = (char*)realloc(share->command, sizeof(char) * (j+1));
			}
			share->command[j] = '\0';
			if(buffer[i] == '\0'){
				free(share->command);
				return -1;
			}else{
				while(buffer[i] == ' '){
					espacios++;
					i++;
				}
				if(espacios > 1){
					free(share->command);
					return -1;
				}else{
					
					j = 0;
					share->sharecode = (char*)malloc(sizeof(char));
					while (buffer[i] != ' ' && buffer[i] != '\0') {
						share->sharecode[j] = buffer[i];
						i++;
						j++;
						share->sharecode = (char*)realloc(share->sharecode, sizeof(char) * (j+1));
					}
					share->sharecode[j] = '\0';
					string_to_uppercase(share->sharecode,j);
					if(buffer[i] == '\0'){
						free(share->command);
						free(share->sharecode);
						return -1;
					}else{
						while(buffer[i] == ' '){
							espacios++;
							i++;
						}
						if(espacios > 2){
							free(share->command);
							free(share->sharecode);
							return -1;
						}else{
							
							j = 0;
							share->quantity = (char*)malloc(sizeof(char));
							while (buffer[i] != ' ' && buffer[i] != '\0') {
								share->quantity[j] = buffer[i];
								i++;
								j++;
								share->quantity = (char*)realloc(share->quantity, sizeof(char) * (j+1));
							}
							share->quantity[j] = '\0';
							if(espacios == 2){
								return 1;
								free(share->command);
								free(share->sharecode);
								free(share->quantity);
							}else{
								free(share->command);
								free(share->sharecode);
								free(share->quantity);
								return -1;

							}
							
						}
					}
					
				}
			}
		}else{
			return -1;
		}
}
/**************************************************************************************************
*
* @Nombre: 	shell_show_me_the_money
* @Def: 	Funcion que muestra por pantalla el dinero disponible del cliente.
* @Arg:	 	void
* @Ret:  	void
*
****************************************************************************************************/
void shell_show_me_the_money(void) {
	char buffer[STRING_BUFFER_SIZE];
	bzero(buffer,STRING_BUFFER_SIZE);
	sprintf(buffer,"\n\n%.2f€\n", op.money);
	print_message(buffer);
}
/***************************************************************************************************
*
* @Nombre: 	shell_show_stock
* @Def: 	Funcion que muestra las acciones que tiene compradas el cliente
* @Arg:	 	void
* @Ret:  	void
*
****************************************************************************************************/
void shell_show_stock(void) {
	char buffer[STRING_BUFFER_SIZE];
	elemento e;
	print_message("\n\n");	
	LISTA_VesInici(&(op.acciones));
	
	while ( !LISTA_Final(op.acciones) ) {
				
		bzero(buffer,STRING_BUFFER_SIZE);
		e=LISTA_Consulta(op.acciones);
		sprintf(buffer,"%s-%d\n",e.ibex,e.num);
		LISTA_Avanza(&(op.acciones));
		print_message(buffer);				
	}
}
/* INTERRUPTIONS FUNCTIONS */
/***********************************************************************************************************
*
* @Nombre: 	irq_handler
* @Def: 	Funcion que actua cuando se recibe una interrupcion
* @Arg:	 	void
* @Ret:  	void
*
*************************************************************************************************************/
void irq_handler(int irq_signal) {
	if ( irq_signal == SIGINT || irq_signal == SIGTERM ) {
		closeDozer();
	}
}
/*************************************************************************************************************
*
* @Nombre: 	print_message
* @Def: 	Funcion que escribe un mensaje por pantalla
* @Arg:	 	void
* @Ret:  	void
*
**************************************************************************************************************/
void print_message(char *message) {
	write(FD_SCREEN, message, strlen(message));
}
/**************************************************************************************************************
*
* @Nombre: 	initGlobals
* @Def: 	Funcion que inicia las variables globales
* @Arg:	 	void
* @Ret:  	void
*
***************************************************************************************************************/
void initGlobals(void) {
	bzero(op.name,OPERATOR_NAME_SIZE);
	op.money = 0;
	boolContinue = TRUE;
	gekkoContinue = TRUE;
	config_gekko = new_Config();
	fd_socket_gekko = -1;
}
/****************************************************************************************************************
*
* @Nombre: 	closeGlobals
* @Def: 	Funcion que destruye los semaforos, cierra sockets
* @Arg:	 	void
* @Ret:  	Out: Frame: Devuelve una trama limpia.
*
******************************************************************************************************************/
void closeGlobals(void) {
	pthread_mutex_destroy(&mtx_shell);
	pthread_mutex_destroy(&mtx_stockdata);
	LISTA_Destrueix(&op.acciones);
}
/******************************************************************************************************************
*
* @Nombre: 	closeDozer
* @Def: 	Funcion que se encarga de realizar un cierre correcto del programa
* @Arg:	 	void
* @Ret:  	void
*
*******************************************************************************************************************/
void closeDozer(void) {
	write_file_stock();
	disconnect_gekko();
	closeGlobals();
	print_message("\nSayonara\n");
	exit(EXIT_SUCCESS);
}
/****************************************************************************************************************
*
* @Nombre: 	string_to_lowercase
* @Def: 	Funcion que convierte las letras mayusculas en minusculas cuando se introduce un comando por shell
* @Arg:	 	In: char *string: Mensaje que queremos convertir a minusculas
* @Arg:		In: int size: Tamaño del array de caracteres de la string.
* @Ret:  	void
*
*****************************************************************************************************************/
void string_to_lowercase(char *string, int size) {
	int i=0;
	for ( i = 0; i<size; i++ ) {
		string[i] = tolower(string[i]);
	}
}
/****************************************************************************************************************
*
* @Nombre: 	string_to_lowercase
* @Def: 	Funcion que convierte las letras mayusculas en minusculas cuando se introduce un comando por shell
* @Arg:	 	In: char *string: Mensaje que queremos convertir a minusculas
* @Arg:		In: int size: Tamaño del array de caracteres de la string.
* @Ret:  	void
*
*****************************************************************************************************************/
void string_to_uppercase(char *string, int size) {
	int i=0;
	for ( i = 0; i<size; i++ ) {
		string[i] = toupper(string[i]);
	}
}
/****************************************************************************************************************
*
* @Nombre: 	read_file_config_gekko
* @Def: 	Funcion que lee el fichero de configuracion para la conexion con Gekko
* @Arg:	 	In: char[] file: Nombre del fichero desde donde queremos cargar la configuracion
* @Ret:  	Out: int : Variable de control de resultado
*
*****************************************************************************************************************/
int read_file_config_gekko(char file[]) {
	char c;
    int nOK;
    int i;
    int j;
	int fd;
    char tmp[10];
	char buffer[STRING_BUFFER_SIZE];

	fd = open(file, O_RDONLY);
	if(fd == -1){
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Dozer] - ERROR: No se pudo abrir el fichero de configuracion del servidor [%s] \n",file);
		print_message(buffer);
		exit(EXIT_FAILURE);
	}else{	   

		nOK = read(fd,&c,1);
		while(nOK!=0){
			i=0;
			while(c!='\n'){
				config_gekko.host_ip[i]=c;
				i++;
				nOK = read(fd,&c,1);
			}
			config_gekko.host_ip[i]='\0'; 	    
			j=0;
			nOK = read(fd,&c,1);
			while(c!='\n'){
				tmp[j]=c;
				j++;
				nOK = read(fd,&c,1);
			}
			tmp[j] = '\0';
			
			config_gekko.host_port = atoi(tmp);
			if(c =='\n') break;
		}	
		
		close(fd);
	}
	return 0;
}
/****************************************************************************************************************
*
* @Nombre: 	read_file_dtbase_stock
* @Def: 	Funcion que lee el fichero de datos del cliente, con la informacion cargada.
* @Arg:	 	In: char[] file: Nombre del fichero desde donde queremos cargar la configuracion
* @Ret:  	Out: int : Variable de control de resultado
*
*****************************************************************************************************************/
int read_file_dtbase_stock(char file[]) {
	char c;
    int nOK;
    int i;
	int k;
    int j;
    char tmp[10];
	int fd;
    elemento e;
	char buffer[STRING_BUFFER_SIZE];
	   
	fd = open(file, O_RDONLY);
	if(fd == -1){
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"[Dozer] - ERROR: No se pudo abrir el fichero de stock [%s] \n",file);
		print_message(buffer);
		exit(EXIT_FAILURE);
	}else{	   
	
	//Se crea la lista de accines del usuario
	LISTA_Crea(&(op.acciones));
	
    nOK = read(fd,&c,1);
    while(nOK!=0){
		i=0;
		while(c!='\n'){
            op.name[i]=c;
			i++;
            nOK = read(fd,&c,1);
        }
        op.name[i]='\0';
  	    
	  j=0;
        nOK = read(fd,&c,1);
        while(c!='\n'){
            tmp[j]=c;
            j++;
            nOK = read(fd,&c,1);
        }
		tmp[j] = '\0';
		op.money = atof(tmp);
		
		k=0;
		nOK = read(fd,&c,1);
		
			while(c!='\n'){
				while(c!='-'){
					e.ibex[k] = c;
					k++;
					nOK = read(fd,&c,1);
				}
				e.ibex[k]='\0';
				nOK = read(fd,&c,1);
				k=0;
				while(c!='\n'){
					tmp[k]=c;
					k++;
					nOK = read(fd,&c,1);
				}
				tmp[k] = '\0';
				e.num = atoi(tmp);
				LISTA_Insereix(&(op.acciones),e);
				nOK = read(fd,&c,1);
				k=0;
			}
		
		if(c == '\n') break;
		}
   	}	
		
  	close(fd);
    return 0; 
}
/**************************************************************************************************************
*
* @Nombre: 	read_file_dtbase_stock
* @Def: 	Funcion que guarda la informacion de datos del cliente de nuevo al fichero
* @Arg:	 	void
* @Ret:  	void
*
**************************************************************************************************************/
void write_file_stock(void) {
	char str[50]; 
	elemento e;
	int fdStock = open("stock.dat", O_WRONLY | O_TRUNC | O_CREAT, 0666 );
	if ( fdStock == -1 ) {
		print_message("[Dozer] - ERROR: No se ha podido abrir el fichero para guardar los datos... \n");
		
	} else {
		// Saving login
		bzero(str,sizeof(str));
		sprintf(str,"%s\n",op.name);
		write(fdStock,str,strlen(str));
		
		// Saving total amount
		sprintf(str,"%f\n",op.money);
		write(fdStock,str,strlen(str));
		
		// Saving all ibex data
		LISTA_VesInici(&(op.acciones));
		while ( !LISTA_Final(op.acciones) ) {
			bzero(str,50);
			e=LISTA_Consulta(op.acciones);
			sprintf(str,"%s-%d\n",e.ibex,e.num);
			write(fdStock,str,strlen(str));
			LISTA_Avanza(&(op.acciones));
		}
		
		close(fdStock);
	}
}
/**************************************************************************************************************
*
* @Nombre: 	print_ibex_list
* @Def: 	Funcion que printa los 35 IBEX con sus actualizaciones.
* @Arg:	 	frameRX: Recibe la trama que recibe del Gekko.
* @Ret:  	void
*
**************************************************************************************************************/
void print_ibex_list(Frame frameRX) {
	char buffer[STRING_BUFFER_SIZE];
	int readed_bytes = 0;
	int i = 1; // Ya hemos recibido una trama X
	
	bzero(buffer,STRING_BUFFER_SIZE);
	sprintf(buffer,"\n\n%s \n",frameRX.data);
	print_message(buffer);	
	
	for ( i = 1; i<35; i++) {
		readed_bytes = read(fd_socket_gekko, &frameRX, sizeof(frameRX));
		if ( readed_bytes < 0 ) {
			print_message("[Dozer] - ERROR: Gekko se ha desconectado.\n");
			raise(SIGINT);
		}
		if ( frameRX.type != 'X' ) {
			print_message("[Dozer] - ERROR: Se ha recibido una respuesta no esperada\n");
			break;
		} else {
			
			
		}
		
		bzero(buffer,STRING_BUFFER_SIZE);
		sprintf(buffer,"%s \n",frameRX.data);
		print_message(buffer);	
	}
	
}
