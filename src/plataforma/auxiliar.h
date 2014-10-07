/*
 * auxiliar.h
 *
 *  Created on: 20/06/2013
 *      Author: utnso
 */

#ifndef AUXILIAR_H_
#define AUXILIAR_H_

#include <string.h>
#include "commons/string.h"
#include <stdio.h>
#include <string.h>    //strlen
#include <stdarg.h>
#include "commons/error.h"
#include <stdlib.h>
#include "../sockets.h"
#include "../colas.h"
#include "../mensajeria.h"
#include <pthread.h>
#include "commons/log.h"

typedef struct{
	t_log* logOrquestador;
	pthread_mutex_t acceso;
}tipoLog;

typedef struct{
	char* nivelABuscar;
	cola_t* colaNiveles;
}UnNivel;

typedef struct{
	char* recursosAsignados;
	char* simboloPersonajeAdesbloq;

}t_desbloqueo;

typedef enum{
	LISTO,BLOQUEADO,FINALIZADO
} estadoPersonaje;


typedef struct {
	int fd; 		/*Descriptor del socket al q esta asociado el proceso, para poder devolver la respuesta dsp*/
	estadoPersonaje estado; //estado del personaje
	char* recursoEnEspera; //recurso que espera si esta bloqueado
	char* simbolo;
	int ordenLlegada;
	} TprocesoPersonaje;



typedef struct{
	int idSock;
	char* nivel;
	char* ipPuertoNivel;
	char* ipPuertoPlan;
	int seCreoSockPlan;
	cola_t* colaListos;
	cola_t* colaFinalizados;
	cola_t* colaBloqueados;
	TprocesoPersonaje* pjEjec;
	pthread_t planificador;
}tipoNiveles;

typedef struct{
	char* personaje;
	int ordenLlegada;
}interbloq;


tipoNiveles* crearElemento();
void imprimirColaNiveles(UnNivel* Nivel);
tipoNiveles* encontrarUnNivel(UnNivel* Nivel);
char* retornarRecursos(char* mensaje);
char** reemplazarUnRecurso(char** vector, int pos);
//t_desbloqueo* asignarRecursosApersonajes(char** recursos, tipoNiveles* unNivel);
t_desbloqueo* asignarRecursosApersonajes(char** recursos, cola_t* colaBloqueados);
void desbloquearPersonajes(tipoNiveles* unNivel, char** simbolos);
int tamanio_string(char* cadena,char* simbolo);
void agregarPersonaje(char* Personajes, char* simbolo);
char* resolverInterbloqueo(char** personajes, tipoNiveles* unNivel);
void matarPersonaje(char* simbolo, tipoNiveles* unNivel, t_log* logger);
void* lanzarHilo();
TprocesoPersonaje* crearNodoPersonaje();
void eliminarPersonajeDesc(tipoNiveles* unNivel, char* simbolo);
void loggeoPersonajes(t_log* logPlan, cola_t* cola, char* tipoCola);
void cerrarSockets(tipoNiveles* unNivel);
#endif /* AUXILIAR_H_ */
