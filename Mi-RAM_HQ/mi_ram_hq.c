#include "mi_ram_hq.h"

int idPatota = 1;

t_list* listaPCB = list_Create();

int main(void) {

    int puerto = 3222;

    int serverSock = iniciarConexionDesdeServidor(puerto);

    //Abro un thread manejar las conexiones
    pthread_t manejo_tripulante;
    pthread_create(&manejo_tripulante, NULL, (void) atender_tripulantes, (void)serverSock);
    pthread_join(manejo_tripulante, (void*) NULL);


    return EXIT_SUCCESS;
}


void atender_tripulantes(int serverSock) {

    while(1){

    int tripulanteSock = esperar_tripulante(serverSock);

    pthread_t t;
    pthread_create(&t, NULL, (void) manejar_tripulante, (void) tripulanteSock);
    pthread_detach(t);

    }

}


int esperar_tripulante(int serverSock) {

    struct sockaddr_in serverAddress = malloc(sizeof(struct sockaddr_in));
    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (struct sockaddr) serverAddress, &len);

    //log_info(logger, "Se conecto un cliente!");

    return socket_tripulante;

}


void manejar_tripulante(int tripulanteSock) { // Esta funcion deberia usar la funcion deserializarSegun() de bibliotecas

    t_paquete paquete = recibirPaquete(tripulanteSock);


    switch(paquete->codigo_operacion){

                case PERSONA:
                            deserializarPersona(paquete->buffer);
                            break;

                case TAREA_PATOTA:
                {
                            pcb* nuevoPCB = malloc(sizeof(pcb));
                            nuevoPCB->pid = idPatota;
                            idPatota++;
                            nuevoPCB->listaTareas = list_Create();

                            deserializarTarea(paquete->buffer, nuevoPCB->listaTareas);
                            list_add(listaPCB, (void*) nuevoPCB);
                }

                default:
                        printf("\n No se puede deserializar ese tipo de estructura negro \n");
                        exit(1);
            }

    eliminarPaquete(paquete);
}




