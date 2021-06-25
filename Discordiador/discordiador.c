#include "discordiador.h"

char** todasLasTareasIO;

int main() {

	sem_init(&semPlanificacion,0,1);
	sem_init(&semaforoPlanificadorInicio,0,0);
	sem_init(&semaforoPlanificadorFin,0,0);
	sem_init(&semaforoPlanificadorFin,0,0);

	planificacionPlay = 1;
	todasLasTareasIO = malloc(sizeof(char*) * 7);

	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");
	todasLasTareasIO[6] = NULL;

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
	pthread_mutex_init(&mutexLogDiscordiador, NULL);
	pthread_mutex_init(&mutexTotalTripus, NULL);
	//
	int tripulantes = 3;
	t_coordenadas coordenadas[tripulantes ];
	for(int i=0; i<tripulantes;i++){
		coordenadas[i].posX = i +1;
		coordenadas[i].posY = i + 2;
	}
	//CONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2
	iniciarPatota(coordenadas, "GENERAR_OXIGENO 4;2;3;7\nGENERAR_COMIDA 2;2;3;7", tripulantes );


	int tripulantes2 = 2;
	t_coordenadas coordenadas2[tripulantes2 ];
	for(int i=0; i<tripulantes;i++){
		coordenadas2[i].posX = i +1;
		coordenadas2[i].posY = i + 2;
	}
	//CONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2
	iniciarPatota(coordenadas2, "GENERAR_OXIGENO 4;2;3;7\nGENERAR_COMIDA 2;2;3;7", tripulantes2 );


	pthread_t planificador;
	pthread_create(&planificador, NULL, (void*) hiloPlani, NULL);
	pthread_join(planificador, (void**) NULL);

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}

int leerTotalTripus(){
	lock(&mutexTotalTripus);
	int total = totalTripus;
	unlock(&mutexTotalTripus);
	return total;
}

void hiloPlani(){
	while(1){
		if(planificacionPlay && leerTotalTripus() > 0){

			for(int i=0; i <leerTotalTripus(); i++){
				sem_wait(&semaforoPlanificadorInicio);
			}

			lock(&mutexLogDiscordiador);
			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripus);
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");
			unlock(&mutexLogDiscordiador);

			actualizarCola(EXEC, colaExec, mutexColaExec);
			actualizarCola(BLOCKED, colaBlocked, mutexColaBlocked);
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

			for(int i=0; i<leerTotalTripus(); i++){
				sem_post(&semaforoPlanificadorFin);
			}
			lock(&mutexLogDiscordiador);
			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");
			unlock(&mutexLogDiscordiador);
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
				lock(&mutexLogDiscordiador);
				log_info(logDiscordiador,"tripulanteId %d: estoy en new", tripulante->idTripulante);
				unlock(&mutexLogDiscordiador);
				recibirPrimerTareaDeMiRAM(tripulante);
				sem_post(&semUltimoTripu);
				tripulante->estado = READY;
				break;
			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
				lock(&mutexLogDiscordiador);
				log_info(logDiscordiador,"tripulanteId %d: estoy en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec);
				unlock(&mutexLogDiscordiador);
				tripulante->estado = EXEC;
				break;
			case EXEC:
				lock(&mutexLogDiscordiador);
				log_info(logDiscordiador,"tripulanteId %d: estoy en exec", tripulante->idTripulante);
				unlock(&mutexLogDiscordiador);
				desplazarse(tripulante);
				ciclosExec --;
				quantumPendiente--;
				sleep(1);
				if(ciclosExec > 0 && quantumPendiente != 0){
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
					lock(&mutexLogDiscordiador);
					log_info(logDiscordiador,"tripulanteId %d: estoy en block ejecutando, me quedan %d cilos",
							tripulante->idTripulante, ciclosBlocked);
					unlock(&mutexLogDiscordiador);
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
				lock(&mutexTotalTripus);
				totalTripus--;
				unlock(&mutexTotalTripus);
				tripuVivo = 0;
				lock(&mutexLogDiscordiador);
				log_info(logDiscordiador,"tripulanteId %d: estoy en end, YA TERMINE. Ahora quedan %d tripus",
						tripulante->idTripulante, totalTripus);
				unlock(&mutexLogDiscordiador);
				// el unico caso donde no se hace un post al inicio del plani
				// esta asi porq como arriba se hace un totalTripu --, el wait
				// del planiInicio va a hacer una iteracion menos
				sem_post(&semaforoPlanificadorInicio);
				break;
		}
		if(tripulante->estado != END){
			actualizarEstadoEnRAM(tripulante);

			sem_post(&semaforoPlanificadorInicio);
			lock(&mutexLogDiscordiador);
			log_info(logDiscordiador,"tripulanteId %d: ESPERANDO QUE EL PLANI TERMINE",
					tripulante->idTripulante);
			unlock(&mutexLogDiscordiador);
			sem_wait(&semaforoPlanificadorFin);
			lock(&mutexLogDiscordiador);
			log_info(logDiscordiador,"tripulanteId %d: YA TERMINO EL PLANI, AHORA CONTINUO",
					tripulante->idTripulante);
			unlock(&mutexLogDiscordiador);
		}
	}
}


void actualizarEstadoEnRAM(t_tripulante* tripulante){

//	log_info(logDiscordiador,"Se manda a actualizar el tripulante de ID: %d",tripulante->idTripulante);
	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, ESTADO_TRIPULANTE);
//	log_info(logDiscordiador,"El tripulante de ID %d, esta enviando paquete con cod_op %d",
//			tripulante->idTripulante, paqueteAenviar->codigo_operacion);

	log_info(logDiscordiador,"TRIPULANTE %d -VOY A MANDAR ACTUALIZACION EN RAM", tripulante->idTripulante);

	esperarConfirmacionDeRAM(miRAMsocket);

	close(miRAMsocket);
}


