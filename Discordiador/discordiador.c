#include "discordiador.h"

char** todasLasTareasIO;

int main() {

	sem_init(&semPlanificacion,0,1);
	sem_init(&semaforoPlanificadorInicio,0,0);
	sem_init(&semaforoPlanificadorFin,0,0);
	planificacion_play = 1;
	todasLasTareasIO = malloc(sizeof(char*) * 6);

	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");

	idTripulante = 0;
	idPatota = 0;
	gradoMultiprocesamiento = 1;
	quantum = -1;

	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/logDiscordiador.log","Discordiador",1);

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	colaNew = queue_create();
	colaReady = queue_create();
	colaExec = queue_create();
	colaBlocked = queue_create();

	pthread_mutex_init(&mutexColaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
	pthread_mutex_init(&mutexColaExec, NULL);
	pthread_mutex_init(&mutexColaBlocked, NULL);
	pthread_mutex_init(&mutexPlanificadorFin, NULL);
	//
	int tripulantes = 3;
	t_coordenadas coordenadas[tripulantes ];
	for(int i=0; i<tripulantes;i++){
		coordenadas[i].posX = i;
		coordenadas[i].posY = i + 1;
	}
	//CONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2
	iniciarPatota(coordenadas, "GENERAR_OXIGENO 4;2;3;7", tripulantes );
	pthread_t planificador;
	pthread_create(&planificador, NULL, (void*) hiloPlani, NULL);
	pthread_join(planificador, (void**) NULL);

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}



void hiloPlani(){
	while(1){
		if(planificacion_play){

			// hay q rodear al for con un mutex por la lectura de totalTripus?
			for(int i=0; i <totalTripus; i++){
				sem_wait(&semaforoPlanificadorInicio);
			}
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");


			lock(mutexColaExec);
			actualizar(EXEC, colaExec);
			unlock(mutexColaExec);
			lock(mutexColaBlocked);
			actualizar(BLOCKED, colaBlocked);
			unlock(mutexColaBlocked);
			lock(mutexColaNew);
			actualizar(NEW, colaNew);
			unlock(mutexColaNew);
			lock(mutexColaReady);
			actualizar(READY, colaReady);
			unlock(mutexColaReady);


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

			for(int i=0; i<totalTripus; i++){
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
					ciclosExec = calcularCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
				log_info(logDiscordiador,"tripulanteId %d: estoy en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec);
				tripulante->estado = EXEC;
				break;
			case EXEC:
				sleep(1);
				log_info(logDiscordiador,"tripulanteId %d: estoy en exec", tripulante->idTripulante);
				ciclosExec --;
				quantumPendiente--;
				if(ciclosExec > 0 && quantumPendiente != 0){
					desplazarse(tripulante);
				}
				else{
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
						recibirProximaTareaDeMiRAM(tripulante);
						if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") == 0){
							tripulante->estado = END;
						}
						ciclosExec = calcularCiclosExec(tripulante);
					}
				}
				break;
			case BLOCKED:
				if(tripulante->idTripulante == idTripulanteBlocked){
					log_info(logDiscordiador,"tripulanteId %d: estoy en block ejecutando", tripulante->idTripulante);
					sleep(1);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						idTripulanteBlocked = -1;
						tripulante->estado = READY;
					}
				}

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
				totalTripus--; //aca se esta haciendo escritura, en el plani se hace lectura?
				tripuVivo = 0;
				log_info(logDiscordiador,"tripulanteId %d: estoy en end, YA TERMINE", tripulante->idTripulante);
				// el unico caso donde no se hace un post al inicio del plani
				// esta asi porq como arriba se hace un totalTripu --, el wait
				// del planiInicio va a hacer una iteracion menos
				sem_post(&semaforoPlanificadorInicio);
				break;
		}
		if(tripulante->estado != END){
			actualizarEstadoEnRAM(tripulante);
			sem_post(&semaforoPlanificadorInicio);
			log_info(logDiscordiador,"tripulanteId %d: ESPERANDO QUE EL PLANI TERMINE",
					tripulante->idTripulante);
			sem_wait(&semaforoPlanificadorFin);
			log_info(logDiscordiador,"tripulanteId %d: YA TERMINO EL PLANI, AHORA CONTINUO",
					tripulante->idTripulante);
		}
	}
}




