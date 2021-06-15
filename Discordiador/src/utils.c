#include "utils.h"

void modificarTripulanteBlocked(int numero){
	lock(mutexIdTripulanteBlocked);
	idTripulanteBlocked = numero;
	unlock(mutexIdTripulanteBlocked);
}

int leerTripulanteBlocked(){
	lock(mutexIdTripulanteBlocked);
	int id = idTripulanteBlocked;
	unlock(mutexIdTripulanteBlocked);
	return id;
}


void iniciarTareasIO(){
	todasLasTareasIO = malloc(sizeof(char*) * 7);
	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");
	todasLasTareasIO[6] = NULL;
}

void iniciarColas(){
	colaNew = queue_create();
	colaReady = queue_create();
	colaExec = queue_create();
	colaBlocked = queue_create();
	colaEnd = queue_create();
}

void iniciarSemaforos(){
	sem_init(&semPlanificacion,0,1);
	sem_init(&semaforoPlanificadorInicio,0,0);
	sem_init(&semaforoPlanificadorFin,0,0);
}

void iniciarMutex(){
	pthread_mutex_init(&mutexColaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
	pthread_mutex_init(&mutexColaExec, NULL);
	pthread_mutex_init(&mutexColaBlocked, NULL);
	pthread_mutex_init(&mutexColaEnd, NULL);
	pthread_mutex_init(&mutexPlanificadorFin, NULL);
	pthread_mutex_init(&mutexLogDiscordiador, NULL);
	pthread_mutex_init(&mutexTotalTripus, NULL);
	pthread_mutex_init(&mutexIdTripulanteBlocked, NULL);


}

void cargar_configuracion(){

	puertoEIPRAM = malloc(sizeof(puertoEIP));
	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ");
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ"));

	puertoEIPMongo = malloc(sizeof(puertoEIP));
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE");
	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE"));

	gradoMultiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	char * algoritmo;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	if(strcmp(algoritmo,"FIFO")==0){
		quantum = SIN_QUANTUM;
	}
	else{
		quantum =  config_get_int_value(config,"QUANTUM");
	}
	free(algoritmo);

	//duracionSabotaje = config_get_int_value(config,"DURACION_SABOTAJE");
	//retardoCiclosCPU = config_get_int_value(config,"RETARDO_CICLO_CPU");

	//planificacion = CORRIENDO;

}


int leerTotalTripus(){
	lock(mutexTotalTripus);
	int total = totalTripus;
	unlock(mutexTotalTripus);
	return total;
}


void crearConfig(){

	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");

	if(config == NULL){

		log_error(logDiscordiador, "La ruta es incorrecta ");

		exit(1);
	}
}

char * pathLog(){
	char *pathLog = string_new();
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/Discordiador/logs/");
	string_append(&pathLog, "log ");
	string_append(&pathLog, temporal_get_string_time("%d-%m-%y %H:%M:%S"));
	string_append(&pathLog, ".log");
	return pathLog;
}

t_tripulante* elTripuMasCerca(t_coordenadas lugarSabotaje){
	/*
	 * TODO devuelve el tripu mas cerca a una cierta posicion.
	 * Hay q recorrer todas las colas e ir comparando las posiciones
	 * de los tripus hasta q quede un solo tripu. De las colas no
	 * hay q sacar nada, solo estamos leyendo. Es decir q las
	 * colas deben quedar como estaban.
	 */
	//planificacionPlay = 0;//alguien tiene que pausar antes de invocar esta funcion
	int diferenciaX  = 0;
	int diferenciaY = 0;
	int diferenciaMasCerca = 0;
	int diferenciaComparado = 0;

	t_list * listaDeComparacion = list_create();
	list_add_all(listaDeComparacion, colaReady->elements);
	queue_size(colaReady);
	list_add_all(listaDeComparacion, colaExec->elements);
	queue_size(colaExec);
	printf("\n tamaño lista: %d\n",list_size(listaDeComparacion));
	t_list_iterator * iterator = list_iterator_create(listaDeComparacion);
	t_tripulante * tripulante;
	t_tripulante * tripulanteMasCerca;


	tripulanteMasCerca = (t_tripulante *) list_iterator_next(iterator);
	printf("\n %p \n",tripulanteMasCerca);
	diferenciaX = diferencia(tripulanteMasCerca->posX, lugarSabotaje.posX);
	diferenciaY = diferencia(tripulanteMasCerca->posY, lugarSabotaje.posY);

	diferenciaMasCerca = diferenciaX + diferenciaY;
	printf("\ndiferencia : %d de tripulanteid: %d\n", diferenciaMasCerca, tripulanteMasCerca->idTripulante);
	while(list_iterator_has_next(iterator)){


		tripulante = (t_tripulante *) list_iterator_next(iterator);
		diferenciaX = diferencia(tripulante->posX, lugarSabotaje.posX);
		diferenciaY = diferencia(tripulante->posY, lugarSabotaje.posY);

		diferenciaComparado = diferenciaX + diferenciaY;
		printf("\ndiferencia: %d de tripulanteid: %d\n", diferenciaComparado, tripulante->idTripulante);
		//s: x=0,y=0
		//t4: x=1,y=1
		//t2: x=1,y=0-1
		//t1: x=6,y=6

		if(diferenciaComparado < diferenciaMasCerca || (diferenciaComparado == diferenciaMasCerca && tripulante->idTripulante < tripulanteMasCerca->idTripulante)){
			printf("\nel tripulanteid: %d con posX: %d posY: %d desaloja al el tripulanteid: %d con posX: %d posY: %d\n"
			, tripulante->idTripulante, tripulante->posX, tripulante->posY,
			tripulanteMasCerca->idTripulante, tripulanteMasCerca->posX, tripulanteMasCerca->posY);

			diferenciaMasCerca = diferenciaComparado;
			tripulanteMasCerca =  tripulante;
		}
	}
	list_iterator_destroy(iterator);
	list_destroy(listaDeComparacion);
	return tripulanteMasCerca;

}
void esperarTerminarTripulante(t_tripulante* tripulante){
	sem_wait(&tripulante->semaforoFin);
}


void avisarTerminoPlanificacion(t_tripulante* tripulante){
	sem_post(&tripulante->semaforoInicio);
}

void hiloPlani(){
	while(1){
		if(planificacion == CORRIENDO && leerTotalTripus() > 0){

			list_iterate(colaExec->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaBlocked->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaNew->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaReady->elements, (void*)esperarTerminarTripulante);

			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripus);
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");

			actualizarCola(EXEC, colaExec, mutexColaExec);
			actualizarCola(BLOCKED, colaBlocked, mutexColaBlocked);
			casoBlocked();
			actualizarCola(NEW, colaNew, mutexColaNew);
			actualizarCola(READY, colaReady, mutexColaReady);

/*
			if(haySabotaje){ // HAY SABOTAJE
				tripulanteDesabotaje = elTripuMasCerca(sabotaje.coordenadas);
				// el sabotaje es una variable global de tipo t_tarea y tripulanteDesabotaje tambien es global
				t_estado estadoAnterior = imagenTripu->estado;
				t_estado tareaAnterior = imagenTripu->instruccionAejecutar;
				tripulanteDesabotaje->estado = SABOTAJE;
				tripulanteDesabotaje->instruccionAejecutar = sabotaje;
				sem_wait(&semaforoSabo);
				tripulanteDesabotaje->estado = estadoAnterior;
				tripulanteDesabotaje->instruccionAejecutar = tareaAnterior;
//				tripulanteDesabotaje tiene q ponerse como en un null, un tripu null
				noHaySabotaje = 1;
				sem_post(semaforoImagen); // se inicializa en 0
				sem_post(&semaforoSabotajeResuelto); //se usa para indicar al hiloSabotaje
				// q ya se termino. Mas q nada por si nos mandan dos sabos juntos
			}

*/

//			planificadorFin = 0;

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

			list_iterate(colaExec->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaBlocked->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaNew->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaReady->elements, (void*)avisarTerminoPlanificacion);

		}

	}

}