void actualizarCola(t_estado estado, t_queue* cola, pthread_mutex_t colaMutex){

	t_tripulante* tripulante;
	int tamanioInicialCola = queue_size(cola);
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador,"------El tamanio inicial de la cola de %s es de %d-----", traducirEstado(estado), tamanioInicialCola);
	unlock(&mutexLogDiscordiador);
	for(int i=0; i<tamanioInicialCola; i++){
		lock(&colaMutex);
		tripulante = (t_tripulante*) queue_pop(cola);
		unlock(&colaMutex);
		//if(queue_size(colaExec) >= gradoMultiprocesamiento && tripulante->estado == EXEC){
			//tripulante->estado = estado;
		//}
		if(tripulante->estado != estado){
			pasarDeCola(tripulante);
		}
		else{
			lock(&colaMutex);
			queue_push(cola, tripulante);
			unlock(&colaMutex);
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


uint32_t calculoCiclosExec(t_tripulante* tripulante){

	uint32_t desplazamientoEnX = diferencia(tripulante->instruccionAejecutar->posX, tripulante->posX);
	uint32_t desplazamientoEnY = diferencia(tripulante->instruccionAejecutar->posY, tripulante->posY);

	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return  desplazamientoEnX + desplazamientoEnY + 1;
	}

	return desplazamientoEnX + desplazamientoEnY + tripulante->instruccionAejecutar->tiempo;
}


uint32_t diferencia(uint32_t numero1, uint32_t numero2){
	return abs(numero1-numero2);
}


void desplazarse(t_tripulante* tripulante){
	uint32_t diferenciaEnX = diferencia(tripulante->posX, tripulante->instruccionAejecutar->posX);
	uint32_t restaEnX = tripulante->posX - tripulante->instruccionAejecutar->posX;
	uint32_t restaEnY = tripulante->posY - tripulante->instruccionAejecutar->posY;
	uint32_t diferenciaEnY = diferencia(tripulante->posY, tripulante->instruccionAejecutar->posY);
	uint32_t desplazamiento = 0;
	//FALTAN MUTEX
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",
			tripulante->posX, tripulante->posY );
	unlock(&mutexLogDiscordiador);
	if(diferenciaEnX){
		lock(&mutexLogDiscordiador);
		desplazamiento = restaEnX / diferenciaEnX;
		log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
					tripulante->posX, tripulante->instruccionAejecutar->posX, restaEnX, diferenciaEnX, desplazamiento);
		unlock(&mutexLogDiscordiador);
		tripulante->posX = tripulante->posX - desplazamiento;
	}
	else if(diferenciaEnY){
		lock(&mutexLogDiscordiador);
		desplazamiento = restaEnY / diferenciaEnY;
		log_info(logDiscordiador,"El valor que se le asigna es (%d - %d)(%d) / %d = %d",
					tripulante->posY, tripulante->instruccionAejecutar->posY, restaEnY, diferenciaEnY, desplazamiento);
		unlock(&mutexLogDiscordiador);
		tripulante->posY = tripulante->posY - desplazamiento;
	}
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY);
	unlock(&mutexLogDiscordiador);
}


//------ PLANIFICADOR SANTI FIN ------


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


