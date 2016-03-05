/********************************************************************
*
* @Archivo: 	dozer.c 
* @Finalidad:	Fase 1 de la practica de sistemas operativos
* @Autor:	Itziar Sanchez (LS26023) , Guille Rodriguez (LS26151)
* @Fecha: 	04/11/2014
*
********************************************************************/


#include<stdio.h>
#include <stdlib.h>
#include<conio.h>
#include<string.h>

typedef struct {
	char ibex[5];
	int num;
} IBEX;

typedef IBEX elemento;
typedef struct n{
			elemento e;
			struct n *sig;
			}nodo;
typedef struct{
			nodo *pri;
			nodo *pdi;
			}lista;
void LISTA_Crea(lista *l);
void LISTA_Destrueix(lista *l);
void LISTA_Insereix(lista *l,elemento e);
elemento LISTA_Consulta(lista l);
void LISTA_Elimina(lista *l);
void LISTA_VesInici(lista *l);
void LISTA_Avanza(lista *l);
int LISTA_Final(lista l);
int LISTA_Buida(lista l);
int LISTA_BuscaIbex(lista *l,char ibex[100]);
void LISTA_ActualizaIbex(lista *l,elemento e);