void hilitoSabo(){

	while(1){
		/*
		 * TODO la parte de recibir un saboteje, osea un recv y se
		 * deserializa para ver si llego un sabotaje en el caso de
		 * que si, se hace lo siguiente:
		 */

		haySabotaje = 1;
//		sem_wait(&semaforoSabotajeResuelto);
	}
}


void hiloTripu(t_tripulante* tripulante){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	//int ciclosSabo = 0;
	int quantumPendiente = quantum;
	while(tripulante->estado != END){

		if(tripulante->estado != NEW){
			actualizarEstadoEnRAM(tripulante);
		}
			//LO PUSE ACA POR EL PROBLEMA DE Q SI LO PONGO ABAJO LE VA A
		//MANDAR UN ESTDO INCORRECTO EN EL CADO DE QUE NO PUEDA ENTRAR A EXEC
		//EL OTRO PROBLEMA ES QUE LA PRIMERA VEZ LE VA ENVIAR UN TIPU SIN TAREA A RAM
		switch(tripulante->estado){
			case NEW:
				log_info(logDiscordiador,"tripulanteId %d: estoy en new", tripulante->idTripulante);
				recibirPrimerTareaDeMiRAM(tripulante);
				tripulante->estado = READY;
				break;
			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
				log_info(logDiscordiador,"tripulanteId %d: estoy en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec);
				tripulante->estado = EXEC;
				break;
			case EXEC:
				log_info(logDiscordiador,"tripulanteId %d: estoy en exec con %d ciclos y %d quantum",
						tripulante->idTripulante, ciclosExec, quantumPendiente);
				desplazarse(tripulante);
				ciclosExec --;
				quantumPendiente--;
				sleep(1);
				if(quantumPendiente == 0){
					tripulante->estado = READY;
				}
				if(ciclosExec <= 0){
					if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
						int socketMongo = enviarA(puertoEIPMongo, tripulante->instruccionAejecutar, TAREA);
						close(socketMongo);
						ciclosBlocked = tripulante->instruccionAejecutar->tiempo;
						tripulante->estado = BLOCKED;
					}
					else{
						siguienteTarea(tripulante, &ciclosExec);
					}
				}
				break;
			case BLOCKED:
				if(tripulante->idTripulante == idTripulanteBlocked){
					sleep(1);
					lock(mutexLogDiscordiador);
					log_info(logDiscordiador,"tripulanteId %d: estoy en block ejecutando, me quedan %d ciclos",
							tripulante->idTripulante, ciclosBlocked);
					unlock(mutexLogDiscordiador);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						idTripulanteBlocked = NO_HAY_TRIPULANTE_BLOQUEADO;
						tripulante->estado = READY;
						siguienteTarea(tripulante, &ciclosExec);
					}
				}
				break;
			case END:
				log_error(logDiscordiador,"----tripulanteId %d: no deberia estar aca----",
						tripulante->idTripulante);
				break;
			case SABOTAJE: // para algunos es como un bloqueo donde no se hace nada
/*				if(tripu->idTripulante == tripulanteDesabotaje->idTripulante){
					sleep(1);
					ciclosSabo --;
					if(ciclosSabo > 0){
						desplazarse(tripu);
					}
					else{

					 *    FIN DE SABO
					 * hay q pasar a todos en su respectivo orden a sus estados anteriores
					 * y hacer q el tripu q se movio hasta para solucionar el sabo re calcule
					 * sus cilos de ejecucion teniendo en cuenta q se tiene q volver a
					 * desplazar y q puede ser q ya haya hecho parte de la tarea

					sem_post(semaforoSabo); //este semaforo le permite recuperar su "imagen"
					//el wait se hace dentro del hiloSabotaje antes de q devolverle la imagen
					//y el semaforo se inicializa en 0
					sem_wait(semaforoImagen); //lo hago para asegurarme q le devuelva la imagen
					}
				}
				sem_post(semaforoPlanificadorInicio);
*/
				break;
		}
		sem_post(&tripulante->semaforoFin);
		sem_wait(&tripulante->semaforoInicio);
	}
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	esperarConfirmacionDeRAM(miRAMsocket);
	log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

	for (int i=0; i<cantidadTripulantes; i++){
		totalTripus ++;
		log_info(logDiscordiador,"---------posx:%d;posy:%d---------",coordenadas[i].posX,coordenadas[i].posY);
		iniciarTripulante(*(coordenadas+i), patota->ID);
	}
	free(patota);
	close(miRAMsocket);
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
	tripulante->instruccionAejecutar = malloc(sizeof(t_tarea*));
	tripulante->instruccionAejecutar->nombreTarea = malloc(sizeof(char*));
	sem_init(&tripulante->semaforoInicio, 0, 0);
	sem_init(&tripulante->semaforoFin, 0, 0);


	lock(mutexColaNew);
	queue_push(colaNew,(void*)tripulante);
	unlock(mutexColaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d",tripulante->idTripulante);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripu, (void*) tripulante);
	pthread_detach(_hiloTripulante);
}


