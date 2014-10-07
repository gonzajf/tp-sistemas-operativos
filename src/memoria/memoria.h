#ifndef LIBMEMORIA_H_
#define LIBMEMORIA_H_

    #include "commons/collections/list.h"
#include "commons/string.h"

	typedef char* t_memoria;

    typedef struct {
        char id;
        int inicio;
        int tamanio;
        char* dato;
        bool libre;
    } t_particion;

    t_list* listaParticiones;

    t_memoria crear_memoria(int tamanio);
    int first_fit(t_memoria segmento,int tamanio);
    int almacenar_particion(t_memoria segmento, char id, int tamanio, char* contenido);
    int eliminar_particion(t_memoria segmento, char id);
    void liberar_memoria(t_memoria segmento);
    t_list* particiones(t_memoria segmento);
    void cleanList();
#endif /* LIBMEMORIA_H_ */
