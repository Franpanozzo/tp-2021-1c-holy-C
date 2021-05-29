#include "discordiador.h"

char** todasLasTareasIO;

int main() {

	todasLasTareasIO = malloc(sizeof(char*) * 6);

	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");

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
	colaES = queue_create();

	pthread_mutex_init(&mutexListaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
	pthread_mutex_init(&mutexListaExec, NULL);

	//leerConsola();

	t_coordenadas coordenadas[4];
	for(int i=0; i<4;i++){
		coordenadas[i].posX = i;
		coordenadas[i].posY = i + 1;
	}

	planificador("INICIAR_PLANIFICACION",coordenadas);


	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}

void planificador(char* string,t_coordenadas* coordenadas){

	char* algoritmo;
	int grado_multiprocesamiento;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	grado_multiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	log_info(logDiscordiador,"Empezando planificacion usando el algoritmo %s",algoritmo);

	if(strcmp(string,"INICIAR_PLANIFICACION") == 0){
		iniciarPatota(coordenadas, "GENERAR_OXIGENO 4;5;6;7\nCONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2", 4);

	}

	while(list_size(listaExec) <= grado_multiprocesamiento && queue_is_empty(colaDeReady) != 1){

		t_tripulante* tripulante = queue_peek(colaDeReady);

		log_info(logDiscordiador,"Moviendo tripulante %d a EXEC", tripulante->idTripulante);

		queue_pop(colaDeReady);

		pasarDeEstado(tripulante, EXEC);
	}

	t_list_iterator* iterador = list_iterator_create(listaExec);

	while(list_iterator_has_next(iterador)){

		t_tripulante* tripulanteAsacar = (t_tripulante*) list_iterator_next(iterador);

		if(strcmp(algoritmo,"FIFO") == 0){
			planificacionFIFO(tripulanteAsacar);
		}
		else if(strcmp(algoritmo,"RR") == 0){
			planificacionRR(tripulanteAsacar);
		}
		else{
			log_info(logDiscordiador,"No puedo planificar en ese algoritmo negro\n");
			exit(1);
		}
	}
}

int calculoCiclosARealizar(t_tripulante* tripulante){

	int movimientosEnX = fabs(tripulante->instruccionAejecutar->posX - tripulante->posX);
	int movimientosEnY = fabs(tripulante->instruccionAejecutar->posY - tripulante->posY);

	return movimientosEnX + movimientosEnY;
}

void tareasIO(t_tripulante* tripulante){

	int ciclos = calculoCiclosARealizar(tripulante) + 1;

	while(ciclos > 0){

		//int socketMiRAM = iniciarConexionDesdeCliente(puertoEIPRAM);

		if(tripulante->posX > tripulante->instruccionAejecutar->posX){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posX--;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

		}

		if(tripulante->posX < tripulante->instruccionAejecutar->posX){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posX++;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}

		if(tripulante->posY > tripulante->instruccionAejecutar->posY){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posY--;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}

		if(tripulante->posY < tripulante->instruccionAejecutar->posY){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posY++;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}
		if(tripulante->posX == tripulante->instruccionAejecutar->posX && tripulante->posY == tripulante->instruccionAejecutar->posY){
			ciclos--;
			sleep(1);
			int socketMongo = iniciarConexionDesdeClienteHacia(puertoEIPMongo);
			t_paquete* tareaArealizar = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);
			enviarPaquete(tareaArealizar,socketMongo);
			t_paquete* prueba = recibirPaquete(socketMongo);
			char* respuesta = deserializarString(prueba);
			log_info(logDiscordiador,"Recibi %s \n", respuesta);
			//bool esIgualA(t_tripulante* tripulanteAcomparar){
				//return tripulanteAcomparar == tripulante;
			//}
			//list_remove_by_condition(listaExec,(void*) esIgualA);
			t_bloqueado* tripulanteBloqueado = malloc(sizeof(t_bloqueado));
			tripulanteBloqueado->socket = socketMongo;
			tripulanteBloqueado->tripulante = tripulante;
			queue_push(colaES, tripulanteBloqueado);

		}

	}


}


void tareasNoIO(t_tripulante* tripulante){

	//int cantidadAciclar = calculoCiclosARealizar(tripulante) + tripulante->instruccionAejecutar->tiempo;
}

void planificacionFIFO(t_tripulante* tripulante){

	switch(esIO(tripulante->instruccionAejecutar->nombreTarea)){

			case 1:
			{
					log_info(logDiscordiador,"Recibi una accion IO del tripulante %d",tripulante->idTripulante);
					tareasIO(tripulante);
					break;
			}
			case 0:
			{		log_info(logDiscordiador,"Recibi una accion que no es IO del tripulante %d",tripulante->idTripulante);
					tareasNoIO(tripulante);
					break;
			}

			default:
			{
					log_info(logDiscordiador,"Recibi cualquier cosa negro\n");
					exit(1);
			}
			}
}



void planificacionRR(t_tripulante* tripulante){

}

