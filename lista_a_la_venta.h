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

typedef struct{
    char sOperador[14];
    int nNumAcciones;
}Operacion;

typedef Operacion elemento;
typedef struct n{
			elemento e;
			struct n *sig;
}nodo;
typedef struct{
			nodo *pri;
			nodo *pdi;
}lista;
typedef struct{
			char nomAccion[5];
			lista listaVentas;
}Ventas;
			

void LISTA_VENTA_Crea(lista *l);
void LISTA_VENTA_Destrueix(lista *l);
void LISTA_VENTA_Insereix(lista *l,elemento e);
elemento LISTA_VENTA_Consulta(lista l);
void LISTA_VENTA_Elimina(lista *l);
void LISTA_VENTA_VesInici(lista *l);
void LISTA_VENTA_Avanza(lista *l);
int LISTA_VENTA_Final(lista l);
int LISTA_VENTA_Buida(lista l);
int LISTA_VENTA_BuscaOperador(lista *l,char operador[]);
void LISTA_VENTA_ActualizaOperador(lista *l,elemento e);