t_eliminado* deleteTripulante(uint32_t id, t_queue* cola){

	t_eliminado* eliminado = malloc(sizeof(t_eliminado));

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

		while(list_iterator_has_next(list_iterator)) {

			eliminado->tripulante = list_iterator_next(list_iterator);

			if(eliminado->tripulante->idTripulante == id){

				eliminado->tripulante->estado = BLOCKED;
				lock(&mutexColaBlocked);
				queue_push(colaBlocked,eliminado->tripulante);
				unlock(&mutexColaBlocked);
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
		lock(&mutexLogDiscordiador);
		log_info(logDiscordiador,"No se ha encontrado un tripulante con el Id: %d \n",id);
		unlock(&mutexLogDiscordiador);
		planificacionPlay = 1;

	}
	if(resultado == 1){

		int socketMiRAM = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

		t_paquete* paquete = armarPaqueteCon(vector[flag],EXPULSAR);

		enviarPaquete(paquete,socketMiRAM);
		lock(&mutexLogDiscordiador);
		log_info(logDiscordiador,"Se ha eliminado el tripulante con el Id: %d \n",id);
		unlock(&mutexLogDiscordiador);
		planificacionPlay = 1;

	}

	else{
		lock(&mutexLogDiscordiador);
		log_info(logDiscordiador,"Esta funcionando mal eliminarTripulante negro \n");
		unlock(&mutexLogDiscordiador);
		exit(1);

	}

}


void crearConfig(){

	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");

	if(config == NULL){
		lock(&mutexLogDiscordiador);
		log_error(logDiscordiador, "\n La ruta es incorrecta \n");
		unlock(&mutexLogDiscordiador);
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

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	esperarConfirmacionDeRAM(miRAMsocket);
	log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

	for (int i=0; i<cantidadTripulantes; i++){
		totalTripus ++;
		iniciarTripulante(coordenadas[i], patota->ID);
	}

	for(int i=0;i<cantidadTripulantes;i++){
		sem_wait(&semUltimoTripu);
	}

	mandarTripulanteNulo();

	close(miRAMsocket);
}


void mandarTripulanteNulo() {
	t_tripulante* tripulante = malloc(sizeof(t_tripulante));

	tripulante->posX = 0;
	tripulante->posY = 0;
	tripulante->idTripulante = -1;
	tripulante->idPatota = 0;
	tripulante->estado = NEW;

	int socket =iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	enviarPaquete(armarPaqueteCon((void*) tripulante, TRIPULANTE), socket);

	recibirTareaDeMiRAM(socket, tripulante);

	close(socket);
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

	lock(&mutexColaNew);
	queue_push(colaNew,(void*)tripulante);
	unlock(&mutexColaNew);

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
			lock(&mutexColaReady);
			queue_push(colaReady, tripulante);
			unlock(&mutexColaReady);
			log_info(logDiscordiador,"El tripulante %d paso a COLA READY \n", tripulante->idTripulante);
			break;

		case EXEC:
			lock(&mutexColaExec);
			queue_push(colaExec, tripulante);
			unlock(&mutexColaExec);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC \n", tripulante->idTripulante);
			break;

		case BLOCKED:
			lock(&mutexColaBlocked);
			queue_push(colaBlocked, tripulante);
			unlock(&mutexColaBlocked);
			log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED \n", tripulante->idTripulante);
			break;

		case END:
			break;

		default:
				printf("\n No se reconoce el siguiente estado \n");
				exit(1);
	}

}


int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion){
	int socket = iniciarConexionDesdeClienteHacia(puerto);
	enviarPaquete(armarPaqueteCon(informacion, codigoOperacion), socket);
	return socket;
	//----ESTA FUNCION NO LLEVA EL CLOSE, PERO HAY Q AGREGARLO SIEMPRE
}


void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

	if(paqueteRecibido->codigoOperacion == TAREA){

	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);

	    lock(&mutexLogDiscordiador);
	    log_info(logDiscordiador, "TRIPULANTE: %d - recibi la tarea %s de MIRAM",
	    			tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
	    unlock(&mutexLogDiscordiador);
	}
	else{
		lock(&mutexLogDiscordiador);
	    log_error(logDiscordiador,"El paquete recibido no es una tarea\n");
	    unlock(&mutexLogDiscordiador);
	    exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}


void recibirPrimerTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, TRIPULANTE);
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador, "tripulanteId: %d envie a MIRAM mi info principal",
			tripulante->idTripulante);
	unlock(&mutexLogDiscordiador);
	recibirTareaDeMiRAM(miRAMsocket, tripulante);
	close(miRAMsocket);
}


void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, SIGUIENTE_TAREA);
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador, "TRIPULANTE: %d - VOY A BUSCAR PROX TAREA A MI RAM",
		    			tripulante->idTripulante);
	unlock(&mutexLogDiscordiador);
	recibirTareaDeMiRAM(miRAMsocket,tripulante);
	close(miRAMsocket);
}












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
	lock(&mutexLogDiscordiador);
	log_info(logDiscordiador,"Confirmacion %s\n",mensajeRecibido);
	unlock(&mutexLogDiscordiador);

	if(strcmp(mensajeRecibido,"TAREA REALIZADA") == 0){
		lock(&mutexLogDiscordiador);
		log_info(logDiscordiador,"Se elimino la tarea %s\n",tarea->nombreTarea);
		unlock(&mutexLogDiscordiador);
	}
	else{
		lock(&mutexLogDiscordiador);
		log_info(logDiscordiador, "No sabemos q hacer todavia =(\n");
		unlock(&mutexLogDiscordiador);
	}
	free(tarea->nombreTarea);
	free(tarea);
	free(mensajeRecibido);

	eliminarPaquete(paqueteRecibido);

}


