/********************************************************************
*
* @Archivo: 	dozer.c 
* @Finalidad:	Fase 1 de la practica de sistemas operativos
* @Autor:	Itziar Sanchez (LS26023) , Guille Rodriguez (LS26151)
* @Fecha: 	04/11/2014
*
********************************************************************/

#include "lista_dozer.h"



Dozer new_Dozer(void) {
	Dozer dozer;
	bzero(dozer.sOperador,14);
	dozer.nFdSocket = -1;
	return dozer;
}


void LISTA_DOZER_Crea(Clients_dozers *l){
l->pri=(nodo2*)malloc(sizeof(nodo2));
if (l->pri==NULL){ }
else{
	l->pdi=l->pri;
	l->pri->sig=NULL;
}
}
//***********
void LISTA_DOZER_Destrueix(Clients_dozers *l){
	
	nodo2 *q;

	while(l->pri!=NULL){
		q=l->pri;
		l->pri=q->sig;
		free(q);
	}
}//*************


void LISTA_DOZER_Insereix(Clients_dozers *l,Dozer e){
	
	nodo2 *aux;
	l->pdi=l->pri;
		while (l->pdi->sig!=NULL){
			l->pdi=l->pdi->sig;
		}
	aux=(nodo2*)malloc(sizeof(nodo2));
	if (aux==NULL){  }
	else{
		strcpy(aux->e.sOperador,e.sOperador);
		aux->e.nFdSocket = e.nFdSocket;
		strcpy(aux->e.data_enviar,e.data_enviar);
		aux->sig=l->pdi->sig;
		l->pdi->sig=aux;
	}
}
//**********
Dozer LISTA_DOZER_Consulta(Clients_dozers l){
Dozer e;
if (l.pdi->sig==NULL) {}
else e=l.pdi->sig->e;
return e;
}
//***********
void LISTA_DOZER_Elimina(Clients_dozers *l){
nodo2 *q;
if  (l->pdi->sig==NULL) {}
else{
	q=l->pdi->sig;
	l->pdi->sig=q->sig;
	free(q);
}
}
//***********
void LISTA_DOZER_VesInici(Clients_dozers *l){
if(l->pri->sig==NULL)	{}
else l->pdi=l->pri;
}
//*********
void LISTA_DOZER_Avanza(Clients_dozers *l){
	if(l->pdi->sig==NULL) {}
	else{
		l->pdi=l->pdi->sig;
	}
}
//**************
int LISTA_DOZER_Final(Clients_dozers l){
	return l.pdi->sig==NULL;
}
//********
int LISTA_DOZER_Buida(Clients_dozers l){
	return l.pri==l.pdi;
}

int LISTA_DOZER_BuscaDozer(Clients_dozers *l,char dozer[14]){
int ok=-1;
	l->pdi=l->pri;
	while(l->pdi->sig!=NULL && strcmp(l->pdi->sig->e.sOperador,dozer)!=0){
		l->pdi=l->pdi->sig;
	}
	if  (l->pdi->sig!=NULL){
		ok=1;
	}
	return ok;
}
void LISTA_DOZER_EliminaDozer(Clients_dozers *l,char dozer[14]){
	int ok;
		
	ok = LISTA_DOZER_BuscaDozer(l,dozer);
	LISTA_DOZER_Elimina(l);
	
}

