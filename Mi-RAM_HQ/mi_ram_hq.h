#ifndef MI_RAM_HQH
#define MI_RAM_HQH


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <commons/config.h>
#include <bibliotecas.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct {

	    uint32_t pid;
	    t_list* listaTareas;

	} pcb;

void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarTareas(void*,t_list*);
void deserializarInfoPCB(t_paquete*);
void armarTarea(char*,t_list*);

/**
	* @NAME: deserializarSegun
	* @DESC: recibe un t_paquete* para deserializarlo al TAD que contiene el t_buffer*
	* segun el tipoDeDato y operarlo como corresponda dentro del switch(nunca salir de ahi)
	*/
void deserializarSegun(t_paquete*);


#endif
