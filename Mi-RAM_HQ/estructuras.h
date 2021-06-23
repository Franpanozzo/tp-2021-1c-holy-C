#ifndef ESTRUCTURASH
#define ESTRUCTURASH


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
#include <commons/bitarray.h>
#include <math.h>
#include <commons/temporal.h>
#include <commons/txt.h>




typedef struct {

	uint32_t pid;
	uint32_t dlTareas;

} pcb;

typedef struct {

	uint32_t idTripulante;
	char estado;
	uint32_t posX;
	uint32_t posY;
	uint32_t proximaAEjecutar;
	uint32_t dlPatota;

} tcb;

typedef enum {
	PCB,
	TCB,
	TAREAS
} tipoEstructura;



#endif
