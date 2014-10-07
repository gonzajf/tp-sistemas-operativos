/*
 * plataforma.c
 *
 *  Created on: 01/06/2013
 *      Author: utnso
 */


#include <stdlib.h>
#include "pthread.h"
#include <stdint.h>
#include "auxiliar.h"
#include <stdio.h>
#include <stdarg.h>
#include "plataforma.h"
#include "commons/log.h"
#include "variablesGlobales.h"
#include "koopa.h"
#include "commons/log.h"
#include <signal.h>
#include <errno.h>


int main(int argc, char* argv[]){

	pthread_t thread_orquestador;
	direccion=string_duplicate((char*)argv[1]);

	pthread_create(&thread_orquestador, NULL, Orquestador,NULL);

	pthread_join(thread_orquestador,NULL);

	//LLAMAR A KOOPA
	iniciarKoopa();

	return EXIT_SUCCESS;
}
