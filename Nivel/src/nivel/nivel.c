/*
 * nivel.c
 *
 *  Created on: 03/05/2013
 *      Author: JuampiMercado
 */

#include "nivel.h"
#include "Gui/tad_items.h"
#include <stdlib.h>
#include <curses.h>

char** string_split(char*,char*);

t_recursosDelPersonaje* crearPersonaje(){
	t_recursosDelPersonaje* elemento=(t_recursosDelPersonaje*)malloc(sizeof(t_recursosDelPersonaje));
	return elemento;
}

t_recursosDelPersonaje* buscoPersonajePorSimbolo(t_list* unaLista,char* simbolo){
	int i=0;
	t_recursosDelPersonaje* elemento=(t_recursosDelPersonaje*)malloc(sizeof(t_recursosDelPersonaje));

	for (i = 0; i < (list_size(unaLista)); ++i) {
		elemento=list_get(unaLista, i);
		if(simbolo[0]==elemento->simbolo[0]){
			log_debug(logger,"encontre el personaje: %s\n",elemento->simbolo);
			break;
			}
		}

	return elemento;
}

int buscarIndiceDePersonaje(t_list* unaLista,t_recursosDelPersonaje* unPersonaje){
	int indice;
	t_recursosDelPersonaje* elemento=(t_recursosDelPersonaje*)malloc(sizeof(t_recursosDelPersonaje));

	for (indice = 0; indice < (list_size(unaLista)); ++indice) {
		elemento=list_get(unaLista, indice);		//Busco personaje
		if(elemento->simbolo[0]==unPersonaje->simbolo[0]){
			break;
		}
	}

	return indice;
}


t_recursosDelPersonaje* encontrarUnPersonaje(t_list* unaLista, int socket){

	int i=0;
	t_recursosDelPersonaje* elemento=(t_recursosDelPersonaje*)malloc(sizeof(t_recursosDelPersonaje));

	for (i = 0; i < (list_size(unaLista)); ++i) {
		elemento=list_get(unaLista, i);
		if(socket==elemento->socket){
			log_debug(logger,"encontre el personaje: %s\n",elemento->simbolo);
			break;
			}
		}

	return elemento;
}






