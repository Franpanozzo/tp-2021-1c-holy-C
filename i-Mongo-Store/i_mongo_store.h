#ifndef IMONGOSTORE_H_
#define IMONGOSTORE_H_


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


t_log* logImongo;


void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarSegun(t_paquete*, int);
int indiceTarea(t_tarea*);


#endif

