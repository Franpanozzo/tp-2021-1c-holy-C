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
#include <commons/log.h>

t_log* logMiRAM;

typedef struct {

	uint32_t pid;
	t_list* listaTareas;

} pcb;

typedef struct {

	uint32_t idTripulante;
	t_estado estado;
	uint32_t posX;
	uint32_t posY;
	t_tarea* proximaAEjecutar;
	pcb* patota;

} tcb;


void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarTareas(void*,t_list*,uint32_t);
void deserializarInfoPCB(t_paquete*);
void armarTarea(char*,t_list*);
void eliminarTarea(t_tarea*);
void eliminarPCB(pcb*);
void eliminarListaPCB(t_list*);
void eliminarListaTCB(t_list*);
void eliminarTCB(tcb* tcb);


/**
	* @NAME: deserializarSegun
	* @DESC: recibe un t_paquete* para deserializarlo al TAD que contiene el t_buffer*
	* segun el tipoDeDato y operarlo como corresponda dentro del switch(nunca salir de ahi)
	*/
void deserializarSegun(t_paquete*, int);
void deserializarTripulante(t_paquete*,int);
void asignarPatota(uint32_t, tcb*);
void asignarSiguienteTarea(tcb*);
void mandarTarea(t_tarea* , int);


#endif
