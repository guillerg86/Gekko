/********************************************************************
*
* @Archivo: 	dozer.c 
* @Finalidad:	Fase 1 de la practica de sistemas operativos
* @Autor:	Itziar Sanchez (LS26023) , Guille Rodriguez (LS26151)
* @Fecha: 	04/11/2014
*
********************************************************************/

#include "lista.h"

void LISTA_Crea(lista *l){
l->pri=(nodo*)malloc(sizeof(nodo));
if (l->pri==NULL){ }
else{
	l->pdi=l->pri;
	l->pri->sig=NULL;
}
}
//***********
void LISTA_Destrueix(lista *l){
	
	nodo *q;

	while(l->pri!=NULL){
		q=l->pri;
		l->pri=q->sig;
		free(q);
	}
}//*************


void LISTA_Insereix(lista *l,elemento e){
	
	nodo *aux;
	l->pdi=l->pri;
		while (l->pdi->sig!=NULL && strcmp(l->pdi->sig->e.ibex,e.ibex)<0){
			l->pdi=l->pdi->sig;
		}
	aux=(nodo*)malloc(sizeof(nodo));
	if (aux==NULL){  }
	else{
		strcpy(aux->e.ibex,e.ibex);
		aux->e.num = e.num;
		aux->sig=l->pdi->sig;
		l->pdi->sig=aux;
	}
}
//**********
elemento LISTA_Consulta(lista l){
elemento e;
if (l.pdi->sig==NULL) {} 
else e=l.pdi->sig->e;
return e;
}
//***********
void LISTA_Elimina(lista *l){
nodo *q;
if  (l->pdi->sig==NULL) {}
else{
	q=l->pdi->sig;
	l->pdi->sig=q->sig;
	free(q);
}
}
//***********
void LISTA_VesInici(lista *l){
if(l->pri->sig==NULL)	{}
else l->pdi=l->pri;
}
//*********
void LISTA_Avanza(lista *l){
if(l->pdi->sig==NULL) {}
else{
l->pdi=l->pdi->sig;
}
}
//**************
int LISTA_Final(lista l){
return l.pdi->sig==NULL;
}
//********
int LISTA_Buida(lista l){
return l.pri==l.pdi;
}
//********
int LISTA_BuscaIbex(lista *l,char ibex[100]){
int ok=0;
l->pdi=l->pri;
while(l->pdi->sig!=NULL && strcmp(l->pdi->sig->e.ibex,ibex)!=0){
	l->pdi=l->pdi->sig;
}
if  (l->pdi->sig!=NULL){
ok=1;
}
return ok;
}
//*********
void LISTA_ActualizaIbex(lista *l,elemento e){
if(l->pdi->sig==NULL) {}
else l->pdi->sig->e=e;
}
