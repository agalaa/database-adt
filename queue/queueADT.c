/**
*   @file queueADT.c
*   Implementacion de una cola con deepCopy
*   Fecha de ultima modificacion 06/11/2007.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queueADT.h"

/**
*	\struct nodeT
* 	Estructura de la celda de la cola.
* 	@param data Puntero al tipo de dato.
* 	@param next Putero a otra celda
*/ 
typedef struct nodeT
{
	queueElemT data;
	struct nodeT *next;
}nodeT;

/**
*	\struct queueCDT
* 	Header de la cola.
* 	@param deepCpy Puntero a funcion de copia.
* 	@param freeCpy Puntero a la funcion de liberacion. 
* 	@param first Puntero anext Putero a otra celda
*/ 
struct queueCDT
{	
	deepCopyT deepCpy;
	freeCpyT freeCpy;
	nodeT *first;
	nodeT *last;
};

queueADT newQueue( deepCopyT dpcpy, freeCpyT freecpy )
{
	queueADT queue;
	
	if ( ( queue = malloc(sizeof(struct queueCDT)) ) == NULL )
		return NULL;
	
	queue->deepCpy = dpcpy;
	queue->freeCpy = freecpy;
	queue->first = NULL;
	
	return queue;
}

int enqueue( queueADT queue, queueElemT elem )
{
	nodeT *aux;
		
	if ( ( aux = malloc ( sizeof(nodeT) ) ) == NULL )
		return 0;
	
	aux->data = queue->deepCpy( elem );
	
	aux->next = NULL;
	
	if ( queue->first == NULL )
		queue->first = aux;
	else
		queue->last->next = aux;
	
	queue->last = aux;
	
	return 1;
}

queueElemT dequeue( queueADT queue )
{
	queueElemT aux;
	nodeT *cp;
	
	cp = queue->first;
	
	if ( cp == NULL )
		return NULL;
	
	aux = queue->deepCpy( queue->first->data );
	
	queue->freeCpy(queue->first->data);
	
	free( cp );
	
	queue->first = queue->first->next;
	
	return aux;
}

int
queueLength( queueADT queue )
{
	int n;
	nodeT * aux;
	n = 0;
	
	/* Otra opciones: Se puede llevar cuenta de las enqueueciones y retornar ese valor  */

	for ( aux = queue->first ; aux != NULL ; aux = aux->next )
		n++;
	
	return n;
}

int queueIsEmpty( queueADT queue )
{
	return queue->first == NULL;
}

void freeQueue( queueADT queue )
{
	nodeT * aux;
	
	while ( !queueIsEmpty( queue ) )
	{
		aux = queue->first;
		queue->first = queue->first->next;
		queue->freeCpy( aux->data );
		free( aux );
	}
	
	free( queue );
	
	return;
}
