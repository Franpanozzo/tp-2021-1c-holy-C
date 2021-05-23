#include "mi_ram_hq.h"

t_list* listaPCB;
t_list* listaTCB;

pthread_mutex_t mutexListaTCB;
pthread_mutex_t mutexListaPCB;

int main(void) {

    int puerto = 3222;
    listaPCB = list_create();
    listaTCB = list_create();
    pthread_mutex_init(&mutexListaTCB, NULL);
    pthread_mutex_init(&mutexListaPCB, NULL);

    logMiRAM = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/logMiRAM.log","Mi-Ram",1);

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
 	free(tarea->nombreTarea);
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

    log_info(logMiRAM, "Se conecto un cliente!\n");

    return socket_tripulante;

}

//CUANDO CREAS UN HILO HAY QUE PASAR SI O SI UN PUNTERO

void manejarTripulante(int *tripulanteSock) {

    t_paquete* paquete = recibirPaquete(*tripulanteSock);

    deserializarSegun(paquete,*tripulanteSock);

    //eliminarPaquete(paquete);
}


void deserializarSegun(t_paquete* paquete, int tripulanteSock){

	log_info(logMiRAM,"Deserializando...");

	switch(paquete->codigo_operacion){

			case PATOTA:

						deserializarInfoPCB(paquete);
						break;

			case TRIPULANTE:

						deserializarTripulante(paquete,tripulanteSock);
						break;

			default:
					log_info(logMiRAM,"No se puede deserializar ese tipo de estructura negro \n");
					exit(1);
		}
}


void deserializarTareas(void* stream,t_list* listaTareas,uint32_t tamanio){

    char* string = malloc(tamanio);

    memcpy(string,stream,tamanio);

    char** arrayDeTareas = string_split(string,"\n");

    int i = 0;

    while(strcmp(arrayDeTareas[i], "") != 0){
    	log_info(logMiRAM,"Procesando... %s\n",arrayDeTareas[i]);
        armarTarea(arrayDeTareas[i],listaTareas);
        i++;

   }

    //free(string);
}


void armarTarea(char* string,t_list* lista){


	t_tarea* tarea = malloc(sizeof(t_tarea));

	    char** arrayParametros = string_split(string,";");

	    if(string_contains(arrayParametros[0]," ")){

	    	char** arrayPrimerElemento = string_split(arrayParametros[0]," ");
	    	tarea->nombreTarea = strdup(arrayPrimerElemento[0]);
	    	tarea->parametro= (int) atoi(arrayPrimerElemento[1]);

	    } else {

	        tarea->nombreTarea = strdup(arrayParametros[0]);

	    }
	    tarea->posX = (uint32_t) atoi(arrayParametros[1]);
	    tarea->posY = (uint32_t) atoi(arrayParametros[2]);
	    tarea->tiempo = (uint32_t) atoi(arrayParametros[3]);

	    list_add(lista,tarea);

}


void deserializarInfoPCB(t_paquete* paquete) {

	pcb* nuevoPCB = malloc(sizeof(pcb));

	void* stream = paquete->buffer->stream;
	uint32_t tamanio;

	int offset = 0;
	memcpy(&(nuevoPCB->pid),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tamanio),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	nuevoPCB->listaTareas = list_create();

	deserializarTareas(stream + offset, nuevoPCB->listaTareas, tamanio);

	lock(mutexListaPCB);
	list_add(listaPCB,nuevoPCB);
    unlock(mutexListaPCB);

}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(sizeof(paquete->buffer->size));

	memcpy(string,&(paquete->buffer->stream),sizeof(paquete->buffer->size));


	return string;
}

void deserializarTripulante(t_paquete* paquete, int tripulanteSock) {

	tcb* nuevoTCB = malloc(sizeof(tcb));
	uint32_t idPatota;
	void* stream = paquete->buffer->stream;

	int offset=0;

	memcpy(&(idPatota),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->idTripulante),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->estado),stream + offset,sizeof(t_estado));
	offset += sizeof(t_estado);
	memcpy(&(nuevoTCB->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(nuevoTCB->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	log_info(logMiRAM,"Recibi el tribulante de id %d de la  patota %d",nuevoTCB->idTripulante, idPatota);

	asignarPatota(idPatota,nuevoTCB);

	log_info(logMiRAM,"Se le asigno la patota %d al tripulante %d",idPatota, nuevoTCB->idTripulante);

	asignarSiguienteTarea(nuevoTCB);
	mandarTarea(nuevoTCB->proximaAEjecutar, tripulanteSock);

	lock(mutexListaTCB);
	list_add(listaTCB,nuevoTCB);
	unlock(mutexListaTCB);

}

void asignarPatota(uint32_t idPatotaBuscada,tcb* tripulante) {

	bool idIgualA(pcb* pcbAComparar){

		bool a;
		a = pcbAComparar->pid == idPatotaBuscada;
		log_info(logMiRAM,"Comparado con primer patota %d \n",a);
		return a;
	}

	pcb* patotaCorrespondiente;

	patotaCorrespondiente = list_find(listaPCB,(void*) idIgualA);

		if(patotaCorrespondiente == NULL){

		log_info(logMiRAM,"No existe PCB para ese TCB negro\n");

		exit(1);

		}
		else{

		tripulante->patota = patotaCorrespondiente;
		// ASIGNAR TAREATRIPULANTE A LA PRIMERA POSICION DE LA LISTA DE LAS TAREAS DEL PCB
		}
}


void asignarSiguienteTarea(tcb* tripulante) {

	//Hago que su proxima tarea a ejecutar sea la primera de las lista,
	//despues hay que hablar bien como va a pedirle las tareas
	tripulante->proximaAEjecutar = list_get(tripulante->patota->listaTareas,0);

	log_info(logMiRAM,"Asignando la tarea: %s\n", tripulante->proximaAEjecutar->nombreTarea);

}


void mandarTarea(t_tarea* tarea, int socketTrip) {

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tarea, TAREA);

	log_info(logMiRAM,"Tarea enviada\n");

	enviarPaquete(paqueteEnviado, socketTrip);

	//eliminarPaquete(paqueteEnviado);

}







