/*
 * memoria.h
 *
 *  Created on: 12 jun. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_


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

#define MEM_PPAL 0
#define MEM_VIRT 1

#define FRAME_INVALIDO 2147483646


typedef struct {

	int tamanioMemoria;
	char* esquemaMemoria;
	int tamanioPagina;
	int tamanioSwap;
	char* pathSwap;
	char* algoritmoReemplazo;
	char* criterioSeeleccion;
	int puerto;

} t_configRam;


t_log* logMemoria;
t_configRam configRam;

void* memoria_principal;
t_bitarray* frames_ocupados_ppal;
int cant_frames_ppal;


void cargar_configuracion();


#endif /* MEMORIA_H_ */




