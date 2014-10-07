/*
 * mensajeria.h
 *
 *  Created on: 04/05/2013
 *      Author: utnso
 */

#ifndef MENSAJERIA_H_
#define MENSAJERIA_H_

#include <stdint.h>

/*Listar los distintos tipos de mensaje que vamos a necesitar*/
typedef enum tipoMensaje
{
	e_desconexion, e_handshake, e_ipPuerto, e_recurso, e_coordenadas, e_nivel, e_movimiento,
	e_finalizoTurno, e_finalizoNivel, e_finalizoPlanNiveles, e_muerte,
	e_procesosInterBlockeados, e_recursosLiberados, e_ok, e_error

}TipoMensaje;

typedef struct mensaje
{
	TipoMensaje tipoMensaje;
	int longitudMensaje;
	char* mensaje;
} __attribute__((packed)) MSJ;

typedef struct {
	uint32_t tipo;
	uint32_t length;
} __attribute__((packed)) t_header ;

char* itoa(int);
//char* compactarMensaje(MSJ*);
//MSJ* descompactarMensaje(char*);

MSJ* crearMensaje();
void liberarMensaje(MSJ* msj);


#endif /* MENSAJERIA_H_ */