void* chequeoInterbloqueo(){
	t_temporal Vtemporal[10];
	t_recursosDelPersonaje* pj;
	t_list* indexAux,*listaInterbloqueados,* listaAuxiliar;
	MSJ* recibido=NULL;
	char* mensajeConPjsBloqueados=(char*) malloc(sizeof(char));
	int i,j,indice;
	char* recurso;
	t_config* configThread=config_create(path_config);
	int tiempoChequeoDeadlock=config_get_int_value(configThread, "TiempoChequeoDeadlock");
	int recovery=config_get_int_value(configThread, "Recovery");
	char* nivel=config_get_string_value(configThread, "Nombre");


	while(1){			//Se va a terminar cuando cierre el proceso


		sleep(tiempoChequeoDeadlock);

		if(list_size(personajesBloqueados->personajesBloqueados)>1){

			log_debug(logger,"(Hilo)Hay personajes bloqueados");

			indexAux=list_create();
			listaAuxiliar=list_create();

			listaInterbloqueados=list_create();
			pj=crearPersonaje();


			//Armo una nueva lista auxiliar con personajesBloqueados
			pthread_mutex_lock(&(personajesBloqueados->acceso));
			listaAuxiliar=list_take(personajesBloqueados->personajesBloqueados, personajesBloqueados->personajesBloqueados->elements_count);
			pthread_mutex_unlock(&(personajesBloqueados->acceso));

			log_debug(logger,"(Hilo)Creo la cola auxiliar con %i personajes\n",list_size(listaAuxiliar));

			log_debug(logger,"(Hilo)Asigno 1 a los procesos que tengan RecursosAsignados en NULL");

			indice=0;
			while(indice<list_size(listaAuxiliar)){					//Asigno 1 a los procesos que tengan RecursosAsignados en NULL
				pj=list_get(listaAuxiliar,indice);
				if(pj->recursosAsignados->elements_count==0){
					pj->Interbloqueo=1;
				}
				else pj->Interbloqueo=0;
				indice++;
			}



			for(i=0;i<10;i++){				//Inicio el vector Temporal
				Vtemporal[i].inicial=contenidoNivel[i].inicial;
				Vtemporal[i].instancias=contenidoNivel[i].instancias;
			}
			log_debug(logger,"(Hilo)Cargue el vector temporal");




			while(1){


				for(indice=0;indice<listaAuxiliar->elements_count;indice++){			//Busco al personaje que tenga interbloqueo = 0
					pj=list_get(listaAuxiliar,indice);
					if(pj->Interbloqueo==0){
						break;
					}
				}

				if(indice<list_size(listaAuxiliar)){
					log_debug(logger,"(Hilo) Saque un personaje con interbloqueo 0");
					i=0;
					while(Vtemporal[i].inicial[0]!= pj->recursoSolicitado[0]){	//Busco el recurso en el vector
						i++;
					}
					log_debug(logger,"(Hilo)Veo si hay recursos en el Vtemporal");

					if(Vtemporal[i].instancias>0){				//Si existe tal personaje y hay instancias:

						log_debug(logger,"(Hilo)Hay entonces pongo al personaje en 1");

						pj->Interbloqueo=1;				//Pongo en 1 el interbloqueo
						indexAux=pj->recursosAsignados;
						for(i=0;i<list_size(indexAux);i++){
							recurso=list_get(indexAux,i);
							j=0;
							while(Vtemporal[j].inicial[0]!= recurso[0]){			//Busco cual es el recurso
								j++;
							}
							Vtemporal[j].instancias=Vtemporal[j].instancias +1;			//le sumo un recurso
							log_debug(logger,"(Hilo)Sumo instancias en el Vtemporal");
						}

					}
					else break;		//Si el recurso en el vector temporal no es mayor a 0, entonces salgo
				}
				else break;			//Si no existe el personaje con interbloqueo =0 salgo
			}



			log_debug(logger,"(Hilo)Sali del algoritmo");
			indice=0;
			while(indice<list_size(listaAuxiliar)){					//Si el personaje tiene interbloqueo 0, lo meto en la cola de interbloqueados
				pj=list_get(listaAuxiliar,indice);
				if(pj->Interbloqueo==0){
					list_add(listaInterbloqueados,pj);
				}
				indice++;
			}



			if(list_size(listaInterbloqueados)!=0){
				mensajeConPjsBloqueados=nivel;
				for(i=0;i<list_size(listaInterbloqueados);i++){
					pj=list_get(listaInterbloqueados,i);
					mensajeConPjsBloqueados=concat(3,mensajeConPjsBloqueados,":",pj->simbolo);		//Armo un mensaje con simbolos
				}
				log_debug(logger,"(Hilo)El mensaje con personajes interbloqueados es: %s",mensajeConPjsBloqueados);
			}


			if(recovery==1){			//Si el recovery esta activado(=1) entonces debo avisar al orquestador del interbloqueo

				log_debug(logger,"(Hilo)Esta encendido el recovery");

				if(list_size(listaInterbloqueados)>1){		//Quiere decir que hay pjs interbloqueados

					log_debug(logger,"(Hilo)Hay personajes interbloqueados, voy a mandar el mensaje al orquestador");

					enviarMensaje(idSocketThread,e_procesosInterBlockeados,mensajeConPjsBloqueados,logger);	//Mando al orquestador quienes son los personajes interbloqueados
					log_debug(logger,"(Hilo)Envie procesos interbloqueados");

					recibido=recibirMensaje(idSocketThread,logger);

					if(recibido->tipoMensaje==e_muerte) log_debug(logger,"(Hilo)El orquestador va a matar a %s\n",recibido->mensaje);
					else log_debug(logger,"(Hilo)Error al recibir personajes que va a matar el orquestador");
				}

				else log_debug(logger,"(Hilo)Pero no hay personajes interbloqueados");

			}



		}//Del if personajesBloqueados>0

	}//Del while

		list_destroy_and_destroy_elements(listaAuxiliar,free);
		list_destroy_and_destroy_elements(indexAux,free);
		list_destroy_and_destroy_elements(listaInterbloqueados,free);
		free(mensajeConPjsBloqueados);
		config_destroy(configThread);

	return 0;
}//Del thread









