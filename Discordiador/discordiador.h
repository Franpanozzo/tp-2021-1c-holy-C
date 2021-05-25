#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_


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
#include "consola.h"


t_config* config;

t_log* logDiscordiador;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante;
int idPatota;

t_list* listaDeNew;

t_queue* colaDeReady;

t_list* listaExec;

pthread_mutex_t mutexListaNew;
pthread_mutex_t mutexColaReady;


void crearConfig();
void eliminarPatota(t_patota*);
void iniciarTripulante(t_coordenadas, uint32_t);
void iniciarPatota(t_coordenadas*, char*, uint32_t);
void recibirTareaDeMiRAM(int,t_tripulante*);
void pasarDeEstado(t_tripulante*, t_estado);
void newTripulante(t_tripulante* );
t_patota* asignarDatosAPatota(char*);
char* deserializarString (t_paquete*);
void mandarTareaAejecutar(t_tripulante*,int);
void recibirConfirmacionDeMongo(int,t_tarea*);


#endif
