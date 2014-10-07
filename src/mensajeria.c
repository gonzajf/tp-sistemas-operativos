/*
 * mensajeria.c
 *
 *  Created on: 20/04/2013
 *      Author: utnso
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mensajeria.h"
#include <commons/string.h>
#include <stdint.h>

char* itoa(int valor)
{
	char* string = malloc(sizeof(valor));
	int valorRetorno;

	valorRetorno = snprintf(string,32,"%d",valor);

	if(valorRetorno == -1) return("-1");

	return(string);
}

//char* compactarMensaje(MSJ* mensaje)
//{
//	return string_from_format("%d|%d|%s", mensaje->tipoMensaje, mensaje->longitudMensaje, mensaje->mensaje);
//}
//
//MSJ* descompactarMensaje(char* paquete)
//{
//    MSJ* msj=crear_memoria();
//    char** vectorCompactado=string_split(paquete, "|");
//    msj->tipoMensaje = atoi(vectorCompactado[0]);
//    msj->longitudMensaje = atoi(vectorCompactado[1]);
//
//	if(vectorCompactado[2] == NULL) {
//		msj->mensaje = string_new();
//	}
//	else {
//		msj->mensaje = string_duplicate(vectorCompactado[2]);
//	}
//
//	free(vectorCompactado);
//
//	return(msj);
//}


MSJ* crearMensaje(){
	MSJ* msj = (MSJ*)malloc(sizeof(struct mensaje));
	msj->longitudMensaje=0;
	msj->tipoMensaje=-1;
	msj->mensaje=string_new();

	return msj;
}

void liberarMensaje(MSJ* msj){
	free(msj->mensaje);
	free(msj);
}