int main(int argc, char* argv[]){
	t_socket_servidor* nivel_servidor;
	t_socket_cliente* nivel_cliente;
	MSJ* mensajeRecibido, *mensajeOrquestador;
	t_recursosDelPersonaje* personaje,*personajeBloqueado;
	t_list* listaDeConectados;

	path_config=argv[1];
	t_config* configNivel=config_create(path_config);
	char* nombre=config_get_string_value(configNivel, "Nombre");
	char* orquestador=config_get_string_value(configNivel,"orquestador");
	char** ipYpuertoOrquestador=string_split(orquestador,":");
	char* ip_orquestador= ipYpuertoOrquestador[0];
	int puerto_orquestador=atoi(ipYpuertoOrquestador[1]);
	int cantidadCajas=config_get_int_value(configNivel, "cantidadCajas");
	char* ip_nivel=config_get_string_value(configNivel,"ipNivel");
	int puerto_nivel_servidor=config_get_int_value(configNivel, "puertoServidor");
	int puerto_nivel_cliente=config_get_int_value(configNivel, "puertoCliente");
	char** movimiento=string_split("@,XX,YY",",");

	char** recursoNivel;
	char** recursosQueAsignoElOrquestador;
	char* ipYpuertoNivel="";
	short int coneccion;
	int actividadSocket=-1;
	char* cajax="";
	char* recurso="";
	int i,j;
	short int personajesConectados;
	int posicionX,posicionY;
	char* posXY="";
	char* mensajeConRecursosQueSeLiberan="";
	int indice;
	char* num="";
	int	numero;
	char** elMensaje=NULL;
	char* nombrelog;
	int rows,cols;

	pthread_t thread_chequeoInterbloqueo;

//----Creo el log------------------------------
	nombrelog=concat(3,"log",nombre,".txt");
	logger = log_create(nombrelog,nombre,false,LOG_LEVEL_DEBUG);



//----Inicializo la GUI del nivel-------------

	ITEM_NIVEL* ListaItems = NULL;
	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

//------------Inicializo el vector que contiene los datos de los recursos del nivel
	for(i=0;i<cantidadCajas;i++){
		contenidoNivel[i].inicial="";
		contenidoNivel[i].instancias=0;
		contenidoNivel[i].posX=0;
		contenidoNivel[i].posY=0;
	}


//-----Cargo el vector con los datos del archivo de config-------------
	for(i=0;i<cantidadCajas;i++){
		num=itoa(i+1);
		cajax= concat(2,"Caja",(char*)num);
		recursoNivel=config_get_array_value(configNivel,cajax);
		contenidoNivel[i].inicial=recursoNivel[1];
		numero=atoi(recursoNivel[2]);
		contenidoNivel[i].instancias=numero;
		contenidoNivel[i].posX=atoi(recursoNivel[3]);
		contenidoNivel[i].posY=atoi(recursoNivel[4]);
		CrearCaja(&ListaItems, contenidoNivel[i].inicial[0], contenidoNivel[i].posX, contenidoNivel[i].posY, numero);
	}


	for(i=0;i<cantidadCajas;i++){
		log_debug(logger,"Recurso: %s instancias: %d PosX: %d PosY: %d\n",contenidoNivel[i].inicial,contenidoNivel[i].instancias,contenidoNivel[i].posX,contenidoNivel[i].posY);
	}



	nivel_gui_dibujar(ListaItems);

//-------Creo el socket cliente y me conecto al orquestador---------------------

	nivel_cliente=crearCliente(ip_nivel,puerto_nivel_cliente,logger);
	idSocketThread=nivel_cliente->sock->idSocket;
	log_debug(logger,ip_orquestador);
	log_debug(logger,"%d\n",puerto_orquestador);
	coneccion=1;
	if(coneccion==1){
		if(conectarCliente(nivel_cliente,ip_orquestador,puerto_orquestador,logger)==-1){
				log_debug(logger,"No me conecte\n");
		}
		else 	log_debug(logger,"Me conecte");
	}



	char* strpo=itoa(puerto_nivel_servidor);
	ipYpuertoNivel=concat(5,nombre,":",ip_nivel,":",strpo);
	if(enviarMensaje(nivel_cliente->sock->idSocket,e_handshake,ipYpuertoNivel,logger)==-1){
		log_debug(logger,"Fallo envio. Reintentando..");
	}
	else {
		log_debug(logger,"Le mande:%s\n",ipYpuertoNivel);
/*		mensajeRecibido=recibirMensaje(nivel_cliente->sock->idSocket,logger);			hay que ponernos de acuerdo Pau si me mandas o no respuesta al handshake
		log_debug(logger,"Che, recibi esto:");
		log_debug(logger,recibido->mensaje);*/
	}

	/*avise al orquestador mi nombre, mi ip y mi puerto de servidor*/


	//----------------------------------------------------------------------------------



	//Logica de conexion de personajes

	nivel_servidor= crearServidor(ip_nivel,puerto_nivel_servidor,logger);	//Creo el servidor
	log_debug(logger,"Esperando que se conecte un personaje..");	//Y espero que se conecten

	multiplexarClientes(nivel_servidor,logger);				// Va a ser siempre una conexion, porque no hay personajes conectados hasta ahora


	listaDeConectados=list_create();
	personajesBloqueados=(tipoLista*)malloc(sizeof(tipoLista));
	personajesBloqueados->personajesBloqueados=list_create();
	pthread_mutex_init(&(personajesBloqueados->acceso),NULL);




	pthread_create(&thread_chequeoInterbloqueo, NULL, chequeoInterbloqueo,NULL);	//Lanzo el hilo

	personajesConectados=1;

	while(personajesConectados!=-1){


		actividadSocket=multiplexarClientes(nivel_servidor,logger);


		if(actividadSocket!=0){						//Pregunto si lo que recibi fue un mensaje

			mensajeRecibido=recibirMensaje(actividadSocket,logger);

			switch(mensajeRecibido->tipoMensaje){

//--------Peticion de coordenadas--------------------------------------------


			case e_coordenadas:		/*	pregunto si me pide recursos*/

				log_debug(logger,"Me piden coordenadas");

				i=0;
				while(mensajeRecibido->mensaje[0]!=contenidoNivel[i].inicial[0]){		//Busco las coordenadas en el array
					i++;
				}

				posicionX= contenidoNivel[i].posX;
				posicionY= contenidoNivel[i].posY;
				if(posicionX<MAX_X && posicionY< MAX_Y){			//Me fijo si las coordenadas no superan los limites maximos
					posXY= concat(3,itoa(posicionX),":",itoa(posicionY));
					log_debug(logger,posXY);
					enviarMensaje(actividadSocket, e_coordenadas,posXY,logger);	//Mando mensaje con las coordenadas al personaje
				}

				personaje=encontrarUnPersonaje(listaDeConectados, actividadSocket);	//Busco al personaje en la cola de conectados

				personaje->recursoSolicitado=mensajeRecibido->mensaje;	//le asigno que recurso busca actualmente
				log_debug(logger,"Mande coordenadas al personaje: %s del recurso:%s\n",personaje->simbolo,mensajeRecibido->mensaje);

			break;








//--------------------Peticion de recurso------------------------------------------------

			case e_recurso:		//El personaje me pide instancia de recurso
				log_debug(logger,"Me pide una instancia de recurso");
				log_debug(logger,"En el array HAY:");
				for(i=0;i<cantidadCajas;i++){
					log_debug(logger,"Recurso:%s instancias:%d\n",contenidoNivel[i].inicial,contenidoNivel[i].instancias);
				}

				recurso=mensajeRecibido->mensaje;
				i=0;
				while(recurso[0]!= contenidoNivel[i].inicial[0]){	//Recorro el array hasta que caigo en el recurso solicitado
					i++;
				}



				personaje=encontrarUnPersonaje(listaDeConectados,actividadSocket);		//Busco al personaje en la cola de conectados

				if(contenidoNivel[i].instancias!=0){
					enviarMensaje(actividadSocket,e_ok,"",logger);
					contenidoNivel[i].instancias=contenidoNivel[i].instancias-1;		//Si hay instancias doy el okey y saco una instancia de ese recurso
					restarRecurso(ListaItems, contenidoNivel[i].inicial[0]);
					nivel_gui_dibujar(ListaItems);
					list_add(personaje->recursosAsignados,recurso);	//pongo el recurso en la cola de recursos asignados al personaje
					personaje->recursoSolicitado="";		//ahora desconozco cual es el proximo recurso que el personaje necesita
				}
				else {
					log_debug(logger,"Hay un bloqueo");
					personajeBloqueado=crearPersonaje();
					personajeBloqueado=personaje;
					personaje->bloqueado=1;
					personajeBloqueado->bloqueado=1;
					enviarMensaje(actividadSocket,e_error,"",logger);	/*le digo que hay un bloqueo con este personaje*/

					pthread_mutex_lock(&(personajesBloqueados->acceso));
					list_add(personajesBloqueados->personajesBloqueados,personajeBloqueado);		//mando a la cola de personajes bloqueados esperando ese recurso
					pthread_mutex_unlock(&(personajesBloqueados->acceso));


					log_debug(logger,"Hay %i personajes bloqueados\n",list_size(personajesBloqueados->personajesBloqueados));

				}


				log_debug(logger,"En el array quedan:");
				for(i=0;i<cantidadCajas;i++){
					log_debug(logger,"Recurso:%s instancias:%d\n",contenidoNivel[i].inicial,contenidoNivel[i].instancias);
				}
			break;


//----------HANDSHAKE-------------------------------------------------------

			case e_handshake:		//el personaje manda su primer mensaje presentandose
				log_debug(logger,"Llego un handshake");
				personajesConectados++;
				personaje=crearPersonaje();
				elMensaje=string_split(mensajeRecibido->mensaje,":"); //Aca me separa el Mario:@
				personaje->simbolo=elMensaje[1];	//Meto el simbolo
				personaje->Interbloqueo=0;
				personaje->bloqueado=0;
				personaje->socket=actividadSocket;
				personaje->recursoSolicitado="";
				personaje->recursosAsignados=list_create();
				log_debug(logger,personaje->simbolo);										//le agrego el simbolo del personaje
				list_add(listaDeConectados,personaje);
				CrearPersonaje(&ListaItems, personaje->simbolo[0], 0, 0);
				nivel_gui_dibujar(ListaItems);

			break;

//----------Desconexion-------------------------------------

			case e_desconexion: case e_finalizoNivel:		//el personaje manda su primer mensaje presentandose

								if(mensajeRecibido->tipoMensaje==e_desconexion)	log_debug(logger,"Desconexion repentina");
								else log_debug(logger,"Finalizo el nivel un personaje");


							//-----Lo encuentro y lo saco de la lista de conectados-------------------------------------
									for (indice = 0; indice < (list_size(listaDeConectados)); ++indice) {
										personaje=list_get(listaDeConectados,indice);		//Busco personaje
										if(actividadSocket==personaje->socket){
											break;
											}
										}

									personaje=list_remove(listaDeConectados,indice);
									BorrarItem(&ListaItems, personaje->simbolo[0]);
									nivel_gui_dibujar(ListaItems);

							//-----Si esta en la cola de bloqueados tambien lo saco de ahi-------------------------------
									if(personaje->bloqueado==1){
										for (indice = 0; indice < (list_size(personajesBloqueados->personajesBloqueados)); ++indice) {
											personaje=list_get(personajesBloqueados->personajesBloqueados,indice);	//Busco personaje
											if(actividadSocket==personaje->socket){
												break;
												}
											}
										pthread_mutex_lock(&(personajesBloqueados->acceso));
										list_remove(personajesBloqueados->personajesBloqueados,indice);
										pthread_mutex_unlock(&(personajesBloqueados->acceso));

									}

			//HAY QUE VER SI FUNCIONA DESDE ACA.	(Lo agregue junto con la correccion al hilo de chequeo de interbloqueo)

						//----Armo un mensaje con los recursos que se liberan al orquestador y actualizo el vector de disponibles------------------------
									log_debug(logger,personaje->simbolo);
									mensajeConRecursosQueSeLiberan=concat(3,nombre,":",personaje->simbolo);
									while(1<=list_size(personaje->recursosAsignados)){
										recurso=list_remove(personaje->recursosAsignados,0);
										i=0;
										while(recurso[0]!= contenidoNivel[i].inicial[0]){
												i++;
										}
										contenidoNivel[i].instancias=contenidoNivel[i].instancias+1;
										sumarRecurso(ListaItems, contenidoNivel[i].inicial[0]);

										mensajeConRecursosQueSeLiberan=concat(3,mensajeConRecursosQueSeLiberan,":",recurso);
									}
									log_debug(logger,"Voy a enviar mensajes que se liberaron al orquestador");
									enviarMsjConOSinSenial(idSocketThread,e_recursosLiberados,mensajeConRecursosQueSeLiberan, 1,logger);
						//--El orquestador me dice a quien asigno que recursos-----------
									log_debug(logger,"Envie al orquestador los mensajes que se liberaron: %s\n",mensajeConRecursosQueSeLiberan);
									mensajeOrquestador=recibirMsjConOSinSenial(idSocketThread, 1,logger);
									log_debug(logger,"Respuesta del orquestador: %s\n",mensajeOrquestador->mensaje);
						//--Si longitud es !=0 es porque asigno recursos a algun personaje, sino no asigno nada a nadie-------------------------------


									if(mensajeOrquestador->longitudMensaje!=0){

										recursosQueAsignoElOrquestador=string_split(mensajeOrquestador->mensaje,":");
										i=0;
										log_debug(logger,"El orquestador asigno recursos a personajes");
						//--Actualizo personajes y vector de disponibles segun las asignaciones del orquestador----------------------------------------
										while(recursosQueAsignoElOrquestador[i]!=NULL){
											personaje=buscoPersonajePorSimbolo(personajesBloqueados->personajesBloqueados,recursosQueAsignoElOrquestador[i]);	//El primer i va a ser el personaje
											i++;
											recurso=recursosQueAsignoElOrquestador[i];		//El segundo i va a ser el recurso que se le asigno
											list_add(personaje->recursosAsignados,recurso);
											log_debug(logger,"Voy a desbloquear los personajes a los que les asigno esos recursos");
											personaje->bloqueado=0;
											i++;	//Los hago rusticos asi los i++ para no complicarme al pedo
											j=0;
											while(recurso[0]!= contenidoNivel[j].inicial[0]){
													j++;											//Lo busco en el vector de disponibles
											}
											contenidoNivel[j].instancias=contenidoNivel[j].instancias-1;		//Le saco una instancia
											restarRecurso(ListaItems, contenidoNivel[j].inicial[0]);
											log_debug(logger,"Desbloqueo segun orquestador a %s y le doy el recurso %s\n",personaje->simbolo,recurso);
											indice=buscarIndiceDePersonaje(personajesBloqueados->personajesBloqueados,personaje);	//Lo saco de la cola de bloqueados

											pthread_mutex_lock(&(personajesBloqueados->acceso));
											personaje=list_remove(personajesBloqueados->personajesBloqueados,indice);
											pthread_mutex_unlock(&(personajesBloqueados->acceso));

											log_debug(logger,"Acabo dedesbloquear a: %s",personaje->simbolo);
										}

									}

			//HASTA ACA------

									desecharSocket(actividadSocket,nivel_servidor);
									personajesConectados--;
									nivel_gui_dibujar(ListaItems);
									log_debug(logger,"En la lista de conectados quedan: %i personajes\n",listaDeConectados->elements_count);
									log_debug(logger,"Quedan %i personajes bloqueados\n",list_size(personajesBloqueados->personajesBloqueados));


			break;



			case e_movimiento:
				enviarMensaje(actividadSocket,e_ok,"",logger);
				movimiento=string_split(mensajeRecibido->mensaje,",");
				MoverPersonaje(ListaItems, movimiento[0][0], atoi(movimiento[1]), atoi(movimiento[2]));
				nivel_gui_dibujar(ListaItems);
				log_debug(logger,mensajeRecibido->mensaje);
			break;


			default: break;

		//Hasta aca son todos los mensajes con actividadSocket!=0

			}//Llave del switch
		}//llave del if



		else 	log_debug(logger,"Hay una conexion nueva");		//Si actividadSocket no es mayor a 0


	}//Llave del while

	log_destroy(logger);

	nivel_gui_terminar();

	return 0;

}//Se termino




//e_procesosInterblockeados			"Nivel11:@:$:#"	esto le mando
//e_muerte						" @"		esto me manda

