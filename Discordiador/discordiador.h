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


typedef struct{
	char* IP;
	int puerto;
} puertoEIP;


void crearConfig();
int iniciarConexionCon(void*);


#endif
