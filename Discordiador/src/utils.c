#include "utils.h"

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

}

void cargar_configuracion(){

	puertoEIPRAM = malloc(sizeof(puertoEIP));
	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ");
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ"));

	puertoEIPMongo = malloc(sizeof(puertoEIP));
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE");
	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE"));

	gradoMultiprocesamiento = 1;
	//gradoMultiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	char * algoritmo;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	if(strcmp(algoritmo,"FIFO")){
		quantum = -1;
	}
	else{
		quantum =  config_get_int_value(config,"QUANTUM");
	}
	free(algoritmo);

	//duracionSabotaje = config_get_int_value(config,"DURACION_SABOTAJE");
	//retardoCiclosCPU = config_get_int_value(config,"RETARDO_CICLO_CPU");

	planificacionPlay = 1;

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

		log_error(logDiscordiador, "\n La ruta es incorrecta \n");

		exit(1);
	}
}

//t_tripulante* elTripuMasCerca(t_coordenadas lugarSabotaje){
	/*
	 * TODO devuelve el tripu mas cerca a una cierta posicion.
	 * Hay q recorrer todas las colas e ir comparando las posiciones
	 * de los tripus hasta q quede un solo tripu. De las colas no
	 * hay q sacar nada, solo estamos leyendo. Es decir q las
	 * colas deben quedar como estaban.
	 */

//}

void hiloPlani(){
	while(1 && leerTotalTripus() != 0){
		if(planificacionPlay && leerTotalTripus() > 0){

			for(int i=0; i <leerTotalTripus(); i++){
				sem_wait(&semaforoPlanificadorInicio);
			}


			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripus);
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");


			actualizarCola(EXEC, colaExec, mutexColaExec);
			actualizarCola(BLOCKED, colaBlocked, mutexColaBlocked);
			actualizarCola(NEW, colaNew, mutexColaNew);
			actualizarCola(READY, colaReady, mutexColaReady);
			actualizarCola(END, colaEnd, mutexColaEnd);
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

			for(int i=0; i<leerTotalTripus(); i++){
				sem_post(&semaforoPlanificadorFin);
			}

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

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
	int tripuVivo = 1;
	while(tripuVivo){
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

				log_info(logDiscordiador,"tripulanteId %d: estoy en exec con %d ciclos", tripulante->idTripulante, ciclosExec);

				desplazarse(tripulante);
				ciclosExec --;
				quantumPendiente--;
				sleep(1);
				if(!(ciclosExec > 0 && quantumPendiente != 0)){
					if(quantumPendiente == 0){
						tripulante->estado = READY;
					}
					else{
						if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
							int socketMongo = iniciarConexionDesdeClienteHacia(puertoEIPMongo);
							mandarTareaAejecutar(tripulante,socketMongo);
							ciclosBlocked = tripulante->instruccionAejecutar->tiempo;
							tripulante->estado = BLOCKED;
						}
						//Esta mal esto
						recibirProximaTareaDeMiRAM(tripulante);
						if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") == 0){
							tripulante->estado = END;
						}
						else{
							ciclosExec = calculoCiclosExec(tripulante);
						}
					}
				}
				break;
			case BLOCKED:
				//if(tripulante->idTripulante == idTripulanteBlocked){

					sleep(1);

					log_info(logDiscordiador,"tripulanteId %d: estoy en block ejecutando, me quedan %d cilos",
							tripulante->idTripulante, ciclosBlocked);

					ciclosBlocked --;
					if(ciclosBlocked == 0){
						idTripulanteBlocked = -1;
						tripulante->estado = READY;
					}
				//}

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
			case END:
				lock(mutexTotalTripus);
				totalTripus--;
				unlock(mutexTotalTripus);
				tripuVivo = 0;

				log_info(logDiscordiador,"tripulanteId %d: estoy en end, YA TERMINE. Ahora quedan %d tripus",
						tripulante->idTripulante, totalTripus);
				//free tripulantes
				// el unico caso donde no se hace un post al inicio del plani
				// esta asi porq como arriba se hace un totalTripu --, el wait
				// del planiInicio va a hacer una iteracion menos
				sem_post(&semaforoPlanificadorInicio);

				break;
		}
		actualizarEstadoEnRAM(tripulante);
		if(tripulante->estado != END){
			sem_post(&semaforoPlanificadorInicio);

			log_info(logDiscordiador,"tripulanteId %d: ESPERANDO QUE EL PLANI TERMINE",
					tripulante->idTripulante);

			sem_wait(&semaforoPlanificadorFin);

			log_info(logDiscordiador,"tripulanteId %d: YA TERMINO EL PLANI, AHORA CONTINUO",
					tripulante->idTripulante);

		}
	}
}