t_patota* asignarDatosAPatota(char* tareasString){

	t_patota* patota = malloc(sizeof(t_patota));

	patota->tamanioTareas = strlen(tareasString) + 1;
	idPatota++;
	patota->ID = idPatota;
	patota->tareas = tareasString;

	log_info(logDiscordiador,"Se creo la patota numero %d",idPatota);
	return patota;
}


void casoBlocked(){
	if(idTripulanteBlocked == NO_HAY_TRIPULANTE_BLOQUEADO && queue_size(colaBlocked) > 0){
		lock(mutexColaBlocked);
		t_tripulante* tripulanteBlocked = (t_tripulante*) queue_peek(colaBlocked);
		unlock(mutexColaBlocked);
		idTripulanteBlocked = tripulanteBlocked->idTripulante;
		log_info(logDiscordiador,"------EL TRIPU BLOQUEADO ES %d-----", idTripulanteBlocked);
	}
}


void actualizarCola(t_estado estado, t_queue* cola, pthread_mutex_t colaMutex){

	t_tripulante* tripulante;
	int tamanioInicialCola = queue_size(cola);
	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----", traducirEstado(estado), tamanioInicialCola);

	for(int i=0; i<tamanioInicialCola; i++){
		lock(colaMutex);
		tripulante = (t_tripulante*) queue_pop(cola);
		unlock(colaMutex);

		if(tripulante->estado != estado /*|| tripulante->estado == END*/){
			if(estado == READY){
				if(queue_size(colaExec) < gradoMultiprocesamiento){
					pasarDeCola(tripulante);
				}
				else{
					tripulante->estado = estado;
					lock(colaMutex);
					queue_push(cola, tripulante);
					unlock(colaMutex);
				}
			}
			else{
				pasarDeCola(tripulante);
			}
		}
		else{
			tripulante->estado = estado;
			lock(colaMutex);
			queue_push(cola, tripulante);
			unlock(colaMutex);
		}

		//sem_post(&tripulante->semaforoInicio);
	}
}


