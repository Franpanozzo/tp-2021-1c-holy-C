#include "mi_ram_hq.h"


t_list* listaPCB;
t_list* listaTCB;

pthread_mutex_t mutexListaTCB;
pthread_mutex_t mutexListaPCB;
pthread_mutex_t mutexListaTareas;
pthread_mutex_t mutexTripulante;



int main(void) {

    int puerto = 3222;

    listaPCB = list_create();
    listaTCB = list_create();

    pthread_mutex_init(&mutexListaTCB, NULL);
    pthread_mutex_init(&mutexListaPCB, NULL);
    pthread_mutex_init(&mutexListaTareas, NULL);
    pthread_mutex_init(&mutexTripulante, NULL);


    logMiRAM = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/logMiRAM.log","Mi-Ram",1);

    int serverSock = iniciarConexionDesdeServidor(puerto);

    //Abro un thread manejar las conexiones
    pthread_t manejo_tripulante;
    pthread_create(&manejo_tripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
    pthread_join(manejo_tripulante, (void**) NULL);

    //falta hacer funcion para destruir las tareas de la lista de tareas del pcb
    //falta hacer funcion para destruir los pcb de las lista de pcbs
    //Prototipo:

     //eliminarListaTCB(listaTCB);
     //eliminarListaPCB(listaPCB);

     return EXIT_SUCCESS;

}


void eliminarTarea(t_tarea* tarea) {
 	free(tarea->nombreTarea);
    free(tarea);
}

void eliminarPCB(pcb* pcb) {
    list_destroy_and_destroy_elements(pcb->listaTareas, (void*) eliminarTarea);
    free(pcb);
}

void eliminarListaPCB(t_list* listaPCB) {
    list_destroy_and_destroy_elements( listaPCB, (void*) eliminarPCB);
}

void eliminarListaTCB(t_list* listaTCB){
	list_destroy_and_destroy_elements( listaTCB, (void*) eliminarTCB);
}

void eliminarTCB(tcb* tcb){
	free(tcb);
}



void atenderTripulantes(int* serverSock) {

    while(1){

    	int * tripulanteSock = malloc(sizeof(int));
		*tripulanteSock = esperarTripulante(*serverSock);

		pthread_t t;
		//pthread_t * t=malloc(sizeof(pthread_t));

		pthread_create(&t, NULL, (void*) manejarTripulante, (void*) tripulanteSock);

		pthread_detach(t);
		//free(t);
		//Para hacerle free hay que pasarlo por parametro en pthread_create

		//pthread_join(t, (void**) NULL);
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
    free(tripulanteSock);
    //no estoy si close libera la memoria,
    //porque no recibe el puntero si no la
    //info que contiene la direccion de memoria
}


void deserializarSegun(t_paquete* paquete, int tripulanteSock){

	log_info(logMiRAM,"Deserializando el cod_op %d", paquete->codigoOperacion);

	switch(paquete->codigoOperacion){
		case PATOTA:
			//log_info(logMiRAM,"Voy a deserializar una patota");
			deserializarInfoPCB(paquete);
			mandarConfirmacionDisc("OK", tripulanteSock);
			break;

		case TRIPULANTE:
			log_info(logMiRAM,"Voy a deserializar un tripulante");

			deserializarTripulante(paquete,tripulanteSock);
			break;

		case EXPULSAR:
			log_info(logMiRAM,"Voy a expulsar un tripulante, todavia no esta emplementada");
//			deserializarTripulante(paquete,tripulanteSock);
			break;

		case ESTADO_TRIPULANTE:
			log_info(logMiRAM,"Voy a actualizar un tripulante");
			actualizarTripulante(paquete);
			mandarConfirmacionDisc("OK", tripulanteSock);
			break;

		case SIGUIENTE_TAREA:
		{
			log_info(logMiRAM,"VOY A ASIGNARLE LA PROX TAREA A UN TRIPU");
			deserializarSolicitudTarea(paquete,tripulanteSock);
			break;
		}
		default:
			log_info(logMiRAM,"No se puede deserializar ese tipo de estructura negro \n");
			exit(1);

		}
	eliminarPaquete(paquete);
	close(tripulanteSock);
}


void mandarConfirmacionDisc(char* aMandar, int socket) {

	t_paquete* aEnviar = armarPaqueteCon((void*) aMandar, STRING);
	log_info(logMiRAM,"Mandando confirmacion de %s a Discordiador",aMandar);
	enviarPaquete(aEnviar,socket);
}



void deserializarSolicitudTarea(t_paquete* paquete, int tripulanteSock) {
	uint32_t idPatota;
	uint32_t idTripulante;
	void * stream = paquete->buffer->stream;
	int offset = 0;
	memcpy(&(idPatota), stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(idTripulante), stream + offset, sizeof(uint32_t));

	/*
	bool esIgualA(tcb* tcbAcomparar){

		return tcbAcomparar->idTripulante == idTripulante
				&& tcbAcomparar->patota->pid == idPatota;
	}*/

	lock(mutexListaPCB);
	pcb* supuestaPatotaTripu = buscarPatota(idPatota);

	log_info(logMiRAM,"SUPUESTA PATOTA  DEL TRIPU %d",idPatota,idTripulante);
    unlock(mutexListaPCB);

	lock(mutexListaTCB);
	tcb* buscado = buscarTripulante(idTripulante, supuestaPatotaTripu);
	unlock(mutexListaTCB);


	lock(mutexTripulante);
	log_info(logMiRAM,"LLENDO A BUSCAR TAREA DEL TRIPU %d",buscado->idTripulante);

	lock(mutexListaTareas);
	setearSgteTarea(buscado);

	t_paquete *paqueteTarea  = armarPaqueteCon(buscado->proximaAEjecutar,TAREA);
	enviarPaquete(paqueteTarea,tripulanteSock);

	unlock(mutexListaTareas);
	unlock(mutexTripulante);

}


void setearSgteTarea(tcb *buscado){
	t_list_iterator * iterator = list_iterator_create(buscado->patota->listaTareas);
	while(list_iterator_has_next(iterator)){
		t_tarea* tarea = list_iterator_next(iterator);
		if(tarea == buscado->proximaAEjecutar){
			tarea = list_iterator_next(iterator);
			buscado->proximaAEjecutar = tarea;
			log_info(logMiRAM,"TRIPU: %d - PROX. TAREA: %s",buscado->idTripulante,tarea->nombreTarea);
			break;
		}
	}
	list_iterator_destroy(iterator);
}


void deserializarTareas(void* stream,t_list* listaTareas,uint32_t tamanio){

    char* string = malloc(tamanio);
    memcpy(string,stream,tamanio);

    char** arrayDeTareas = string_split(string,"\n");

    //strcmp(arrayDeTareas[i], "") != 0
    for(int i =0; arrayDeTareas[i] != NULL; i++){

    	log_info(logMiRAM,"Procesando... %s\n",arrayDeTareas[i]);
        armarTarea(arrayDeTareas[i],listaTareas);

    }

    t_tarea * tareaFinal = malloc(sizeof(t_tarea));
    tareaFinal->nombreTarea = strdup("TAREA_NULA");
	list_add(listaTareas,tareaFinal);

	liberarDoblesPunterosAChar(arrayDeTareas);
    free(string);

}


void armarTarea(char* string,t_list* lista){


	t_tarea* tarea = malloc(sizeof(t_tarea));

	    char** arrayParametros = string_split(string,";");

	    if(string_contains(arrayParametros[0]," ")){

	    	char** arrayPrimerElemento = string_split(arrayParametros[0]," ");

	    	tarea->nombreTarea = strdup(arrayPrimerElemento[0]);

	    	tarea->parametro= (int) atoi(arrayPrimerElemento[1]);

			liberarDoblesPunterosAChar(arrayPrimerElemento);

	    } else {

	        tarea->nombreTarea = strdup(arrayParametros[0]);

	    }

	    tarea->coordenadas.posX = (uint32_t) atoi(arrayParametros[1]);

	    tarea->coordenadas.posY = (uint32_t) atoi(arrayParametros[2]);

	    tarea->tiempo = (uint32_t) atoi(arrayParametros[3]);

	    list_add(lista,tarea);

	    liberarDoblesPunterosAChar(arrayParametros);

}

void liberarDoblesPunterosAChar(char** arrayParametros) {

	char** liberadorDeStrings = arrayParametros;

	while((*liberadorDeStrings) != NULL) {

		free(*liberadorDeStrings);
		liberadorDeStrings++;
		//if((*liberadorDeStrings) == NULL) free(arrayParametros);

	}

	free(arrayParametros);
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


void actualizarTripulante(t_paquete* paquete) {

	uint32_t idTCBAActualizar;

	int offset=0;

	uint32_t idPatota;

	void* stream = paquete->buffer->stream;

	memcpy(&(idPatota),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	pcb* patotaDeTripu = buscarPatota(idPatota);

	memcpy(&(idTCBAActualizar),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	lock(mutexListaTCB);
	tcb* tcbEncontrado = buscarTripulante(idTCBAActualizar,patotaDeTripu);
	unlock(mutexListaTCB);

	lock(mutexTripulante);
	memcpy(&(tcbEncontrado->estado),stream + offset,sizeof(t_estado));
	offset += sizeof(t_estado);

	memcpy(&(tcbEncontrado->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(tcbEncontrado->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	log_info(logMiRAM,"Actualizado el tripulante de id %d de la  patota %d",tcbEncontrado->idTripulante, idPatota);
	unlock(mutexTripulante);

}


tcb* buscarTripulante(int idTCBAActualizar,pcb* patotaDeTripu) {

	bool idIgualA(tcb* tcbAComparar){

			bool a;

			a = tcbAComparar->idTripulante == idTCBAActualizar && (tcbAComparar->patota == patotaDeTripu) ;

			//log_info(logMiRAM,"Comparado con primer patota %d \n",a);

			return a;
		}

		tcb* tripulanteCorrespondiente;

		tripulanteCorrespondiente = list_find(listaTCB,(void*) idIgualA);

			if(tripulanteCorrespondiente == NULL){

			log_error(logMiRAM,"No existe ese TCB negro\n");

			exit(1);

			}
			else{

			return tripulanteCorrespondiente;
			// ASIGNAR TAREATRIPULANTE A LA PRIMERA POSICION DE LA LISTA DE LAS TAREAS DEL PCB

			}
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

	lock(mutexListaPCB);
	pcb* patotaCorrespondiente = buscarPatota(idPatota);
	unlock(mutexListaPCB);

	nuevoTCB->patota = patotaCorrespondiente;

	log_info(logMiRAM,"Se le asigno la patota %d al tripulante %d",idPatota, nuevoTCB->idTripulante);

	lock(mutexListaTareas);
	asignarSiguienteTarea(nuevoTCB);

	mandarTarea(nuevoTCB->proximaAEjecutar, tripulanteSock);
	unlock(mutexListaTareas);


	lock(mutexListaTCB);
	list_add(listaTCB,nuevoTCB);
	unlock(mutexListaTCB);
}


pcb* buscarPatota(uint32_t idPatotaBuscada) {

	bool idIgualA(pcb* pcbAComparar){

		bool a;

		a = pcbAComparar->pid == idPatotaBuscada;

		log_info(logMiRAM,"Comparado patota %d con patota de lista %d \n",pcbAComparar->pid,idPatotaBuscada);

		return a;
	}

	pcb* patotaCorrespondiente;

	patotaCorrespondiente = list_find(listaPCB,(void*) idIgualA);

		if(patotaCorrespondiente == NULL){

		log_error(logMiRAM,"No existe PCB para ese TCB negro\n");

		exit(1);

		}
		else{

		return patotaCorrespondiente;
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

}