void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	esperarConfirmacionDeRAM(miRAMsocket);
	log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

	for (int i=0; i<cantidadTripulantes; i++){
		totalTripus ++;
		log_info(logDiscordiador,"---------------posx:%d;posy:%d---------------",coordenadas[i].posX,coordenadas[i].posY);
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
	sem_init(&tripulante->semaforo, 0, 0);
//	el tripulante no necesita un semaforo

	lock(mutexColaNew);
	queue_push(colaNew,(void*)tripulante);
	unlock(mutexColaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d\n",tripulante->idTripulante);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripu, (void*) tripulante);
	pthread_detach(_hiloTripulante);
}

t_patota* asignarDatosAPatota(char* tareasString){

	t_patota* patota = malloc(sizeof(t_patota));

	patota->tamanioTareas = strlen(tareasString) + 1;
	idPatota++;
	patota->ID = idPatota;
	patota->tareas = tareasString;

	log_info(logDiscordiador,"Se creo la patota numero %d\n",idPatota);
	return patota;
}

void actualizarCola(t_estado estado, t_queue* cola, pthread_mutex_t colaMutex){

	t_tripulante* tripulante;
	int tamanioInicialCola = queue_size(cola);
	char * state = traducirEstado(estado);
	log_info(logDiscordiador,"------El tamanio inicial de la cola de %s es de %d-----", state, tamanioInicialCola);
	free(state);

	for(int i=0; i<tamanioInicialCola; i++){
		lock(colaMutex);
		tripulante = (t_tripulante*) queue_pop(cola);
		unlock(colaMutex);
		//if(queue_size(colaExec) >= gradoMultiprocesamiento && tripulante->estado == EXEC){
			//tripulante->estado = estado;
		//}
		if(tripulante->estado != estado){
			pasarDeCola(tripulante);
		}
		else{
			lock(colaMutex);
			queue_push(cola, tripulante);
			unlock(colaMutex);
		}
	}

}

void iterarCola(t_queue* cola){

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

	while(list_iterator_has_next(list_iterator)) {
		t_tripulante* tripulante = list_iterator_next(list_iterator);
		char* status = traducirEstado(tripulante->estado);
		printf("Tripulante: %d    Patota: %d    Status:%s    \n", tripulante->idTripulante, tripulante->idPatota, status);
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
			log_info(logDiscordiador,"El tripulante %d paso a COLA READY \n", tripulante->idTripulante);
			break;

		case EXEC:
			lock(mutexColaExec);
			queue_push(colaExec, tripulante);
			unlock(mutexColaExec);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC \n", tripulante->idTripulante);
			break;

		case BLOCKED:
			lock(mutexColaBlocked);
			queue_push(colaBlocked, tripulante);
			unlock(mutexColaBlocked);
			log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED \n", tripulante->idTripulante);
			break;

		case END:
			lock(mutexColaEnd);
			queue_push(colaEnd, tripulante);
			unlock(mutexColaEnd);
			log_info(logDiscordiador,"El tripulante %d paso a COLA END \n", tripulante->idTripulante);
			break;

		default:
			printf("\n No se reconoce el siguiente estado \n");
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

	    log_error(logDiscordiador,"El paquete recibido no es una tarea\n");

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
		log_info(logDiscordiador, "Nos llego la confirmacion de MI-RAM de: %s", mensajeConfirmacion);
	}
	else {
		log_error(logDiscordiador,"Nunca llego la confirmacion de RAM");
		exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}

void recibirConfirmacionDeMongo(int socketMongo, t_tarea* tarea){

	t_paquete* paqueteRecibido = recibirPaquete(socketMongo);

	char* mensajeRecibido = deserializarString(paqueteRecibido);

	log_info(logDiscordiador,"Confirmacion %s\n",mensajeRecibido);


	if(strcmp(mensajeRecibido,"TAREA REALIZADA") == 0){

		log_info(logDiscordiador,"Se elimino la tarea %s\n",tarea->nombreTarea);

	}
	else{

		log_info(logDiscordiador, "No sabemos q hacer todavia =(\n");

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
		log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
					tripulante->posX, tripulante->instruccionAejecutar->posX, restaEnX, diferenciaEnX, desplazamiento);

		tripulante->posX = tripulante->posX - desplazamiento;
	}
	else if(diferenciaEnY){

		desplazamiento = restaEnY / diferenciaEnY;
		log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
					tripulante->posY, tripulante->instruccionAejecutar->posY, restaEnY, diferenciaEnY, desplazamiento);

		tripulante->posY = (tripulante->posY - desplazamiento);
	}

	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY);

}

