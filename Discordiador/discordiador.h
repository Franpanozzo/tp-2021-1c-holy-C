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


void crearConfig();
void iniciarPatota(t_coordenadas[], char*, uint32_t);
void hiloTripulante(t_tripulante* );
t_patota* asignarDatosAPatota(char*);
void atenderMiRAM(int,t_tripulante*);
char* deserializarString (t_paquete*);


#endif
