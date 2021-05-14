#ifndef MI_RAM_HQH
#define MI_RAM_HQH


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

typedef enum {

    GENERAR_OXIGENO

} tipoTarea;


typedef struct {

    tipoTarea tarea;
    uint32_t parametro;
    uint32_t posX;
    uint32_t posY;
    uint32_t tiempo;

} t_tarea;


typedef enum {

    uint32_t pid;
    t_list* listaTareas;

} pcb;



void atender_tripulantes(int serverSock);
int esperar_tripulante(int serverSock);
void manejar_tripulante(int tripulanteSock);
#endif
