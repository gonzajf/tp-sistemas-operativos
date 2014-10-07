/*
 * planificador.c
 *
 *  Created on: 07/06/2013
 *      Author: utnso
 */

#include "variablesGlobales.h"
#include "../sockets.h"
#include "../colas.h"
#include <stdlib.h>
#include "../mensajeria.h"
#include "commons/log.h"
#include "commons/string.h"
#include "commons/config.h"
#include <sys/time.h>
#include "auxiliar.h"
#include <sys/types.h>
#include <unistd.h>



void* planificador()
{
	t_log* logPlanificador=(t_log*)malloc(sizeof(t_log));

	int q;

	double seg = config_get_double_value(configPlataforma,"tiempoDeEspera");
	useconds_t segundos=seg*1000000;

	struct timeval tiempoSelect;
	tiempoSelect.tv_sec = 0.01;
	tiempoSelect.tv_usec = 0;

	TprocesoPersonaje* personaje;

	tipoNiveles* elem=(tipoNiveles*)malloc(sizeof(tipoNiveles));
	elem = encontrarUnNivel(niveles);
	char ** ipPuerto=string_split(elem->ipPuertoPlan, ":");
	elem->planificador=pthread_self();
	char* nombrearch=concat(3,"Planif",elem->nivel,".txt");
	logPlanificador = log_create(nombrearch,elem->nivel,false,LOG_LEVEL_DEBUG);

	// creo el servidor. La ip y el puerto se leen por configPlataforma de configuracion
	t_socket_servidor* servidorPlanificador= crearServidor(ipPuerto[0], atoi(ipPuerto[1]),logPlanificador);
	elem->seCreoSockPlan=1;


	MSJ* msj;
	int descriptor;
	TprocesoPersonaje* procesoPersonaje = (TprocesoPersonaje*)malloc(sizeof(TprocesoPersonaje));
	char ** vector;
	TprocesoPersonaje* pers_actual=(TprocesoPersonaje*)malloc(sizeof(TprocesoPersonaje));
	int ordenLlegada=0;
	int i;
	int fd_inotify;


	while(1)
	{
		descriptor = multiplexarSockets(servidorPlanificador, &tiempoSelect,logPlanificador);//NO BLOQUEANTE

		//mientras haya actividad
		while(descriptor != -2)
		{
			if(descriptor>0)
			{
				msj = recibirMensaje(descriptor,logPlanificador);

				switch(msj->tipoMensaje)
				{
					case e_handshake:
						log_debug(logPlanificador, "Llegó un handshake.");
						//armo la estructura que necesito para manejar los personajes
						vector=string_split(msj->mensaje, ":");
						procesoPersonaje=crearNodoPersonaje();
						procesoPersonaje->fd = descriptor;
						procesoPersonaje->simbolo = vector[1];
						procesoPersonaje->estado = LISTO;
						procesoPersonaje->recursoEnEspera = "";
						procesoPersonaje->ordenLlegada=ordenLlegada++;

						//cuando tengo el personaje listo lo meto en la lista de listos
						pushear_cola(elem->colaListos, procesoPersonaje);
						loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");

						break;

					case e_desconexion:
						//elimino el socket de la lista del servidor
						desecharSocket(descriptor,servidorPlanificador);

						for (i = 0; i < (tamanio_cola(elem->colaListos)); ++i) {
							personaje=obtener_contenido_pos_determinada(elem->colaListos, i);
							if(personaje->fd==descriptor) break;
						}
						if (i<tamanio_cola(elem->colaListos)){
							personaje=remover_cola_pos_determinada(elem->colaListos,i);
							loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");
						}
						break;

					//termino el nivel
					case e_finalizoNivel: //CAMBIAR POR EL Q CORRESPONDE
						//busco y elimino

						for (i = 0; i < (tamanio_cola(elem->colaListos)); ++i) {
						  personaje=obtener_contenido_pos_determinada(elem->colaListos, i);
						  if(personaje->fd==descriptor) break;
						}
						personaje=remover_cola_pos_determinada(elem->colaListos,i);
						loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");
						personaje->estado = FINALIZADO;
						pushear_cola(elem->colaFinalizados, personaje);
						log_debug(logPlanificador, "El personaje %s finalizó el nivel %s.",personaje->simbolo,elem->nivel);
						loggeoPersonajes(logPlanificador, elem->colaFinalizados, "Finalizados");
						desecharSocket(descriptor,servidorPlanificador);
						break;
					default:
						break;
				}
			}

			descriptor = multiplexarSockets(servidorPlanificador, &tiempoSelect,logPlanificador);//NO BLOQUEANTE
		}


		q=quantum;

		if (tamanio_cola(elem->colaListos)>0)
		{

			pers_actual=remover_de_cola(elem->colaListos);
			elem->pjEjec=pers_actual;
			loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");

			while (q)
			{
				//sleep segun diga el config
				usleep(segundos);
				enviarMensaje(pers_actual->fd, e_movimiento, "",logPlanificador);
				msj=recibirMensaje(pers_actual->fd,logPlanificador);

				switch(msj->tipoMensaje)
				{
					case e_movimiento:
					q--;
					if (!q)
						pushear_cola(elem->colaListos, pers_actual);
					loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");
					log_debug(logPlanificador, "Se movió el personaje %s.",pers_actual->simbolo);
					break;

					case e_finalizoTurno:
					q=0;
					log_debug(logPlanificador, "Longitud de mensaje: %d.",msj->longitudMensaje);
					if (msj->longitudMensaje == 0){
						pushear_cola(elem->colaListos, pers_actual);
						loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");
						log_debug(logPlanificador, "El personaje %s finalizó turno.",pers_actual->simbolo);
					}else
					{
						pers_actual->estado = BLOQUEADO;
						pers_actual->recursoEnEspera = msj->mensaje;
						pushear_cola(elem->colaBloqueados, pers_actual);
						loggeoPersonajes(logPlanificador, elem->colaBloqueados, "Bloqueados");
						log_debug(logPlanificador, "El personaje %s finalizó turno y quedó bloqueado.",pers_actual->simbolo);
					}
					break;

					case e_desconexion:
						q=0;
						//elimino el socket de la lista del servidor
						desecharSocket(descriptor,servidorPlanificador);
						for (i = 0; i < (tamanio_cola(elem->colaListos)); ++i) {
							personaje=obtener_contenido_pos_determinada(elem->colaListos, i);
							if(personaje->fd==descriptor) break;
						}
						if (i<tamanio_cola(elem->colaListos)){
							personaje=remover_cola_pos_determinada(elem->colaListos,i);
							loggeoPersonajes(logPlanificador, elem->colaListos, "Listos");
						}
						break;

					case e_finalizoNivel:
						q=0;
						pers_actual->estado = FINALIZADO;
						pushear_cola(elem->colaFinalizados, pers_actual);
						loggeoPersonajes(logPlanificador, elem->colaFinalizados, "Finalizados");
						log_debug(logPlanificador, "El personaje %s finalizó el nivel %s.",pers_actual->simbolo,elem->nivel);
						desecharSocket(descriptor,servidorPlanificador);
						break;
					default:
						break;

				}
			}
			elem->pjEjec=NULL;
		}

		else usleep(segundos);
	}

	return NULL;
}
