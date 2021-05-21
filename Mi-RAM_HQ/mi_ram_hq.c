#include "mi_ram_hq.h"

t_list* listaPCB;
t_list* listaTCB;

int main(void) {

    int puerto = 3222;
    listaPCB = list_create();
    listaTCB = list_create();

    int serverSock = iniciarConexionDesdeServidor(puerto);

    //Abro un thread manejar las conexiones
    pthread_t manejo_tripulante;
    pthread_create(&manejo_tripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
    pthread_join(manejo_tripulante, (void*) NULL);


    //falta hacer funcion para destruir las tareas de la lista de tareas del pcb
    //falta hacer funcion para destruir los pcb de las lista de pcbs
    //Prototipo:
    /*
     eliminarListaPCB(listaPCB);
     eliminarListaTCB(listaTCB);
     * */

    return EXIT_SUCCESS;
}

/*
 void eliminarTarea(t_tarea* tarea){
    free(tarea);
}

void eliminarPCB(pcb* pcb){
    list_destroy_and_destroy_elements(pcb->listaTareas, &eliminarTarea);
    free(pcb);
}

void eliminarListaPCB(t_list* listaPCB){
    list_destroy_and_destroy_elements( listaPCB, &eliminarPCB);
}

void eliminarListaTCB(t_list* listaTCB){
	list_destroy_and_destroy_elements( listaTCB, &eliminarTCB);
}

void eliminarTCB(tcb* tcb){
	free(tcb->proximaAEjecutar);
	free(tcb->pcb);
	free(tcb);
}
 */

void atenderTripulantes(int* serverSock) {

    while(1){

		int tripulanteSock = esperarTripulante(*serverSock);

		pthread_t t;
		pthread_create(&t, NULL, (void*) manejarTripulante, (void*) &tripulanteSock);
		pthread_detach(t);
    }

}


int esperarTripulante(int serverSock) {

    struct sockaddr_in serverAddress;
    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (void*) &serverAddress, &len);
    printf("Se conecto un cliente!\n");
    //log_info(logger, "Se conecto un cliente!");

    return socket_tripulante;

}


void manejarTripulante(int *tripulanteSock) {

    t_paquete* paquete = recibirPaquete(*tripulanteSock);

    deserializarSegun(paquete);

    eliminarPaquete(paquete);
}


void deserializarSegun(t_paquete* paquete){

	switch(paquete->codigo_operacion){

			case PATOTA:

						deserializarInfoPCB(paquete);
						break;

			case TRIPULANTE:

						deserializarTripulante(paquete);
						break;

			case TAREA:
						break;

			default:
					printf("\n No se puede deserializar ese tipo de estructura negro \n");
					exit(1);
		}
}


void deserializarTareas(void* stream,t_list* listaTareas,uint32_t tamanio){

    char* string;

    memcpy(&(string),stream,tamanio);

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


void deserializarInfoPCB(t_paquete* paquete) {

	pcb* nuevoPCB = malloc(sizeof(pcb));

	void* stream = paquete->buffer->stream;
	uint32_t* tamanio = malloc(sizeof(uint32_t));

	int offset = 0;
	memcpy(&(nuevoPCB->pid),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tamanio),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	nuevoPCB->listaTareas = list_create();

	deserializarTareas(stream + offset, nuevoPCB->listaTareas, *tamanio);

	t_tarea* tarea = list_get(nuevoPCB->listaTareas,0);
	printf("Recibi pa %s \n", tarea->cod_op);
	list_add(listaPCB,nuevoPCB);

	free(tamanio);
}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(sizeof(paquete->buffer->size));

	memcpy(string,&(paquete->buffer->stream),sizeof(paquete->buffer->size));


	return string;
}

void deserializarTripulante(t_paquete* paquete) {

	tcb* nuevoTCB = malloc(sizeof(tcb));
	uint32_t idPatota;
	void* stream = paquete->buffer->stream;

	int offset=0;

	memcpy(idPatota,stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->idTripulante),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->estado),stream + offset,sizeof(t_estado));
	offset += sizeof(t_estado);
	memcpy(&(nuevoTCB->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	asignarPatota(idPatota,nuevoTCB);
	asignarSiguienteTarea(nuevoTCB);
	list_add(listaTCB,nuevoTCB);

}

void asignarPatota(uint32_t idPatotaBuscada,tcb* tripulante) {

	bool idIgualA(pcb* pcbAComparar){

		return pcbAComparar->pid == idPatotaBuscada;
	}

	pcb* patotaCorrespondiente = malloc(sizeof(pcb));

	patotaCorrespondiente = list_find(listaPCB,(void*) idIgualA);

		if(patotaCorrespondiente == NULL){

		printf("No existe PCB para ese TCB negro\n");

		exit(1);

		}
		else{

		tripulante->patota = patotaCorrespondiente;
		}
}

void asignarSiguienteTarea(tcb* tripulante) {

	//Hago que su proxima tarea a ejecutar sea la primera de las lista,
	//despues hay que hablar bien como va a pedirle las tareas
	tripulante->proximaAEjecutar = list_get(tripulante->patota->listaTareas,0);

}










