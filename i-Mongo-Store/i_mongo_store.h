#ifndef IMONGOSTORE_H_
#define IMONGOSTORE_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <bibliotecas.h>
#include <unistd.h>
#include <sys/select.h>
#include <memory.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <stdbool.h>
#include "utils.h"
#include "variables.h"
#include <math.h>


t_log* logImongo;


void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarSegun(t_paquete*, int);
int indiceTarea(t_tarea*);


#endif