int calculoMovimiento(t_tripulante* tripulante){

    int movimientosEnX = fabs(tripulante->instruccionAejecutar->posX - tripulante->posX);
    int movimientosEnY = fabs(tripulante->instruccionAejecutar->posY - tripulante->posY);

    return movimientosEnX + movimientosEnY;
}


int calcularCiclosExec(t_tripulante* tripulante){
	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return calculoMovimiento(tripulante) + 1;
	}
	else{
		return calculoMovimiento(tripulante) + tripulante->instruccionAejecutar->tiempo;
	}
}


void actualizarEstadoEnRAM(t_tripulante* tripulante){

//	log_info(logDiscordiador,"Se manda a actualizar el tripulante de ID: %d",tripulante->idTripulante);
	int socketRam = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete* paqueteAenviar = armarPaqueteCon(tripulante,ESTADO_TRIPULANTE);
//	log_info(logDiscordiador,"El tripulante de ID %d, esta enviando paquete con cod_op %d",
//			tripulante->idTripulante, paqueteAenviar->codigo_operacion);
	enviarPaquete(paqueteAenviar,socketRam);
	close(socketRam);

}

//actualizar(EXEC, colaExec);
void actualizar(t_estado estado, t_queue* cola){

	t_tripulante* tripulante;
	int tamanioInicialCola = queue_size(cola);
	log_info(logDiscordiador,"------El tamanio inicial de la cola de %d es de %d-----", estado, tamanioInicialCola);
	for(int i=0; i<tamanioInicialCola; i++){
		tripulante = (t_tripulante*) queue_pop(cola);
//		log_info(logDiscordiador,"El tripulante de ID %d (que deberia tener id) tiene estado %d",
//				tripulante->idTripulante, tripulante->estado);
		//if(queue_size(colaExec) >= gradoMultiprocesamiento && tripulante->estado == EXEC){
			//tripulante->estado = estado;
		//}
		if(tripulante->estado != estado){
			pasarDeCola(tripulante);
		}
		else{
			queue_push(cola, &tripulante);
		}
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


int calculoCiclosExec(t_tripulante* tripulante){

	int desplazamientoEnX = diferencia(tripulante->instruccionAejecutar->posX, tripulante->posX);
	int desplazamientoEnY = diferencia(tripulante->instruccionAejecutar->posY, tripulante->posY);

	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return  desplazamientoEnX + desplazamientoEnY + 1;
	}

	return desplazamientoEnX + desplazamientoEnY + tripulante->instruccionAejecutar->tiempo;
}


int diferencia(uint32_t numero1, uint32_t numero2){
	return abs(numero1 -numero2);
}


void desplazarse(t_tripulante* tripulante){
	int diferenciaEnX = diferencia(tripulante->posX, tripulante->instruccionAejecutar->posX);
	int diferenciaEnY = diferencia(tripulante->posY, tripulante->instruccionAejecutar->posY);

	//FALTAN MUTEX
	log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",
			tripulante->posX, tripulante->posY );

	if(diferenciaEnX){
		tripulante->posX = tripulante->posX -
				tripulante->instruccionAejecutar->posX / diferenciaEnX;
	}
	else if(diferenciaEnY){
		tripulante->posY = tripulante->posY -
				tripulante->instruccionAejecutar->posY / diferenciaEnY;
	}

	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY);
}


//------ PLANIFICADOR SANTI FIN ------


char* traducirEstado(t_estado estado){

	char* string;

	switch(estado){

		case NEW:

				{
				string = strdup("New");
				break;
				}

		case READY:

				{
				string = strdup("Ready");
				break;
				}

		case EXEC:

				{
				string = strdup("Exec");
				break;
				}

		case BLOCKED:

				{
				string = strdup("Blocked");
				break;
				}

		case END:

				{
				string = strdup("End");
				break;
				}

		case SABOTAJE:

				{
				string = strdup("Sabotaje");
				break;
				}

	}

	return string;
}


