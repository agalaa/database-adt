/**
*   @file main.h
*   Interface para queueADT.c.
* 	Incluye estructuras y typedefs.
*   Fecha de ultima modificacion 06/11/2007.
*/

#ifndef _QUEUE_ADT_H_
#define _QUEUE_ADT_H_

/**
*	\typedef queueADT
* 	Puntero al tipo concreto de dato de la cola.
*/ 
typedef struct queueCDT *queueADT;

/**
*	\typedef queueElemT
* 	Definicion del tipo de dato de la cola.
*/
typedef void *queueElemT;

/**
*	\typedef deepCopyT
* 	Definicion del prototipo de la funcion de copia
* 	para realizar Deep Copy.
*/
typedef queueElemT (*deepCopyT)( queueElemT elem );

/**
*	\typedef freeCpyT
* 	Definicion del prototipo de la funcion de liberacion
* 	para liberar copias realizadas por la funcion de copia.
*/
typedef void(*freeCpyT)( queueElemT elem );


/**
*	\fn newQueue
*   Creacion de la cola. Esta implementacion usa Deep Copy.
*   @param dpcpy Funcion de copia.
*   @param freecpy Funcion de liberacion del elemento.
*   @return queueADT puntero al CDT si todo esta bien, NULL en caso contrario.
*/
queueADT newQueue ( deepCopyT dpcpy, freeCpyT freecpy );

/**
*	\fn enqueue
*   Encola un elemento en la cola.
*   @param queue La cola que se creo con newQueue.
*   @param elem Elemento a encolar.
*   @return 1 si todo esta bien, 0 en caso contrario.
*/
int enqueue ( queueADT queue, queueElemT elem );

/**
*	\fn dequeue
*   Encola un elemento en la cola.
*   @param queue La cola que se creo con newQueue.
*   @return queueElemT si todo esta bien, NULL en caso contrario.
*/
queueElemT dequeue ( queueADT queue );

/**
*	\fn queueLength
*   Determina la extension de la cola.
*   @param queue La cola que se creo con newQueue.
*   @return int extension de la cola.
*/
int queueLength( queueADT queue );

/**
*	\fn queueIsEmpty
*   Se fija si la cola esta llena o no.
*   @param queue La cola que se creo con newQueue.
*   @return 1 si todo esta bien, 0 en caso contrario.
*/
int queueIsEmpty ( queueADT queue );

/**
*	\fn freeQueue
*   Libera la cola.
*   @param queue La cola que se creo con newQueue.
*/
void freeQueue( queueADT queue );

#endif
