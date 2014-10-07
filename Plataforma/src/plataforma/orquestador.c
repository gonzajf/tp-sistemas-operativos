/*
 * orquestador.c
 *
 *  Created on: 25/05/2013
 *      Author: utnso
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../mensajeria.h"
#include "../sockets.h"
#include <pthread.h>
#include <stdint.h>
#include "commons/string.h"
#include "commons/config.h"
#include "commons/log.h"
#include <stdarg.h>
#include "commons/error.h"
#include "../colas.h"
#include "auxiliar.h"
#include "variablesGlobales.h"
#include "plataforma.h"
#include <sys/inotify.h>



void* Orquestador() {


	MSJ* mensaje=(MSJ*)malloc(sizeof(MSJ));
	char* msj;

	int idSocket;
	char* ip;
	int puerto;
	int i;
	int error;
	int cantPersonajesFinalizados=0;
	int cantPersonajesEnJuego=0;
	int desconectar=0;

	tipoNiveles* unNivel=NULL;
	tipoNiveles* elementoNivel=NULL;
	t_desbloqueo* recursosYsimbolos=(t_desbloqueo*)malloc(sizeof(t_desbloqueo));
	pthread_t planif = NULL;

	char* recursos=string_new();
	char* personajes=string_new();
	char* personajeEliminado;
	char** simbolos;
	char** vector;
	char buffer[1000];

	t_log* logOrquestador=(t_log*)malloc(sizeof(t_log));
	logOrquestador = log_create("logOrquestador.txt","Orquestador",true,LOG_LEVEL_DEBUG);

	configPlataforma=config_create(direccion);
	quantum = config_get_int_value(configPlataforma,"quantum");
	ip=config_get_string_value(configPlataforma, "IP");
	puerto=atoi(config_get_string_value(configPlataforma, "PUERTO"));


	log_debug(logOrquestador, "Creé el archivo de configuración.");

	t_socket_servidor* orquestador=crearServidor(ip,puerto,logOrquestador);//crear socket orquestador y poner en escucha
	log_debug(logOrquestador, "Creé el servidor y espero conexión.");

	niveles=(UnNivel*)malloc(sizeof(UnNivel));
	niveles->nivelABuscar="";
	niveles->colaNiveles=crear_cola();

	int wd;
	int	fd_inotify = inotify_init();
	if(fd_inotify < 0){	perror("Error al iniciar inotify");}
	wd = inotify_add_watch(fd_inotify,direccion,IN_MODIFY);
	if(wd < 0){perror("Error al agregar el watch");}

	orquestador->sockClientes[0] = fd_inotify;

	while(1){
		idSocket=multiplexarClientes(orquestador,logOrquestador);
		if(idSocket == fd_inotify){
			read(fd_inotify, buffer, 1000);
			config_destroy(configPlataforma);
			sleep(2);
			configPlataforma=config_create(direccion);
			quantum = config_get_int_value(configPlataforma,"quantum");
			log_debug(logOrquestador, "Se ha cambiado el quantum a %d",quantum);
		}
		else if(idSocket>0){
			mensaje=recibirMensaje(idSocket,logOrquestador);
			switch(mensaje->tipoMensaje){
				case  e_handshake:
					//msj q envia nivel="nivel14:127.0.0.1:5001";
					elementoNivel=crearElemento();
					vector=string_split(mensaje->mensaje, ":");

					log_debug(logOrquestador, "Se registró un nivel nuevo: %s",vector[0]);

					niveles->nivelABuscar=vector[0];
					elementoNivel->nivel=vector[0];
					elementoNivel->ipPuertoNivel=concat(3,vector[1],":",vector[2]);
					puerto++;
					elementoNivel->ipPuertoPlan=concat(3,ip,":",itoa(puerto));
					elementoNivel->seCreoSockPlan=0;
					elementoNivel->colaBloqueados=crear_cola();
					elementoNivel->colaFinalizados=crear_cola();
					elementoNivel->colaListos=crear_cola();
					elementoNivel->idSock=idSocket;
					elementoNivel->pjEjec=NULL;
					pushear_cola(niveles->colaNiveles, elementoNivel);

					if (pthread_create(&planif, NULL, planificador,NULL) != 0){
						log_error(logOrquestador, "No se pudo crear el hilo planificador.");
					}else{
						log_debug(logOrquestador, "Se creó el hilo planificador.");
					}

					break;

				case e_ipPuerto:

					vector=string_split(mensaje->mensaje,":");
					niveles->nivelABuscar=vector[0];
					unNivel=encontrarUnNivel(niveles);
					//RECIBO UN MSJ TIPO "Nivel11:@"
					if(unNivel!=NULL){
						if(unNivel->seCreoSockPlan){
							msj=concat(3, unNivel->ipPuertoNivel, ":", unNivel->ipPuertoPlan);
							agregarPersonaje(personajes, vector[1]);
				    		enviarMensaje(idSocket,e_ipPuerto,msj,logOrquestador);
				    		log_debug(logOrquestador, "Se envió ip y puerto al personaje.");
				       	}
					}else{
						enviarMensaje(idSocket,e_error,"",logOrquestador);//ESTE MSJ SE ENVIA SI NO FUE CREADO EL SOCKET TODAVIA
						log_debug(logOrquestador, "El %s no se creó hasta el momento.",vector[0]);
					}
				    break;

				case e_recursosLiberados:
					//Nivel11:@:F:H:H
					log_debug(logOrquestador, "Recibí un mensaje de tipo e_recursosLiberados");
					vector=string_split(mensaje->mensaje, ":");
					niveles->nivelABuscar=vector[0];
					unNivel=encontrarUnNivel(niveles);
					eliminarPersonajeDesc(unNivel, vector[1]);

					if(vector[2]!=NULL){


					recursos=retornarRecursos(mensaje->mensaje);//retorna "F:H:H:M:"
					vector=string_split(recursos, ":");

					if(unNivel!=NULL){
						if(tamanio_cola(unNivel->colaBloqueados)){
							recursosYsimbolos=asignarRecursosApersonajes(vector, unNivel->colaBloqueados); //me devuelve simbolo:recurso "@:F:#:M:!:H"
							if(!string_is_empty(recursosYsimbolos->simboloPersonajeAdesbloq)){
								log_debug(logOrquestador,"Personajes:RecursosAsignados:%s",recursosYsimbolos->recursosAsignados);
								simbolos=string_split(recursosYsimbolos->recursosAsignados, ":");
								desbloquearPersonajes(unNivel, simbolos);
								enviarMensaje(idSocket,e_recursosLiberados,recursosYsimbolos->recursosAsignados,logOrquestador);

							}else{
								if(enviarMensaje(idSocket,e_recursosLiberados,"",logOrquestador)){
								log_debug(logOrquestador, "No se asignaron recursos.");
								}
							}
						}else{
							enviarMensaje(idSocket,e_recursosLiberados,"",logOrquestador);
							log_debug(logOrquestador, "No se han asignado recursos; no hay personajes bloqueados.");
						}
					}else{
						enviarMensaje(idSocket,e_error,"",logOrquestador);
						log_debug(logOrquestador, "No se encontró el nivel para asignar recursos.");
					}
					}else {log_debug(logOrquestador,"no me envió recursos para asignar.");
					  if(enviarMensaje(idSocket,e_recursosLiberados,"",logOrquestador)){
					    log_debug(logOrquestador, "No se asignaron recursos.");
					  }}
					break;
				case e_finalizoPlanNiveles:
					log_debug(logOrquestador,"Un personaje finalizó el plan d niveles.");
					cantPersonajesFinalizados++;
					cantPersonajesEnJuego=tamanio_string(personajes,":");
					desecharSocket(idSocket,orquestador);
					if(cantPersonajesFinalizados==cantPersonajesEnJuego){
						desconectar=1;
						log_debug(logOrquestador,"Finalizaron todos los personajes; accediendo a koopa.");
					}else{
						cantPersonajesEnJuego=cantPersonajesEnJuego-cantPersonajesFinalizados;
						log_debug(logOrquestador,"Quedan %d personajes por finalizar.",cantPersonajesEnJuego);
					}
					break;
				case e_procesosInterBlockeados:
					//recibo un mensaje:"Nivel1:@:#:!:"
					log_debug(logOrquestador, "Se produjo interbloqueo.");

					vector=string_split(mensaje->mensaje, ":");
					niveles->nivelABuscar=vector[0];
					unNivel=encontrarUnNivel(niveles);
					personajeEliminado=resolverInterbloqueo(vector,unNivel);
					matarPersonaje(personajeEliminado, unNivel, logOrquestador);  //lo unico q hace es borrarlo de la cola bloqueados
					enviarMensaje(idSocket,e_muerte,personajeEliminado,logOrquestador);
					break;

				case e_desconexion:
					for (i = 0; i < (tamanio_cola(niveles->colaNiveles)); ++i) {
						unNivel=obtener_contenido_pos_determinada(niveles->colaNiveles, i);
						if(idSocket==unNivel->idSock){
							log_debug(logOrquestador, "Se desconectó un nivel.");
							error=pthread_cancel(unNivel->planificador);
							if(error){
								log_error(logOrquestador, "No se pudo cancelar el hilo planificador.");
							}else{
								log_debug(logOrquestador, "El hilo planificador se canceló satisfactoriamente.");
								cerrarSockets(unNivel);
								unNivel=remover_cola_pos_determinada(niveles->colaNiveles , i);
							}
							break;
						}
					}
					desecharSocket(idSocket, orquestador);
					break;
				default:
					break;
				}
		}
		if(desconectar){
			log_debug(logOrquestador, "Empieza Koopa");
			log_destroy(logOrquestador);
			break;
		}
	}

	return NULL;
}
