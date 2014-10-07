/*
 * nivel.h
 *
 *  Created on: 03/07/2013
 *      Author: utnso
 */

#ifndef NIVEL_H_
#define NIVEL_H_


#include "../sockets.h"
#include "../mensajeria.h"
#include <stdio.h>
#include "commons/config.h"
#include <stdlib.h>	/*por el malloc*/
#include <string.h> /*por el strcmp*/
#include "commons/collections/list.h"
#include <unistd.h>
#include <pthread.h>
#include <commons/log.h>




#define MAX_X 1000
#define MAX_Y 1000

typedef struct{
	t_list* personajesBloqueados;
	pthread_mutex_t acceso;
}tipoLista;

typedef struct {				//Vector del contenido del nivel
	char* inicial;
	int instancias;
	unsigned int posX;
	unsigned int posY;
}t_contenidoNivel;

typedef struct {			//nodo de personaje(colaDeConectados)
	char* simbolo;
	int socket;
	int Interbloqueo;	//1 no esta en interbloqueo, en 0 esta.
	t_list* recursosAsignados;
	char* recursoSolicitado;
	int bloqueado;
}t_recursosDelPersonaje;

typedef struct{				//vector temporal
	char* inicial;
	int instancias;
}t_temporal;



t_recursosDelPersonaje* crearPersonaje();
t_recursosDelPersonaje* buscoPersonajePorSimbolo(t_list*,char*);
int buscarIndiceDePersonaje(t_list* unaLista,t_recursosDelPersonaje* unPersonaje);
t_recursosDelPersonaje* encontrarUnPersonaje(t_list*, int);
void* chequeoInterbloqueo();


t_contenidoNivel contenidoNivel[10];
tipoLista* personajesBloqueados;
int idSocketThread;
char * path_config;
t_log* logger;

#endif /* NIVEL_H_ */