/*-------------------------------------------------------------------------------
//LO LLAMA SANTI DESDE LA CONSOLA
void cpuPlanificacion() {
	char* algoritmo;
	int grado_multiprocesamiento;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	grado_multiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	log_info(logDiscordiador,"Empezando planificacion usando el algoritmo %s",algoritmo);

	planificacion_pausada= 1;
	//Iniciar_planificaion -> pausado=1;
	//Pausar_planificacion -> pausado=0;

	while(1){

	while(planificacion_pausada);

	if(planificacion_pausada){
		sem_wait(&semPlanificacion);//lo hace consola
	}

	if(list_size(listaDeNew)>0){//independiente del algoritmo de planificacion
		lock(mutexListaNew);
		t_tripulante* tripulanteARedy =  list_remove(listaDeNew,0);
		unlock(mutexListaNew);
		pasarDeEstado(tripulanteARedy,READY);

	}

	if(list_size(listaExec) < grado_multiprocesamiento){//Cuando la planificacion esta activa

		if(strcmp(algoritmo,"FIFO") == 0) {
			lock(mutexColaReady);
			t_tripulante* tripulanteAExec =  queue_pop(colaDeReady);
			unlock(mutexColaReady);
			pasarDeEstado(tripulanteAExec,EXEC);

		}

		if(strcmp(algoritmo,"RR") == 0) {
			//Todo
		}

		else {
			log_info(logDiscordiador,"No existe ese algoritmo de planificacion negro");
		}
	}
 }

}

------------------------------------------------------------------------------------------------
*/


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

	//eliminarPatota(patota);
	close(server_socket);

}


void iniciarTripulante(t_coordenadas coordenada, uint32_t idPatota){

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));

	pthread_t _hiloTripulante;

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



	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, (void*) tripulante);
	pthread_join(_hiloTripulante, (void**) NULL);
}


int esIO(char* tarea){

	int i = 0;

	while(todasLasTareasIO[i] != NULL){

		if(strcmp(todasLasTareasIO[i],tarea) == 0){
			return 1;
		}


		i++;
	}

	return 0;

}


void pasarDeEstado(t_tripulante* tripulante, t_estado siguienteEstado){


	switch(siguienteEstado){

		case READY:
			//Ir a Hoja 10 del tp
			tripulante->estado = READY;
			lock(mutexColaReady);
			queue_push(colaDeReady,(void*) tripulante);
			unlock(mutexColaReady);
			//¿Aca se mandaria el mensaje para actualiazar el tcb
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			sem_post(&tripulante->semaforo);//le permitimos ir a ready
			break;

		case EXEC:

			tripulante->estado = EXEC;
			lock(mutexListaExec);
			list_add(listaExec,(void*) tripulante);
			unlock(mutexListaExec);
			sem_post(&tripulante->semaforo);

			//¿Aca se mandaria el mensaje para actualiazar el tcb?
			break;

		case BLOCKED:

			break;

		default:
				printf("\n No se reconoce el siguiente estado \n");
				exit(1);
	}

}

/*
void sacarDeNew(t_tripulante* tripulante) {

	t_tripulante* tripulanteParaChequear;

		bool idIgualA(t_tripulante* tripulanteAcomparar){

			return tripulanteAcomparar->idTripulante == tripulante->idTripulante;
		}

	lock(mutexListaNew);
	tripulanteParaChequear = list_remove_by_condition(listaDeNew, (void*) idIgualA);
	unlock(mutexListaNew);


}
*/

void hiloTripulante(t_tripulante* tripulante) {

	recibirPrimerTareaDeMiRAM(tripulante);



    if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") != 0){
	    	log_info(logDiscordiador,"Soy el tripulante %d y recibi la tarea de: %s \n",
	    			tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	    	pasarDeEstado(tripulante, READY);

	    	log_info(logDiscordiador,"La tarea %s del tripulante %d esta en Ready", tripulante->instruccionAejecutar->nombreTarea,tripulante->idTripulante);

	    }
	    else{
	    	log_info(logDiscordiador,"Soy el tripulante %d y se me acabaron las tareas!\n",
	    		    tripulante->idTripulante);

	    }



	//while(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") != 0){

		//int iMongoSocket = iniciarConexionDesdeClienteHacia(puertoEIPMongo);
		//mandarTareaAejecutar(tripulante,iMongoSocket);
		//close(iMongoSocket);

		//recibirProximaTareaDeMiRAM(tripulante);
	//}

}

void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

	if(paqueteRecibido->codigo_operacion == TAREA){

	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);


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

	char* string = malloc(paquete->buffer->size);

	memcpy(string,(paquete->buffer->stream),paquete->buffer->size);

	return string;
}


void recibirConfirmacionDeMongo(int socketMongo, t_tarea* tarea){

	t_paquete* paqueteRecibido = recibirPaquete(socketMongo);

	char* mensajeRecibido = deserializarString(paqueteRecibido);
	log_info(logDiscordiador,"Confirmacion %s\n",mensajeRecibido);

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

	eliminarPaquete(paqueteRecibido);

}


