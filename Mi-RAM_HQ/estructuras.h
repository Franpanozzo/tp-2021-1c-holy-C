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


typedef struct {
	int nroPagina;
	int desplazamiento;
} t_DL;

typedef struct {

	uint32_t pid;
	t_DL dlTareas;

} pcb;

typedef struct {

	uint32_t idTripulante;
	t_estado estado;
	uint32_t posX;
	uint32_t posY;
	t_DL proximaAEjecutar;
	t_DL dlPatota;

} tcb;



#endif