void siguienteTarea(t_tripulante* tripulante, int* ciclosExec){
	recibirProximaTareaDeMiRAM(tripulante);
	if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") == 0){
		lock(mutexTotalTripus);
		totalTripus --;
		unlock(mutexTotalTripus);
		tripulante->estado = END;
		log_info(logDiscordiador,"Tripulante id: %d ya no le quedan tareas por hacer", tripulante->idTripulante);
	}
	else{
		*ciclosExec = calculoCiclosExec(tripulante);
	}
}


void iterarCola(t_queue* cola){

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

	while(list_iterator_has_next(list_iterator)) {
		t_tripulante* tripulante = list_iterator_next(list_iterator);
		char* status = traducirEstado(tripulante->estado);
		//TODO hacerlo log
		log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Status:%s    ", tripulante->idTripulante, tripulante->idPatota, status);
		free(status);
	}

	list_iterator_destroy(list_iterator);

}

void pasarDeCola(t_tripulante* tripulante){
	switch(tripulante->estado){
		case READY:
			lock(mutexColaReady);
			queue_push(colaReady, tripulante);
			unlock(mutexColaReady);
			log_info(logDiscordiador,"El tripulante %d paso a COLA READY", tripulante->idTripulante);
			break;

		case EXEC:
			lock(mutexColaExec);
			queue_push(colaExec, tripulante);
			unlock(mutexColaExec);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC", tripulante->idTripulante);
			break;

		case BLOCKED:
			lock(mutexColaBlocked);
			queue_push(colaBlocked, tripulante);
			unlock(mutexColaBlocked);
			log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED", tripulante->idTripulante);
			break;

		case END:
			enviarA(puertoEIPRAM, tripulante, EXPULSAR);
			liberarTripulante(tripulante);
			break;

		default:
			log_error(logDiscordiador,"No se reconoce el estado", tripulante->idTripulante);
			exit(1);
	}

}

