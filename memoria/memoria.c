
//algoritmo FIRST-FIT

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memoria.h"

t_memoria crear_memoria(int tamanio)
{
	t_memoria memoria = calloc(tamanio, 1);

	listaParticiones = list_create();

	t_particion* particion = (t_particion*)malloc(sizeof(t_particion));

	particion->inicio = 0;
	particion->libre = true;
	particion->tamanio = tamanio;

	list_add(listaParticiones,particion);

	return memoria;
}


//first_fit
//devuelve el indice de la particion libre a insertar, o -1 si no hay ninguna

int first_fit(t_memoria segmento,int tamanio)
{
	int index = 0;
	t_particion* particion;

	for(index=0;index<listaParticiones->elements_count;index++)
	{
		particion = list_get(listaParticiones,index);

		if((particion->libre == true) && (tamanio <= particion->tamanio))
			break;
	}

	if(index == listaParticiones->elements_count)
		index = -1;

	return index;
}


//FUNCIONES AUXILIARES PARA UTILIZAR LAS COMMONS

bool vaAntes(t_particion* particion1,t_particion* particion2)
{
	return particion1->inicio < particion2->inicio;
}


int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido)
{
	int sobrante = 0;
	t_particion* particionNueva;
	t_particion* particionLibre;
	int index = first_fit(segmento,tamanio);

	//si first_fit encontro una particion libre adecuada, inserta
	if(index != -1)
	{
		//busca la particion libre y la utiliza
		particionNueva = list_get(listaParticiones,index);

		//inserta el contenido en el segmento
		memcpy(segmento + particionNueva->inicio,contenido,tamanio);

		if(tamanio < particionNueva->tamanio)
			sobrante = particionNueva->tamanio - tamanio;

		//arma la nueva particion y la inserta en listaParticiones
		particionNueva->dato = segmento + particionNueva->inicio;
		particionNueva->id = id;
		particionNueva->libre = false;
		particionNueva->tamanio = tamanio;

		//modifica o elimina la particion libre encontrada, segun sea necesario
		if(sobrante > 0){
			particionLibre = malloc(sizeof(t_particion));

			particionLibre->tamanio = sobrante;
			particionLibre->inicio = particionNueva->inicio + particionNueva->tamanio;
			particionLibre->libre = true;
			particionLibre->dato = segmento + particionLibre->inicio;

			list_add_in_index(listaParticiones,index + 1,particionLibre);
		}
	}

	//si no encuentra una particion libre adecuada para insertar, no hace nada!
	return -1;
}

int eliminar_particion(t_memoria segmento, char id)
{
	int i;
	//busca la particion por id

	for (i = 0; i < listaParticiones->elements_count; i++) {
		t_particion* particion = list_get(listaParticiones,i);

		if(particion->id == id)
		{	//establece la particion como "libre"
			particion->libre = true;
			break;
		}
	}


	return 0;
}

void cleanList(){
	int i;
	t_particion* particion;
	for (i=0; i<listaParticiones->elements_count;i++){
		particion = list_remove(listaParticiones,i);
		free(particion);

	}
	free(listaParticiones);
}

void liberar_memoria(t_memoria segmento)
{
	//list_destroy_and_destroy_elements(listaParticiones,free);	//elimina la listaParticiones
	cleanList();
	free(segmento);					//libera la memoria
}

t_list* particiones(t_memoria segmento)
{
	t_list* lista = list_take(listaParticiones,listaParticiones->elements_count);

	return lista;
}