void listarTripulante(){

	printf("Estado de la nave: %d \n", system("date"));

	iterarCola(colaNew);
	iterarCola(colaReady);
	iterarCola(colaExec);
	iterarCola(colaBlocked);

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

void pausarPlanificacion(){

	if(planificacion_play == 1){

		planificacion_play = 0;

	}

	if(planificacion_play == 0){

		log_info(logDiscordiador,"PLANIFICACION PAUSADA");

	}

	else{

		log_info(logDiscordiador,"La planificacion tiene cualquier valor negro");

	}
}


t_eliminado* deleteTripulante(uint32_t id, t_queue* cola){

	t_eliminado* eliminado = malloc(sizeof(t_eliminado));

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

		while(list_iterator_has_next(list_iterator)) {

			eliminado->tripulante = list_iterator_next(list_iterator);

			if(eliminado->tripulante->idTripulante == id){

				eliminado->tripulante->estado = BLOCKED;

				queue_push(colaBlocked,eliminado->tripulante);

				eliminado->cantidad++;
			}

		}

		list_iterator_destroy(list_iterator);

		return eliminado;

}


void eliminarTripulante(uint32_t id){

	pausarPlanificacion();

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

		planificacion_play = 1;

	}
	if(resultado == 1){

		int socketMiRAM = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

		t_paquete* paquete = armarPaqueteCon(vector[flag],EXPULSAR);

		enviarPaquete(paquete,socketMiRAM);

		log_info(logDiscordiador,"Se ha eliminado el tripulante con el Id: %d \n",id);

		planificacion_play = 1;

	}

	else{

		log_info(logDiscordiador,"Esta funcionando mal eliminarTripulante negro \n");
		exit(1);

	}

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
	patota->ID = idPatota;
	patota->tareas = tareasString;

	log_info(logDiscordiador,"Se creo la patota numero %d\n",idPatota);
	return patota;
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	t_patota* patota = asignarDatosAPatota(tareasString);
	t_paquete* paquete = armarPaqueteCon((void*) patota,PATOTA);
	enviarPaquete(paquete,server_socket);



	log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

	for (int i=0; i<cantidadTripulantes; i++){
		totalTripus ++;
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
//	el tripulante no necesita un semaforo

	lock(mutexColaNew);
	queue_push(colaNew,(void*)tripulante);
	unlock(mutexColaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d\n",tripulante->idTripulante);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripu, (void*) tripulante);
	pthread_detach(_hiloTripulante);
}


int esIO(char* tarea){

	for(int i=0; todasLasTareasIO[i] != NULL; i++){
		if(strcmp(todasLasTareasIO[i],tarea) == 0){
			return 1;
		}
	}
	return 0;
}


void pasarDeCola(t_tripulante* tripulante){


	switch(tripulante->estado){
		case READY:
			queue_push(colaReady, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a COLA READY \n", tripulante->idTripulante);
			break;

		case EXEC:

			queue_push(colaExec, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC \n", tripulante->idTripulante);
			break;

		case BLOCKED:
			queue_push(colaBlocked, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED \n", tripulante->idTripulante);
			break;

		case END:
			break;

		default:
				printf("\n No se reconoce el siguiente estado \n");
				exit(1);
	}

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
//	log_info(logDiscordiador, "tripulanteId: %d me conecte a MIRAM", tripulante->idTripulante);
	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);
	enviarPaquete(paqueteEnviado, miRAMsocket);
//	log_info(logDiscordiador, "tripulanteId: %d envie a MIRAM mi info principal", tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);
	log_info(logDiscordiador, "tripulanteId: %d recibi la tarea %s de MIRAM",
			tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	close(miRAMsocket);
}


void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){
	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete * paquete = armarPaqueteCon((void*) tripulante,SIGUIENTE_TAREA);
	enviarPaquete(paquete, miRAMsocket);

	recibirTareaDeMiRAM(miRAMsocket,tripulante);
	close(miRAMsocket);
}


// SE PUEDE USAR
void mandarTareaAejecutar(t_tripulante* tripulante, int socketMongo){

	t_paquete* paqueteConLaTarea = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);

	enviarPaquete(paqueteConLaTarea,socketMongo);
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