char* deserializarString (t_paquete* paquete){

	char* string = malloc(paquete->buffer->size);
	memcpy(string,(paquete->buffer->stream),paquete->buffer->size);

	return string;
}

void mandarTareaAejecutar(t_tripulante* tripulante, int socketMongo){

	t_paquete* paqueteConLaTarea = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);

	enviarPaquete(paqueteConLaTarea,socketMongo);
}

void actualizarEstadoEnRAM(t_tripulante* tripulante){

//	log_info(logDiscordiador,"Se manda a actualizar el tripulante de ID: %d",tripulante->idTripulante);
	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, ESTADO_TRIPULANTE);
//	log_info(logDiscordiador,"El tripulante de ID %d, esta enviando paquete con cod_op %d",
//			tripulante->idTripulante, paqueteAenviar->codigo_operacion);

	esperarConfirmacionDeRAM(miRAMsocket);

	close(miRAMsocket);
}

void recibirPrimerTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, TRIPULANTE);

	log_info(logDiscordiador, "tripulanteId: %d envie a MIRAM mi info principal",
			tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);
	close(miRAMsocket);
}

void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, SIGUIENTE_TAREA);

	log_info(logDiscordiador, "TRIPULANTE: %d - VOY A BUSCAR PROX TAREA A MI RAM",
		    			tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket,tripulante);
	close(miRAMsocket);
}

void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);
	log_info(logDiscordiador, "--------------------------codigoOp es: %d -----------------------------", paqueteRecibido->codigoOperacion);
	if(paqueteRecibido->codigoOperacion == TAREA){//revisar
		free(tripulante->instruccionAejecutar->nombreTarea);
		free(tripulante->instruccionAejecutar);
	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);


	    log_info(logDiscordiador, "TRIPULANTE: %d - recibi la tarea %s de MIRAM",
	    			tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);

	}
	else{

	    log_error(logDiscordiador,"El paquete recibido no es una tarea");

	    exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}

int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion){
	int socket = iniciarConexionDesdeClienteHacia(puerto);
	enviarPaquete(armarPaqueteCon(informacion, codigoOperacion), socket);
	return socket;
	//----ESTA FUNCION NO LLEVA EL CLOSE, PERO HAY Q AGREGARLO SIEMPRE
}

void esperarConfirmacionDeRAM(int server_socket) {

	t_paquete* paqueteRecibido = recibirPaquete(server_socket);

	char* mensajeConfirmacion = (char*) paqueteRecibido->buffer->stream;

	if(paqueteRecibido->codigoOperacion == STRING && string_contains(mensajeConfirmacion,"OK")){
		//log_info(logDiscordiador, "Nos llego la confirmacion de MI-RAM de: %s", mensajeConfirmacion);
	}
	else {
		//log_error(logDiscordiador,"Nunca llego la confirmacion de RAM");
		exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}

void recibirConfirmacionDeMongo(int socketMongo, t_tarea* tarea){

	t_paquete* paqueteRecibido = recibirPaquete(socketMongo);

	char* mensajeRecibido = deserializarString(paqueteRecibido);

	//log_info(logDiscordiador,"Confirmacion %s",mensajeRecibido);


	if(strcmp(mensajeRecibido,"TAREA REALIZADA") == 0){

		log_info(logDiscordiador,"Se elimino la tarea %s",tarea->nombreTarea);

	}
	else{

		log_info(logDiscordiador, "No sabemos q hacer todavia =(");

	}
	free(tarea->nombreTarea);
	free(tarea);
	free(mensajeRecibido);

	eliminarPaquete(paqueteRecibido);

}

int esIO(char* tarea){

	for(int i=0; todasLasTareasIO[i] != NULL; i++){
		if(strcmp(todasLasTareasIO[i],tarea) == 0){
			return 1;
		}
	}
	log_info(logDiscordiador,"La tarea %s no es IO",tarea);
	return 0;
}

uint32_t calculoCiclosExec(t_tripulante* tripulante){

	uint32_t desplazamientoEnX = diferencia(tripulante->instruccionAejecutar->posX, tripulante->posX);
	uint32_t desplazamientoEnY = diferencia(tripulante->instruccionAejecutar->posY, tripulante->posY);

	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return  desplazamientoEnX + desplazamientoEnY + 1;
	}

	return desplazamientoEnX + desplazamientoEnY + tripulante->instruccionAejecutar->tiempo;
}

