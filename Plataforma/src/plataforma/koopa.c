/*
 * koopa.c
 *
 *  Created on: 21/05/2013
 *      Author: utnso
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "commons/config.h"

//

void iniciarKoopa(char* ruta_koopa)
{
	t_config* koopaConfig = config_create(ruta_koopa);

	char* ruta_commons = config_get_string_value(koopaConfig,"ruta_commons");
	char* ruta_memoria = config_get_string_value(koopaConfig,"ruta_memoria");
	int debug = config_get_int_value(koopaConfig,"debug");
	char* ruta_binario = config_get_string_value(koopaConfig,"ruta_binario");
	char* ruta_peticiones = config_get_string_value(koopaConfig,"ruta_peticiones");

	char* argumentos[3];

	argumentos[0] = ruta_binario;
	argumentos[2] = NULL;

	if(debug)							//modo debug
	{
		argumentos[1] = "-debug";
	}

	else								//modo normal
	{
		argumentos[1] = ruta_peticiones;
	}

	if(execv(ruta_binario,argumentos) == -1)
		perror("Error en el execv");

	free(ruta_memoria);
	free(ruta_commons);
	free(ruta_binario);
	free(ruta_peticiones);
}
