#ifndef MI_RAM_HQ_H_
#define MI_RAM_HQ_H_


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

void atender_tripulantes(int serverSock);
int esperar_tripulante(int serverSock);
void manejar_tripulante(int tripulanteSock);
#endif
