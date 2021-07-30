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
#include <commons/temporal.h>
#include <stdbool.h>
#include "consola.h"
#include "utils.h"
#include "variables.h"
#include <math.h>

void hiloPlanificador();
void hiloSabotaje();
void hiloTripulante(t_tripulante*);

void iniciarTripulante(t_coordenadas, uint32_t, t_list*);
void iniciarPatota(t_coordenadas*, char*, uint32_t);

void actualizarListaEyB(t_lista*, t_estado);
void actualizarListaReady();
void actualizarListaNew();
void actualizarListaExec();
void actualizarListaBlocked();

#endif
