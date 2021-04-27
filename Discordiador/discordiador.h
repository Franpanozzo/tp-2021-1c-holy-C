#ifndef EJERCICIO10_H_
#define EJERCICIO10_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <memory.h>
#include <commons/config.h>


void iniciar_conexion(int);
void comunicarse();

#endif /* EJERCICIO10_H_ */

typedef struct{
	char* IP;
	int puerto;
} puertoEIP;
