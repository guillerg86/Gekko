#include "lista.h"
#include "Operator.h"
/**********************************************
* @Nombre: 	new_Operator
* @Def: 	Funcion encargada de inicializar y limpiar una struct Operator.
* @Arg:	 	void
* @Ret:  	void
**********************************************/
Operator new_Operator(void) {
	Operator op;
	bzero(op.name,OPERATOR_NAME_SIZE);
	op.money = 0;
}