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
#include <commons/string.h>
#include <commons/collections/list.h>



typedef struct {

    uint32_t pid;
    t_list* listaTareas;

} pcb;



void atender_tripulantes(int* serverSock);
int esperar_tripulante(int serverSock);
void manejar_tripulante(int* tripulanteSock);
void deserializarTareas(t_buffer* buffer,t_list* listaTareas);
void armarTarea(char*,t_list*);



#endif
