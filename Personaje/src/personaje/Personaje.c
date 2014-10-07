/*
    C ECHO client example using sockets
 */
#include <stdlib.h>
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <signal.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <commons/config.h>
#include <commons/log.h>
#include <stdarg.h>
#include "../sockets.h"
#include <errno.h>

int tieneQueMorirsePorUnaSenial=0;//global
int ganoUnaVidaPorUnaSenial=0;//global

t_socket_cliente* s_niv;
t_socket_cliente* s_orq;
t_socket_cliente* s_planif;

char* string_duplicate(char*);
char** string_split(char*,char*);
void* sleep(int);
int close(int);

int tamanio_array(char* cadena){
	int i=0;
	char** array;

	array=(char**)string_split(cadena, ",");
	while(array[i]!=NULL){
		i++;
	}
	return i;
}


void sigCath (int n) {
	switch (n) {
	case SIGTERM:
		tieneQueMorirsePorUnaSenial++;
		close(s_planif->sock->idSocket);
		break;

	case SIGUSR1:
		ganoUnaVidaPorUnaSenial++;
		break;
	}
}

int procPersonaje(char* path_config)
{   tieneQueMorirsePorUnaSenial=0;
ganoUnaVidaPorUnaSenial=0;
t_config *YO=config_create(path_config);//leo la configuracion
int n=0; //representa el nivel actual
int r=0; //representa el recurso actual
int tengoDestino=0; //para saber si se a donde voy
int posX, posY, faltanEnX, faltanEnY; //posiciones del personaje + las que faltan para el prox recurso.
char* new_pos; //hacia donde me movere.
int llegue=0; // indica si ya llegue al recurso que quiero
int esperoUnRecurso=0; //indica si estoy esperando un recurso.
char* key; //la uso para pasar parametros con concatenacion de strings
char** plan = config_get_array_value(YO, "planDeNiveles"); //vector de niveles
int cantNiveles = tamanio_array(config_get_string_value(YO, "planDeNiveles"));
int vidas=config_get_int_value(YO, "vidas");
char** mision=NULL; //vector de recursos a conseguir en el nivel
char* misionAux;
int cantRecursos;
char* ipPlanif, *ipNiv;
int portPlanif, portNiv;
int deboMorir=0;
char* presentacion;
char* miIp = config_get_string_value(YO, "miIp");
int miPuerto = config_get_int_value(YO, "miPuerto");
char* orq=config_get_string_value(YO, "orquestador");
char* ipOrq=strtok(orq, ":");
int portOrq=atoi(strtok(NULL, ":"));
char* auxPeticionOrq;
char* nom_log= concat(3, "log_", config_get_string_value(YO, "nombre"), ".txt");
char* simbolo = config_get_string_value(YO, "simbolo");
char** datosDelNivel, **coordenadas;

t_log* logger = log_create(nom_log, config_get_string_value(YO, "nombre"),true,LOG_LEVEL_DEBUG);


MSJ* bufPlanif;
MSJ* bufOrq;
MSJ* bufNiv;

//inicializo sockets:

s_orq = crearCliente(miIp, miPuerto, logger);
s_planif= crearCliente(miIp, miPuerto+1, logger);
s_niv= crearCliente(miIp, miPuerto+2, logger);

presentacion=concat(3, config_get_string_value(YO, "nombre"), ":", simbolo);
// ARRANCA TODO.

log_debug(logger, "Cantidad de Niveles: %d",cantNiveles);
log_debug(logger, "Comienzo Plan de Niveles");
for ( n=0; n<cantNiveles;n++){//por cada nivel:
	iniciarNivel://etiqueta
	tengoDestino=0;
	// INICIAR_NIVEL

	s_orq = crearCliente(miIp, miPuerto, logger);
	conectarCliente(s_orq, ipOrq, portOrq, logger);//me conecto con el Orquestador

	repreguntarPorNivel://Etiqueta

	auxPeticionOrq=concat(3, plan[n], ":", config_get_string_value(YO, "simbolo"));
	enviarMensaje(s_orq->sock->idSocket,e_ipPuerto ,auxPeticionOrq, logger);//le pregunto los datos del nivel y su planificador
	free(auxPeticionOrq);
	log_debug(logger,"Pido datos del: %s",plan[n]);

	bufOrq=recibirMsjConOSinSenial( s_orq->sock->idSocket,1, logger);//RECV blockeante esperando los datos del nivel
	if (tieneQueMorirsePorUnaSenial){
		vidas--;
		if(vidas==0){
			log_debug(logger, "Mori y me quede sin vidas. Reinicio Plan de Niveles");
			return 1;/* si retorna 1 se reinicia. si retorna 0 termina!*/}
		else{
			log_debug(logger, "Mori pero sigo con %d vidas. Reinicio el Nivel %s", vidas, plan[n]);
			goto iniciarNivel;
		}
	}

	log_debug(logger,"IpPuertos: %s",bufOrq->mensaje);
	log_debug(logger, "%d",bufOrq->tipoMensaje);

	if (bufOrq->tipoMensaje!=e_ipPuerto){
		if (bufOrq->tipoMensaje==e_error){
			log_debug(logger, "El nivel no esta, pruebo de nuevo en 10 segundos");
			sleep(10);
			goto repreguntarPorNivel;
		}else{
			log_debug(logger, "Llego un msj incorrecto. Pincho.");
			log_destroy(logger);
			return 0;
		}
	}//si fue otro error, pincho!

	close(s_orq->sock->idSocket);//desconecto del orquestador

	datosDelNivel=(char**)string_split(bufOrq->mensaje, ":");
	ipNiv=(char*)string_duplicate(datosDelNivel[0]);
	portNiv=atoi(datosDelNivel[1]);
	ipPlanif=(char*)string_duplicate(datosDelNivel[2]);
	portPlanif=atoi(datosDelNivel[3]);
	free(datosDelNivel);

	s_planif= crearCliente(miIp, miPuerto+1, logger);
	conectarCliente(s_planif, ipPlanif, portPlanif, logger);//me conecto con el Planificador

	enviarMensaje(s_planif->sock->idSocket,e_handshake, presentacion, logger);
	log_debug(logger, "Mando handshake al planif del %s", plan[n]);

	s_niv= crearCliente(miIp, miPuerto+2, logger);
	conectarCliente(s_niv, ipNiv, portNiv, logger);//me conecto con el Nivel
	enviarMensaje(s_niv->sock->idSocket,e_handshake, presentacion, logger);


	posX=0;
	posY=0;
	key= concat(3, "obj[", plan[n], "]");

	mision = config_get_array_value(YO, key);
	misionAux = config_get_string_value(YO, key);
	cantRecursos = tamanio_array(misionAux);
	for (r=0; r<cantRecursos;r++){//por cada recurso.
		llegue=0;
		esperoUnRecurso=0;
		while(!llegue) {
			//---------------------------------ESCUCHA ATENTO A SEÑALES--------------------------------------
			reEscucharPrimero:
			bufPlanif=recibirMsjConOSinSenial( s_planif->sock->idSocket, 1, logger);// RECV BLOCKEANTE ESPERANDO TURNO DEL PLANIFICADOR

			if (bufPlanif->tipoMensaje==-1) {
				if (errno == EINTR){ //si fue por una senial..
					errno=0;
					if (!tieneQueMorirsePorUnaSenial){
						goto reEscucharPrimero;
					}//si no tengo q morir, vuelvo a escuchar..
				}
			}

			// ---------------------------------YA ESCUCHO LO QUE QUERIA--------------------------------------
			if (bufPlanif->tipoMensaje==e_muerte){
				deboMorir=1;
				log_debug(logger, "Tiene que morirse por un mensaje del orquestador");
			}

			//--------------CHEKEO SI DEBO MORIR, pero antes si debo sumar una vida--------------
			if (ganoUnaVidaPorUnaSenial){	ganoUnaVidaPorUnaSenial--; vidas++;log_debug(logger, "Gano una vida por una senial. Me quedan %d vidas", vidas);	 }
			if (tieneQueMorirsePorUnaSenial){ tieneQueMorirsePorUnaSenial--; deboMorir=1; log_debug(logger, "Tiene que morirse por una senial.");	}

			if (deboMorir){
				deboMorir=0;
				vidas--;
				close(s_niv->sock->idSocket);//me desconecto del nivel
				close(s_planif->sock->idSocket);//me desconecto del planificador
				if(vidas==0){
					log_debug(logger, "Mori y me quede sin vidas. Reinicio Plan de Niveles");
					return 1;/* si retorna 1 se reinicia. si retorna 0 termina!*/}
				else{
					log_debug(logger, "Mori pero sigo con %d vidas. Reinicio el Nivel %s", vidas, plan[n]);
					goto iniciarNivel;
				}
			}
			//------------------------------------------------------------------------------------

			if (bufPlanif->tipoMensaje==-1){
				log_debug(logger, "Se perdió la coneccion con el nivel.");
				goto iniciarNivel;
			}else if (bufPlanif->tipoMensaje!=e_movimiento){
				log_debug(logger, "Llego un msj incorrecto");
				log_destroy(logger);
				return 0;
			}
			log_debug(logger, "Me toca!");

			if (!tengoDestino){
				log_debug(logger, "Pido coordenadas del recurso!");
				enviarMensaje(s_niv->sock->idSocket,e_coordenadas,mision[r], logger);//PIDO UBICAION AL NIVEL
				bufNiv = recibirMensaje( s_niv->sock->idSocket, logger);//ESPERO RESPUESTA
				if (bufNiv->tipoMensaje!=e_coordenadas){
					log_debug(logger, "Hubo un Error al recibir las coordenadas.");
					log_destroy(logger);
					return 0;
				}
				//calculo lo q falta moverme
				coordenadas=(char**)string_split(bufNiv->mensaje, ":");
				faltanEnX= atoi(coordenadas[0])-posX;
				faltanEnY= atoi(coordenadas[1])-posY;
				free(coordenadas);
				tengoDestino=1;
			}

			if (faltanEnX){ //en este IF decido hacia donde moverme
				if (faltanEnX>0){
					faltanEnX--;
					posX++;}
				else{
					faltanEnX++;
					posX--;
				}}
			else{
				if (faltanEnY>0){
					faltanEnY--;
					posY++;}
				else{
					faltanEnY++;
					posY--;
				}
			}
			new_pos=concat(5, simbolo, ",", (char*)itoa(posX), ",", (char*)itoa(posY));
			log_debug(logger, "Aviso movimiento al nivel");
			enviarMensaje(s_niv->sock->idSocket,e_movimiento, new_pos, logger);//AVISO MOVIMIENTO AL NIVEL
			free(new_pos);
			free(bufNiv);
			bufNiv = recibirMensaje( s_niv->sock->idSocket, logger);//ESPERO RESPUESTA

			if ((!faltanEnX) && (!faltanEnY)){//si no falta nada YA LLEGUE!
				llegue=1;
				tengoDestino=0;
				log_debug(logger, "Pido instancia de %s",mision[r]);
				enviarMensaje(s_niv->sock->idSocket,e_recurso,mision[r], logger);//PIDO RECURSO AL NIVEL

				bufNiv = recibirMensaje( s_niv->sock->idSocket, logger);//ESPERO RESPUESTA

				switch (bufNiv->tipoMensaje){
				case e_ok:
					log_debug(logger, "Aviso al planif q termine con recurso");
					enviarMensaje(s_planif->sock->idSocket,e_finalizoTurno,"", logger);//aviso al planificardor que termine mi turno normal.
					break;
				case e_error:
					log_debug(logger, "Aviso al planif q termine blockeado");
					enviarMensaje(s_planif->sock->idSocket,e_finalizoTurno, mision[r], logger);//aviso al planificardor que termine mi turno bloqueado esperando un "mision[r]".
					esperoUnRecurso=1;
					break;
				default:
					log_debug(logger, "llego un msj incorrecto");
					log_destroy(logger);
					return 0;
					break;
				}
			}
			else{
				log_debug(logger, "Aviso al planif q termine normal");
				enviarMensaje(s_planif->sock->idSocket,e_movimiento,"", logger);//aviso al planificardor que termine mi turno normal.
			}

		}//fin del while !llegue
	}//fin de cada recurso

	if (esperoUnRecurso){
		//---------------------------------ESCUCHA ATENTO A SEÑALES--------------------------------------
		reEscucharSegundo:
		bufPlanif=recibirMsjConOSinSenial( s_planif->sock->idSocket, 1, logger);// RECV BLOCKEANTE ESPERANDO TURNO DEL PLANIFICADOR

		if (bufPlanif->tipoMensaje==-1) {
			if (errno == EINTR){ //si fue por una senial..
				errno=0;
				if (!tieneQueMorirsePorUnaSenial){
					goto reEscucharSegundo;
				}//si no tengo q morir, vuelvo a escuchar..
			}
		}

		// ---------------------------------YA ESCUCHO LO QUE QUERIA--------------------------------------
		if (bufPlanif->tipoMensaje==e_muerte){
			deboMorir=1;
			log_debug(logger, "Tiene que morirse por un mensaje del orquestador");
		}

		//--------------CHEKEO SI DEBO MORIR, pero antes si debo sumar una vida--------------
		if (ganoUnaVidaPorUnaSenial){	ganoUnaVidaPorUnaSenial--; vidas++;log_debug(logger, "Gano una vida por una senial. Me quedan %d vidas", vidas);	 }
		if (tieneQueMorirsePorUnaSenial){ tieneQueMorirsePorUnaSenial--; deboMorir=1; log_debug(logger, "Tiene que morirse por una senial.");	}

		if (deboMorir){
			deboMorir=0;
			vidas--;
			close(s_niv->sock->idSocket);//me desconecto del nivel
			close(s_planif->sock->idSocket);//me desconecto del planificador
			if(vidas==0){
				log_debug(logger, "Mori y me quede sin vidas. Reinicio Plan de Niveles");
				return 1;/* si retorna 1 se reinicia. si retorna 0 termina!*/}
			else{
				log_debug(logger, "Mori pero sigo con %d vidas. Reinicio el Nivel %s", vidas, plan[n]);
				goto iniciarNivel;
			}
		}
		//------------------------------------------------------------------------------------


		if (bufPlanif->tipoMensaje==-1){
			log_debug(logger, "Se perdió la coneccion con el nivel.");
			goto iniciarNivel;
		}else if (bufPlanif->tipoMensaje!=e_movimiento){
			log_debug(logger, "Llego un msj incorrecto");
			log_destroy(logger);
			return 0;
		}//me dieron el ultimo recurso!
		//(sino fuera el ultimo no habria podido salir del for.
		//(no codifico nada) simplemente paso de iteracion en el for al prox nivel
	}

	enviarMsjConOSinSenial(s_niv->sock->idSocket,e_finalizoNivel,"",1, logger);//aviso al nivel que termine
	close(s_niv->sock->idSocket);//me desconecto del nivel
	enviarMsjConOSinSenial(s_planif->sock->idSocket,e_finalizoNivel,"",1, logger);//aviso al planif que termine
	close(s_planif->sock->idSocket);//me desconecto del planificador
	log_debug(logger, "TERMINO NIVEL %s", plan[n]);
	free(mision);
}//fin de cada nivel

// SUPER KOOPA!!!!!!!!!!!!!!
//me conecto al orquestador y le comunico que termine mi plan de niveles.
s_orq = crearCliente(miIp, miPuerto, logger);
conectarCliente(s_orq, ipOrq, portOrq, logger);//me conecto con el Orquestador
enviarMsjConOSinSenial(s_orq->sock->idSocket,e_finalizoPlanNiveles,"",1, logger);

close(s_orq->sock->idSocket);//me desconecto del Orquestador
log_debug(logger, "------------------%s TERMINO PLAN DE NIVELES--------------",config_get_string_value(YO, "nombre"));
config_destroy(YO);
log_destroy(logger);
return 0;
}/*fin main*/

int main(int argc, char* argv[]) {
	struct sigaction act;
	act.sa_sigaction = (void*)sigCath;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGUSR1, &act, NULL);
	int reStart=1;
	while(reStart){//cuando se muere y se queda sin vidas retorna un 1 y se reinicia, si devuelve 0 es q termino todo OK.
		reStart=procPersonaje((char*)argv[1]);
	}
	return 0;
}
