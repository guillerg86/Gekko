/********************************************************************
*
* @Archivo: 	lista_dozer.h
* @Finalidad:	
* @Autor:	Itziar Sanchez (LS26023) , Guille Rodriguez (LS26151)
* @Fecha: 	
*
********************************************************************/


#include<stdio.h>
#include <stdlib.h>
#include<conio.h>
#include<string.h>

typedef struct{
    char sOperador[14];
    int nFdSocket;
	char data_enviar[100];
}Dozer;

typedef struct n2{
			Dozer e;
			struct n2 *sig;
}nodo2;
typedef struct{
			nodo2 *pri;
			nodo2 *pdi;
}Clients_dozers;

Dozer new_Dozer(void);
void LISTA_DOZER_Crea(Clients_dozers *l);
void LISTA_DOZER_Destrueix(Clients_dozers *l);
void LISTA_DOZER_Insereix(Clients_dozers *l,Dozer e);
Dozer LISTA_DOZER_Consulta(Clients_dozers l);
void LISTA_DOZER_Elimina(Clients_dozers *l);
void LISTA_DOZER_VesInici(Clients_dozers *l);
void LISTA_DOZER_Avanza(Clients_dozers *l);
int LISTA_DOZER_Final(Clients_dozers l);
int LISTA_DOZER_Buida(Clients_dozers l);
int LISTA_DOZER_BuscaDozer(Clients_dozers *l,char dozer[14]);
void LISTA_DOZER_EliminaDozer(Clients_dozers *l,char dozer[14]);
//void LISTA_DOZER_ActualizaIbex(Clients_dozers *l,elemento e);