void desplazarse(t_tripulante* tripulante){
	//NO ENTIENDOOOOOOO
	//LE SAQUE LOS UINT Y EMPEZO A ANDAR EL MOVIMIENTO CON COORDENADAS DE CONSOLA
	int diferenciaEnX = diferencia(tripulante->posX, tripulante->instruccionAejecutar->posX);
	int restaEnX = tripulante->posX - tripulante->instruccionAejecutar->posX;
	int restaEnY = tripulante->posY - tripulante->instruccionAejecutar->posY;
	int diferenciaEnY = diferencia(tripulante->posY, tripulante->instruccionAejecutar->posY);
	int desplazamiento = 0;
	//FALTAN MUTEX

	log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",
			tripulante->posX, tripulante->posY );

	if(diferenciaEnX){

		desplazamiento = restaEnX / diferenciaEnX;
		//log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
		//			tripulante->posX, tripulante->instruccionAejecutar->posX, restaEnX, diferenciaEnX, desplazamiento);

		tripulante->posX = tripulante->posX - desplazamiento;
	}
	else if(diferenciaEnY){

		desplazamiento = restaEnY / diferenciaEnY;
		//log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
		//			tripulante->posY, tripulante->instruccionAejecutar->posY, restaEnY, diferenciaEnY, desplazamiento);

		tripulante->posY = (tripulante->posY - desplazamiento);
	}

	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY);

}

uint32_t diferencia(uint32_t numero1, uint32_t numero2){
	return (uint32_t) abs(numero1-numero2);
}

void listarTripulantes(){
	//TODO hacerlo log
	//pausarPlanificacion = 0;
	log_info(logDiscordiador,"Estado de la nave: %s ", temporal_get_string_time("%d-%m-%y %H:%M:%S"));

	iterarCola(colaNew);
	iterarCola(colaReady);
	iterarCola(colaExec);
	iterarCola(colaBlocked);

}

char* traducirEstado(t_estado estado){

	char* string;

	switch(estado){
		case NEW:
			string = strdup("New");
			break;
		case READY:
			string = strdup("Ready");
			break;
		case EXEC:
			string = strdup("Exec");
			break;
		case BLOCKED:
			string = strdup("Blocked");
			break;
		case END:
			string = strdup("End");
			break;
		case SABOTAJE:
			string = strdup("Sabotaje");
			break;
	}

	return string;
}

int deleteTripulante(uint32_t id, t_queue* cola){

	int cantidad = 0;

	t_tripulante * tripulante = malloc(sizeof(t_tripulante));

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

	while(list_iterator_has_next(list_iterator)) {

		tripulante = list_iterator_next(list_iterator);

		if(tripulante->idTripulante == id){

			tripulante->estado = END;

			cantidad++;
		}

	}

	list_iterator_destroy(list_iterator);

	return cantidad;

}

void eliminarTripulante(uint32_t id){


	int vector[3];
	int resultado = 0;

	vector[0]= deleteTripulante(id,colaNew);
	vector[1]= deleteTripulante(id,colaReady);
	vector[2]= deleteTripulante(id,colaExec);

	for(int i=0; i<3; i++){
		resultado += vector[i];
	}
	if(resultado == 0){
		log_info(logDiscordiador,"No se ha encontrado un tripulante con el Id: %d",id);
	}
	if(resultado == 1){
		log_info(logDiscordiador,"Se va a eliminar el tripulante con el Id: %d",id);
	}
	else{
		log_info(logDiscordiador,"Esta funcionando mal eliminarTripulante negro");
		exit(1);
	}

}

void eliminarPatota(t_patota* patota){
	free(patota->tareas);
	free(patota);
}

void liberarTripulante(t_tripulante* tripulante){
	log_info(logDiscordiador, "Eliminando el tripulante: %d, y su ultima tarea fue: %s",tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	free(tripulante->instruccionAejecutar->nombreTarea);
	free(tripulante->instruccionAejecutar);
	free(tripulante);
}
