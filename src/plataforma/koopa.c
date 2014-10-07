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

#define RUTA_CONFIG "/home/utnso/workspace/Plataforma/marioBross/src/plataforma/koopa_config.txt"
//

void iniciarKoopa()
{
	t_config* koopaConfig = config_create(RUTA_CONFIG);

	char* ruta_commons = config_get_string_value(koopaConfig,"ruta_commons");
	char* ruta_memoria = config_get_string_value(koopaConfig,"ruta_memoria");
	int debug = config_get_int_value(koopaConfig,"debug");
	char* ruta_binario = config_get_string_value(koopaConfig,"ruta_binario");
	char* ruta_peticiones = config_get_string_value(koopaConfig,"ruta_peticiones");

	char* comando = (char*)malloc(strlen("export LD_LIBRARY_PATH=") + strlen(ruta_commons) + strlen(":") + strlen(ruta_memoria) + 1);
	char* argumentos[3];

	strcpy(comando,"export LD_LIBRARY_PATH=");
	strcat(comando,ruta_memoria);
	strcat(comando,":");
	strcat(comando,ruta_commons);

	system(comando);

	free(comando);

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
