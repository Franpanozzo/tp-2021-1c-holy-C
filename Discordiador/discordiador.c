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
	pthread_mutex_init(&mutexPlanificadorFin, NULL);
	//
	int tripulantes = 3;
	t_coordenadas coordenadas[tripulantes ];
	for(int i=0; i<tripulantes;i++){
		coordenadas[i].posX = i;
		coordenadas[i].posY = i + 1;
	}
	iniciarPatota(coordenadas, "GENERAR_OXIGENO 4;5;6;7\nCONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2", tripulantes );
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

			for(int i=0; i<totalTripus; i++){
				sem_wait(&semaforoPlanificadorInicio);
				log_info(logDiscordiador,"Planificacion semaforo inicio");
			}


			actualizar(EXEC, colaExec);
			actualizar(BLOCKED, colaBlocked);
			actualizar(NEW, colaNew);
			actualizar(READY, colaReady);

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
				log_info(logDiscordiador,"Planificacion semaforo fin");
			}
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
				sem_post(&semaforoPlanificadorInicio);
				break;
			case READY:
				log_info(logDiscordiador,"tripulanteId %d: estoy en ready", tripulante->idTripulante);
				if(ciclosExec == 0)
				ciclosExec = calcularCiclosExec(tripulante);
				quantumPendiente = quantum;
				tripulante->estado = EXEC;
				sem_post(&semaforoPlanificadorInicio);
				break;
			case EXEC:
				log_info(logDiscordiador,"tripulanteId %d: estoy en exec", tripulante->idTripulante);
				sleep(1);
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
				sem_post(&semaforoPlanificadorInicio);
				break;
			case BLOCKED:
				log_info(logDiscordiador,"tripulanteId %d: estoy en block", tripulante->idTripulante);
				if(tripulante->idTripulante == idTripulanteBlocked){
					sleep(1);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						idTripulanteBlocked = -1;
						tripulante->estado = READY;
					}
				}
				sem_post(&semaforoPlanificadorInicio);
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
				// el unico caso donde no se hace un post al inicio del plani
				// esta asi porq como arriba se hace un totalTripu --, el wait
				// del planiInicio va a hacer una iteracion menos
				break;
		}
		actualizarEstadoEnRAM(tripulante);
		log_info(logDiscordiador,"tripulanteId %d: esperando semaforo", tripulante->idTripulante);
		sem_wait(&semaforoPlanificadorFin);
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

	log_info(logDiscordiador,"Se manda a actualizar el tripulante de ID: %d",tripulante->idTripulante);
	int socketRam = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete* paqueteAenviar = armarPaqueteCon(tripulante,ESTADO_TRIPULANTE);
	enviarPaquete(paqueteAenviar,socketRam);
	close(socketRam);

}

void actualizar(t_estado estado, t_queue* cola){
	t_queue* aux = queue_create();
	t_tripulante* tripulante;
	while(cola != NULL && queue_size(colaExec) <= gradoMultiprocesamiento){
		tripulante = (t_tripulante*) queue_pop(cola);
		if(tripulante->estado != estado){
			pasarDeEstado(tripulante);
		}
		else{
			queue_push(aux, &tripulante);
		}
	}
	cola = aux;
	aux = NULL;
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
		iniciarTripulante(coordenadas[i], patota->ID);
		totalTripus ++;
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


void pasarDeEstado(t_tripulante* tripulante){


	switch(tripulante->estado){
		case READY:
			queue_push(colaReady, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			break;

		case EXEC:

			queue_push(colaExec, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			break;

		case BLOCKED:
			queue_push(colaBlocked, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
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
	log_info(logDiscordiador, "tripulanteId: %d me conecte a MIRAM", tripulante->idTripulante);
	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);
	enviarPaquete(paqueteEnviado, miRAMsocket);
	log_info(logDiscordiador, "tripulanteId: %d envie a MIRAM mi info principal", tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);
	log_info(logDiscordiador, "tripulanteId: %d recibi tarea de MIRAM", tripulante->idTripulante);
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


