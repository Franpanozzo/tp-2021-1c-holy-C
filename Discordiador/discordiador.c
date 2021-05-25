#include "discordiador.h"


int main() {

	idTripulante = 0;
	idPatota = 0;

	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/logDiscordiador.log","Discordiador",1);

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	listaDeNew = list_create();
	colaDeReady = queue_create();
	listaExec = list_create();

	pthread_mutex_init(&mutexListaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
/*
	int serverImongo = iniciarConexionDesdeClienteHacia(puertoEIPMongo);

	t_patota* patotaPrueba = asignarDatosAPatota("GENERAR_OXIGENO 4;5;6;7\nGENERAR_COMIDA;5;6;7\n");

	t_paquete*paquetePatota = armarPaqueteCon((void*) patotaPrueba,PATOTA);

	enviarPaquete(paquetePatota,serverImongo);
*/

	leerConsola();

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}


void crearConfig(){

	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");

	if(config == NULL){

		log_error(logDiscordiador, "\n La ruta es incorrecta \n");
		exit(1);
		}
}


void eliminarPatota(t_patota* patota){
	free(patota->tareas);
	free(patota);
}


t_patota* asignarDatosAPatota(char* tareasString){

	t_patota* patota = malloc(sizeof(t_patota));

	patota->tamanioTareas = strlen(tareasString) + 1;

	idPatota++;

	log_info(logDiscordiador,"Se creo la patota numero %d\n",idPatota);

	patota->ID = idPatota;

	patota->tareas = tareasString;

	return patota;
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	t_patota* patota = asignarDatosAPatota(tareasString);

	t_paquete* paquete = armarPaqueteCon((void*) patota,PATOTA);

	enviarPaquete(paquete,server_socket);

	for (int i=0; i<cantidadTripulantes; i++){

		iniciarTripulante(coordenadas[i], patota->ID);
	}

	eliminarPatota(patota);
	close(server_socket);

}

void iniciarTripulante(t_coordenadas coordenada, uint32_t idPatota){

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));

	pthread_t hiloTripulante;

	idTripulante++;

	tripulante->posX = coordenada.posX;
	tripulante->posY = coordenada.posY;
	tripulante->idTripulante = idTripulante;
	tripulante->idPatota = idPatota;
	tripulante->estado = NEW;

	sem_init(&tripulante->semaforo, 0, 0);

	lock(mutexListaNew);
	list_add(listaDeNew,(void*)tripulante);
	unlock(mutexListaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d\n",tripulante->idTripulante);


	pthread_create(&hiloTripulante, NULL, (void*) newTripulante, (void*) tripulante);
	pthread_join(hiloTripulante, (void**) NULL);
}


void pasarDeEstado(t_tripulante* tripulante, t_estado siguienteEstado){

	t_tripulante* tripulanteParaCheckear;

	bool idIgualA(t_tripulante* tripulanteAcomparar){

		return tripulanteAcomparar->idTripulante == tripulante->idTripulante;
	}

	switch(tripulante->estado){

		case NEW:

			lock(mutexListaNew);
			tripulanteParaCheckear = list_remove_by_condition(listaDeNew, (void*) idIgualA);
			unlock(mutexListaNew);

			break;

		case READY:
			//remover de ready

			break;

		case EXEC:

			break;

		case BLOCKED:

			break;

		default:
				printf("\n No se reconoce el estado \n");
				exit(1);
	}


	switch(siguienteEstado){

		case READY:
			if(tripulanteParaCheckear != NULL){

				tripulante->estado = READY;

				lock(mutexColaReady);
				queue_push(colaDeReady,(void*) tripulanteParaCheckear);
				unlock(mutexColaReady);
				//¿Aca se mandaria el mensaje para actualiazar el tcb?
				log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);

			}
			else{
				log_info(logDiscordiador,"Estas queriendo meter a Ready un NULL  \n");
				exit(1);
			}

			break;

		case EXEC:
			tripulante->estado = EXEC;
			lock(mutexListaExec);
			list_add(listaExec,(void*) tripulanteParaCheckear);
			unlock(mutexListaExec);

			//¿Aca se mandaria el mensaje para actualiazar el tcb?
			break;

		case BLOCKED:

			break;

		default:
				printf("\n No se reconoce el siguiente estado \n");
				exit(1);
	}

}


void newTripulante(t_tripulante* tripulante) {
	//id patato y tripulante

	recibirPrimerTareaDeMiRAM(tripulante);

	pasarDeEstado(tripulante, READY);


	//pasarDeEstado(tripulante,EXEC);//

	while(strcmp(tripulante->instruccionAejecutar->nombreTarea,"tareaFinal") != 0){
		//sem_wait(tripulante->semaforo);
		int iMongoSocket = iniciarConexionDesdeClienteHacia(puertoEIPMongo);
		mandarTareaAejecutar(tripulante,iMongoSocket);
		close(iMongoSocket);

		//recibirProximaTareaDeMiRAM(tripulante);
	}


}

void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

	if(paqueteRecibido->codigo_operacion == TAREA){

	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);

	    log_info(logDiscordiador,"Soy el tripulante %d y recibi la tarea de: %s \n",
	    		tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	}
	else{
	    log_error(logDiscordiador,"El paquete recibido no es una tarea\n");
	    exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}

void recibirPrimerTareaDeMiRAM(t_tripulante* tripulante){
	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);
	enviarPaquete(paqueteEnviado, miRAMsocket);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);
	close(miRAMsocket);
}

void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){
	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete * paquete = armarPaqueteCon((void*) tripulante,SIGUIENTETAREA);
	enviarPaquete(paquete, miRAMsocket);

	recibirTareaDeMiRAM(miRAMsocket,tripulante);
	close(miRAMsocket);
}

void mandarTareaAejecutar(t_tripulante* tripulante, int socketMongo){

	t_paquete* paqueteConLaTarea = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);

	enviarPaquete(paqueteConLaTarea,socketMongo);

	recibirConfirmacionDeMongo(socketMongo,tripulante->instruccionAejecutar);
}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(sizeof(paquete->buffer->size));

	memcpy(string,&(paquete->buffer->stream),sizeof(paquete->buffer->size));

	return string;
}



void recibirConfirmacionDeMongo(int socketMongo, t_tarea* tarea){

	t_paquete* paqueteRecibido = recibirPaquete(socketMongo);

	char* mensajeRecibido = deserializarString(paqueteRecibido);

	if(strcmp(mensajeRecibido,"TAREA REALIZADA") == 0){

		log_info(logDiscordiador,"Se elimino la tarea %s\n",tarea->nombreTarea);

		free(tarea->nombreTarea);
		free(tarea);
		free(mensajeRecibido);
	}

	else{

		log_info(logDiscordiador, "No sabemos q hacer todavia =(\n");

		free(tarea->nombreTarea);
		free(tarea);
		free(mensajeRecibido);

	}

}
