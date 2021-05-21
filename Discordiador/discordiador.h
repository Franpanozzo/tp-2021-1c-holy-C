#ifndef EJERCICIO10_H_
#define EJERCICIO10_H_


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
#include <stdbool.h>


t_config* config;
t_log* logger;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante;
int idPatota;
t_list* listaDeNew;
t_queue* colaDeReady;
pthread_mutex_t mutexListaNew;
pthread_mutex_t mutexColaReady;

void crearConfig();
void iniciarPatota(t_coordenadas[], char*, uint32_t);
void hiloTripulante(t_tripulante* );
t_patota* asignarDatosAPatota(char*);
void atenderMiRAM(int,t_tripulante*);
char* deserializarString (t_paquete*);


#endif