uint32_t diferencia(uint32_t numero1, uint32_t numero2){
	return (uint32_t) abs(numero1-numero2);
}

void listarTripulante(){

	printf("Estado de la nave: %d \n", system("date"));

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

t_eliminado* deleteTripulante(uint32_t id, t_queue* cola){

	t_eliminado* eliminado = malloc(sizeof(t_eliminado));

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

	while(list_iterator_has_next(list_iterator)) {

		eliminado->tripulante = list_iterator_next(list_iterator);

		if(eliminado->tripulante->idTripulante == id){

			eliminado->tripulante->estado = END;
			lock(mutexColaEnd);
			queue_push(colaEnd,eliminado->tripulante);
			unlock(mutexColaEnd);
			eliminado->cantidad++;
		}

	}

	list_iterator_destroy(list_iterator);

	return eliminado;

}

void eliminarTripulante(uint32_t id){

	planificacionPlay = 0;

	t_eliminado* vector[2];
	int resultado = 0;
	int flag = 0;

	vector[0]= deleteTripulante(id,colaNew);
	vector[1]= deleteTripulante(id,colaReady);
	vector[2]= deleteTripulante(id,colaExec);

	for(int i=0; i<3; i++){
		resultado += vector[i]->cantidad;
		if(vector[i]->cantidad == 1){
			flag = i;
		}
	}
	if(resultado == 0){

		log_info(logDiscordiador,"No se ha encontrado un tripulante con el Id: %d \n",id);

		planificacionPlay = 1;

	}
	if(resultado == 1){

		int socketMiRAM = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

		t_paquete* paquete = armarPaqueteCon(vector[flag],EXPULSAR);

		enviarPaquete(paquete,socketMiRAM);

		log_info(logDiscordiador,"Se ha eliminado el tripulante con el Id: %d \n",id);

		planificacionPlay = 1;

	}
	else{
		log_info(logDiscordiador,"Esta funcionando mal eliminarTripulante negro \n");
		exit(1);
	}

}

void eliminarPatota(t_patota* patota){
	free(patota->tareas);
	free(patota);
}

void liberarTripulante(t_tripulante* tripulante){
	log_info(logDiscordiador, "Eliminando de la colaEnd el tripulante: %d, y su ultima tarea fue: %s",tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	free(tripulante->instruccionAejecutar->nombreTarea);
	free(tripulante->instruccionAejecutar);
	free(tripulante);
}

void liberarColaEnd(){
	log_info(logDiscordiador,"Quedaron estos elementos en la cola end: %d, que se van a eliminar",queue_size(colaEnd));
	log_info(logDiscordiador,"Destruyendo colaEnd y tripulantes");
	queue_destroy_and_destroy_elements(colaEnd, (void *)liberarTripulante);
}
