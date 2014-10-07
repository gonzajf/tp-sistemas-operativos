/*
 ============================================================================
 Name        : Super.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <stdarg.h>


char* concat2(int cant, ...){
	int sum = 0, cont = 0;
	char* plus;
	va_list marker;
    if (cant<1) {return 0;}
	va_start( marker, cant );
		while( cont<cant )
	   {
		  plus = va_arg( marker, char*);
	      sum += strlen(plus);
	      cont++;
	   }
	cont=0;
    char* aux ="";
	aux = malloc(sum+1); /* make space for the new string (should check the return value ...) */
	strcpy(aux, "");
	va_start( marker, cant );
	while( cont<cant )
	 {
		plus = va_arg( marker, char*);
		cont++;
		strcat(aux, plus); /* copy name into the new var */
	   }
	va_end( marker );
	return aux;
}
void SepararCadenas(char * cadena, char caracter, char * primera, char * segunda)
{
	while((*primera++=*cadena++)!=caracter);
	while((*segunda++=*cadena++)!=0);
	*(primera-1)=0;
}
int main7898412() {
	int i;
	int j;
	char** plan;
	char* key = "planDeNiveles";
	t_config *YO=config_create("/home/utnso/git/tp-20131c-oiram-repus-corp/src/leerConfig/config.txt");

	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! esto lo dejo de recuerdo xD*/

	printf("\nHola! Soy %s!\n", config_get_string_value(YO, "nombre"));
	printf("Me cargaron %d parametros en mi configuracion\n", config_keys_amount(YO));
	printf("Te cuento, mis amigos me dicen '%s', y mi Plan de Niveles es:\n", config_get_string_value(YO, "simbolo") );

	char** niv = config_get_array_value(YO, key);
	for ( i=0; i<sizeof(niv)-1;i++){
		key= concat2(3, "obj[", niv[i], "]");
		plan = config_get_array_value(YO, key);
		printf("\n%s: ", niv[i]);

		for ( j=0; j<sizeof(plan)-1;j++){
			printf("%s, ", plan[j]);
		}

	}

	char* orqe =config_get_string_value(YO, "orquestador");

	printf("\n\nBueno, disculpame pero te hago un mangazo, tengo que llamar a mi amigo Jose Orquestador! \n");
	printf("Me prestas tu cel? Te paso su numero.. anda marcando %s...\nListo! muchas gracias, me voy yendo.\n\n¡¡SUERTE CON OPERATIVOS!!", orqe);
	char * primera;
	char * segunda;
	primera=strtok(orqe, ":");
	segunda=strtok(NULL, ":");
	printf("%s + %s", primera, segunda);

	return EXIT_SUCCESS;
}


