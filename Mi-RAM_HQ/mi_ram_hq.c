#include "mi_ram_hq.h"

int idPatota = 1;

t_list* listaPCB;

int main(void) {

    int puerto = 3222;
    listaPCB = list_create();

    int serverSock = iniciarConexionDesdeServidor(puerto);

    //Abro un thread manejar las conexiones
    pthread_t manejo_tripulante;
    pthread_create(&manejo_tripulante, NULL, (void*) atender_tripulantes, (void*) &serverSock);
    pthread_join(manejo_tripulante, (void*) NULL);


    //falta hacer funcion para destruir las tareas de la lista de tareas del pcb
    //falta hacer funcion para destruir los pcb de las lista de pcbs

    return EXIT_SUCCESS;
}


void atender_tripulantes(int* serverSock) {

    while(1){

		int tripulanteSock = esperar_tripulante(*serverSock);

		pthread_t t;
		pthread_create(&t, NULL, (void*) manejar_tripulante, (void*) &tripulanteSock);
		pthread_detach(t);
    }

}


int esperar_tripulante(int serverSock) {

    struct sockaddr_in serverAddress;
    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (void*) &serverAddress, &len);
    printf("Se conecto un cliente!\n");
    //log_info(logger, "Se conecto un cliente!");

    return socket_tripulante;

}


void manejar_tripulante(int *tripulanteSock) { // Esta funcion deberia usar la funcion deserializarSegun() de bibliotecas

    t_paquete* paquete = recibirPaquete(*tripulanteSock);
    printf("sizeBuffer: %d\n",paquete->buffer->size);

    switch(paquete->codigo_operacion){

		case PERSONA:

			deserializarPersona(paquete->buffer);
			break;

		case TAREA_PATOTA:
		{
			pcb* nuevoPCB = malloc(sizeof(pcb));
			nuevoPCB->pid = idPatota;
			idPatota++;
			nuevoPCB->listaTareas = list_create();

			deserializarTareas(paquete->buffer, nuevoPCB->listaTareas);
			t_tarea* tarea = list_get(nuevoPCB->listaTareas,0);
			printf("Recibi pa %s \n", tarea->cod_op);
			list_add(listaPCB,nuevoPCB);
			break;
		}

		default:
			printf("\n No se puede deserializar ese tipo de estructura negro \n");
			exit(1);
    }

    eliminarPaquete(paquete);
}


void deserializarTareas(t_buffer* buffer,t_list* listaTareas){

    char* string = (char*) buffer->stream;

     char** arrayDeTareas = string_split(string,"\n");

    int i = 0;
    while(arrayDeTareas[i] != NULL){
        armarTarea(arrayDeTareas[i],listaTareas);
   }
}


void armarTarea(char* string,t_list* lista){
    t_tarea* tarea = malloc(sizeof(t_tarea));

    char** arrayParametros = string_split(string,";");

    if(string_contains(arrayParametros[0]," ")){
    	char** arrayPrimerElemento = string_split(arrayParametros[0]," ");
    	tarea->cod_op = arrayPrimerElemento[0];
    	tarea->parametro= (int) atoi(arrayPrimerElemento[1]);
    } else {
    	tarea->cod_op = arrayParametros[0];
    }
    tarea->posX = (int) atoi(arrayParametros[1]);
    tarea->posY = (int) atoi(arrayParametros[2]);
    tarea->tiempo = (int) atoi(arrayParametros[3]);

    list_add(lista,tarea);
}




