#include "utils.h"

// ---------INICIALIZACION--------

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

void iniciarListas(){

	listaNew = malloc(sizeof(t_lista));
	listaReady = malloc(sizeof(t_lista));
	listaExec = malloc(sizeof(t_lista));
	listaBlocked = malloc(sizeof(t_lista));
	listaSabotaje = malloc(sizeof(t_lista));
	listaAeliminar = malloc(sizeof(t_lista));


	listaNew->elementos = list_create();
	listaReady->elementos = list_create();
	listaExec->elementos = list_create();
	listaBlocked->elementos = list_create();
	listaSabotaje->elementos = list_create();
	listaAeliminar->elementos = list_create();


	pthread_mutex_init(&listaNew->mutex, NULL);
	pthread_mutex_init(&(listaReady->mutex), NULL);
	pthread_mutex_init(&listaExec->mutex, NULL);
	pthread_mutex_init(&listaBlocked->mutex, NULL);
	pthread_mutex_init(&listaSabotaje->mutex, NULL);
	pthread_mutex_init(&listaAeliminar->mutex, NULL);

}


void iniciarMutex(){
	pthread_mutex_init(&mutexTotalTripus, NULL);
	pthread_mutex_init(&mutexIdTripulanteBlocked, NULL);
	pthread_mutex_init(&mutexPlanificador, NULL);
}

void cargarConfiguracion(){

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

	sabotaje->tiempo = config_get_int_value(config,"DURACION_SABOTAJE");
	retardoCiclosCPU = config_get_int_value(config,"RETARDO_CICLO_CPU");
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
	char *fecha = temporal_get_string_time("%d-%m-%y %H:%M:%S");
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/Discordiador/logs/");
	string_append(&pathLog, "log ");
	string_append(&pathLog, fecha);
	string_append(&pathLog, ".log");
	free(fecha);
	return pathLog;
}

// ---------OPERACIONES VARIABLES GLOBALES--------

int leerPlanificacion(){
	lock(&mutexPlanificador);
	int _planificacion = planificacion;
	unlock(&mutexPlanificador);
	return _planificacion;
}


void modificarPlanificacion(int estadoPlanificacion){
	lock(&mutexPlanificador);
	planificacion = estadoPlanificacion;
	unlock(&mutexPlanificador);
}


int totalTripulantes(){
	int total = 0;

	lock(&listaNew->mutex);
	total += list_size(listaNew->elementos);
	unlock(&listaNew->mutex);

	lock(&listaReady->mutex);
	total += list_size(listaReady->elementos);
	unlock(&listaReady->mutex);

	lock(&listaExec->mutex);
	total += list_size(listaExec->elementos);
	unlock(&listaExec->mutex);

	lock(&listaBlocked->mutex);
	total += list_size(listaBlocked->elementos);
	unlock(&listaBlocked->mutex);

	lock(&listaSabotaje->mutex);
	total += list_size(listaSabotaje->elementos);
	unlock(&listaSabotaje->mutex);

	return total;
}


void modificarTripulanteBlocked(int numero){
	lock(&mutexIdTripulanteBlocked);
	idTripulanteBlocked = numero;
	unlock(&mutexIdTripulanteBlocked);
}


int leerTripulanteBlocked(){
	lock(&mutexIdTripulanteBlocked);
	int id = idTripulanteBlocked;
	unlock(&mutexIdTripulanteBlocked);
	return id;
}


void meterEnLista(t_tripulante* dato, t_lista* lista){
	lock(&lista->mutex);
	list_add(lista->elementos, dato);
	unlock(&lista->mutex);
}

void* sacarDeLista(t_lista* lista){
	lock(&lista->mutex);
	void* dato = list_remove(lista->elementos, 0);
	unlock(&lista->mutex);
	return dato;
}


// -------AUXILIARES------


void cambiarDeEstado(t_tripulante* tripulante, t_estado estado){
	tripulante->estado = estado;
}

void esperarTerminarTripulante(t_tripulante* unTripulante){
	sem_wait(&unTripulante->semaforoFin);
}


void avisarTerminoPlanificacion(t_tripulante* tripulante){
	sem_post(&tripulante->semaforoInicio);
}


void avisarHaySabotaje(t_tripulante* tripulante){
	sem_wait(&tripulante->semaforoInicio);
}


void avisarTerminoSabotaje(t_tripulante* tripulante){
	sem_post(&tripulante->semaforoInicio);
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


uint32_t diferencia(uint32_t numero1, uint32_t numero2){
	return (uint32_t) abs(numero1-numero2);
}


char* traducirEstado(t_estado estado){
	switch(estado){
		case NEW:
			return "New";
		case READY:
			return "Ready";
		case EXEC:
			return "Exec";
		case BLOCKED:
			return "Blocked";
		case SABOTAJE:
			return "Sabotaje";
	}
	return "ERROR";
}


void comunicarseConTripulantes(t_lista* lista, void(*closure)(void*)){
	lock(&lista->mutex);
	if(!list_is_empty(lista->elementos))
		list_iterate(lista->elementos, closure);
	unlock(&lista->mutex);
}

// ----- PRINCIPALES -----


void elegirTripulanteAbloquear(){
	lock(&listaBlocked->mutex);
	t_tripulante* tripulanteBlocked = (t_tripulante*) list_get(listaBlocked->elementos, 0);
	unlock(&listaBlocked->mutex);
	idTripulanteBlocked = tripulanteBlocked->idTripulante;
	log_info(logDiscordiador,"------EL TRIPU BLOQUEADO ES %d-----", idTripulanteBlocked);
}


void pasarAlistaSabotaje(t_lista* lista){

	bool tripulanteDeMenorId(t_tripulante* tripulante1, t_tripulante* tripulante2){
		return tripulante1->idTripulante < tripulante2->idTripulante;
	}

	lock(&lista->mutex);
	list_sort(lista->elementos, (void*)tripulanteDeMenorId);
	list_iterate(lista->elementos, (void*)ponerEnSabotaje);
	list_iterate(lista->elementos, (void*)pasarDeLista);
	list_clean(lista->elementos);
	unlock(&lista->mutex);
}


void pasarDeLista(t_tripulante* tripulante){
	actualizarEstadoEnRAM(tripulante);

	switch(tripulante->estado){
		case READY:
			meterEnLista(tripulante, listaReady);
			log_info(logDiscordiador,"El tripulante %d paso a COLA READY", tripulante->idTripulante);
			sem_post(&tripulante->semaforoInicio);
			break;

		case EXEC:
			meterEnLista(tripulante, listaExec);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC", tripulante->idTripulante);
			break;

		case BLOCKED:
			meterEnLista(tripulante, listaBlocked);
			log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED", tripulante->idTripulante);
			break;

		case SABOTAJE:
			meterEnLista(tripulante, listaSabotaje);
			log_info(logDiscordiador,"El tripulante %d paso a COLA SABOTAJE", tripulante->idTripulante);
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


void actualizarEstadoEnRAM(t_tripulante* tripulante){
	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, ESTADO_TRIPULANTE);
	if(!confirmacion(miRAMsocket))
		log_error(logDiscordiador,"No se pudo actalizar en miRam el estado "
				"del tripulante %d", tripulante->idTripulante);
	close(miRAMsocket);
}


int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion){
	int socket = iniciarConexionDesdeClienteHacia(puerto);
	enviarPaquete(armarPaqueteCon(informacion, codigoOperacion), socket);
	return socket;
	//----ESTA FUNCION NO LLEVA EL CLOSE, PERO HAY Q AGREGARLO SIEMPRE
}


bool confirmacion(int server_socket){

	t_paquete* paqueteRecibido = recibirPaquete(server_socket);
	bool confirmacion = 0;

	if(paqueteRecibido->codigoOperacion == STRING){

		char* mensajeConfirmacion = deserializarString(paqueteRecibido);
		confirmacion = strcmp(mensajeConfirmacion,"OK") == 0;
		free(mensajeConfirmacion);
	}

	eliminarPaquete(paqueteRecibido);
	return confirmacion;
}


int esIO(char* tarea){

	for(int i=0; todasLasTareasIO[i] != NULL; i++){
		if(strcmp(todasLasTareasIO[i],tarea) == 0){
			return 1;
		}
	}
	return 0;
}


uint32_t distancia(t_coordenadas unaCoordenada, t_coordenadas otraCoordenada){
	return diferencia(unaCoordenada.posX, otraCoordenada.posX) +
			diferencia(unaCoordenada.posY, otraCoordenada.posY);
}


uint32_t calculoCiclosExec(t_tripulante* tripulante){
	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return 1;
	}
	return tripulante->instruccionAejecutar->tiempo;
}


void desplazarse(t_tripulante* tripulante, t_coordenadas destino){

//	t_desplazamiento desplazamiento;
//	desplazamiento.id = tripulante->idTripulante;
//	desplazamiento.inicio = tripulante->coordenadas;

	int diferenciaEnX = diferencia(tripulante->coordenadas.posX, destino.posX);
	int diferenciaEnY = diferencia(tripulante->coordenadas.posY, destino.posY);
	int restaEnX = tripulante->coordenadas.posX - destino.posX;
	int restaEnY = tripulante->coordenadas.posY - destino.posY;

//	log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",
//			tripulante->coordenadas.posX, tripulante->coordenadas.posY );

	if(diferenciaEnX){
		tripulante->coordenadas.posX -= restaEnX / diferenciaEnX;;
	}
	else if(diferenciaEnY){
		tripulante->coordenadas.posY -= restaEnY / diferenciaEnY;
	}

//	desplazamiento.fin = tripulante->coordenadas;

//	enviarA(puertoEIPRAM, tripulante, MOVIMIENTO); // FALTA EL CLOSE
//	enviarA(puertoEIPMongo, tripulante, DESPLAZAMIENTO);
//	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",
//			tripulante->coordenadas.posX, tripulante->coordenadas.posY);

}


void listarLista(t_lista* lista){

	void imprimirTripulante(t_tripulante* tripulante){
		log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Estado: %s",
				tripulante->idTripulante, tripulante->idPatota, traducirEstado(tripulante->estado));
	}

	bool estaVivo(t_tripulante* tripulante){
		bool hayQueSacarlo(t_tripulante* otroTripulante){
				return tripulante == otroTripulante;
			}
		return !list_any_satisfy(listaAeliminar->elementos, (void*) hayQueSacarlo);
	}

	lock(&lista->mutex);
	t_list* listaAux = list_filter(lista->elementos, (void*) estaVivo);
	unlock(&lista->mutex);
	list_iterate(listaAux, (void*)imprimirTripulante);
	list_destroy(listaAux);
}


void listarTripulantes(){

 	void imprimirTripulante(t_tripulante* tripulante){
		bool estaVivo(t_tripulante* tripulante){
			bool hayQueSacarlo(t_tripulante* otroTripulante){
					return tripulante == otroTripulante;
				}
			return !list_any_satisfy(listaAeliminar->elementos, (void*) hayQueSacarlo);
		}

 		if(estaVivo(tripulante)){
			log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Estado: %s",
					tripulante->idTripulante, tripulante->idPatota, traducirEstado(tripulante->estado));
 		}
	}



	log_info(logDiscordiador,"Estado de la nave: %s", temporal_get_string_time("%d-%m-%y %H:%M:%S"));

	t_list* listaAux = list_create();

	lock(&listaNew->mutex);
	list_add_all(listaAux, listaNew->elementos);
	unlock(&listaNew->mutex);

	lock(&listaReady->mutex);
	list_add_all(listaAux, listaReady->elementos);
	unlock(&listaReady->mutex);

	lock(&listaExec->mutex);
	list_add_all(listaAux, listaExec->elementos);
	unlock(&listaExec->mutex);

	lock(&listaBlocked->mutex);
	list_add_all(listaAux, listaBlocked->elementos);
	unlock(&listaBlocked->mutex);

	lock(&listaSabotaje->mutex);
	list_add_all(listaAux, listaSabotaje->elementos);
	unlock(&listaSabotaje->mutex);

	list_iterate(listaAux, (void*)imprimirTripulante);
	list_destroy(listaAux);
}



void eliminarTripulante(int id){
	bool esElBuscado(t_tripulante* tripulante){
		return tripulante->idTripulante == id;
	}

	t_tripulante* tripulanteAeliminar = NULL;
	t_lista* arrayListas[4] = {listaReady, listaExec, listaBlocked, listaNew};

	for(int i=0; tripulanteAeliminar == NULL && i<4; i++){
		tripulanteAeliminar = (t_tripulante*)list_find(arrayListas[i]->elementos, (void*)esElBuscado);
	}

	log_info(logDiscordiador, "Se aniadio al tripulante %d", tripulanteAeliminar->idTripulante);
	lock(&listaAeliminar->mutex);
	list_add(listaAeliminar->elementos, tripulanteAeliminar);
	unlock(&listaAeliminar->mutex);
}


/*
 * bool esElBuscado(t_tripulante* tripulante){
		return tripulante->idTripulante == id;
	}

	t_list* listaAux = list_create();
	list_add_all(listaAux, listaReady->elementos);
	list_add_all(listaAux, listaExec->elementos);
	list_add_all(listaAux, listaBlocked->elementos);
	list_add_all(listaAux, listaNew->elementos);
	list_add_all(listaAux, listaSabotaje->elementos);

	t_tripulante* tripulanteAeliminar = (t_tripulante*)list_find(listaAux, (void*)esElBuscado);
	log_info(logDiscordiador, "Se aniadio al tripulante %d", tripulanteAeliminar->idTripulante);
	lock(&listaAeliminar->mutex);
	list_add(listaAeliminar->elementos, tripulanteAeliminar);
	unlock(&listaAeliminar->mutex);

	list_clean(listaAux);
	list_destroy(listaAux);
 */

void sacarDeColas(t_tripulante* tripulante){
	bool hayQueSacarlo(t_tripulante* otroTripulante){
		return tripulante == otroTripulante;
	}

	list_remove_by_condition(listaReady->elementos, (void*)hayQueSacarlo);
	list_remove_by_condition(listaExec->elementos, (void*)hayQueSacarlo);
	list_remove_by_condition(listaBlocked->elementos, (void*)hayQueSacarlo);
	list_remove_by_condition(listaNew->elementos, (void*)hayQueSacarlo);
	list_remove_by_condition(listaSabotaje->elementos, (void*)hayQueSacarlo);

	tripulante->estaVivo = 0;
	avisarTerminoPlanificacion(tripulante);
}

// ----- ME QUEDE ACA EN LA ORGANIZACION DE LAS FUNCIONES

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


bool esElTripulanteSabotaje(void* unTripulante){
	t_tripulante* tripulante = (t_tripulante*) unTripulante;
	return tripulante->idTripulante == sabotaje->tripulanteSabotaje->idTripulante;
}


void elegirTripulanteSabotaje(){

	t_tripulante* masCercaAlSabotaje(void* primerTripulante, void* segundoTripulante){
		t_tripulante* tripulante1 = (t_tripulante*) primerTripulante;
		t_tripulante* tripulante2 = (t_tripulante*) segundoTripulante;

		if(distancia(tripulante1->coordenadas, sabotaje->coordenadas) <
				distancia(tripulante2->coordenadas, sabotaje->coordenadas))
			return tripulante1;
		else
			return tripulante2;
	}

	lock(&listaSabotaje->mutex);

	sabotaje->tripulanteSabotaje = (t_tripulante*)
			list_get_minimum(listaSabotaje->elementos, (void*)masCercaAlSabotaje);

	log_info(logDiscordiador,"el tripulante que realiza el sabotaje es el %d",
			sabotaje->tripulanteSabotaje->idTripulante);

	list_remove_by_condition(listaSabotaje->elementos, (void*)esElTripulanteSabotaje);

	unlock(&listaSabotaje->mutex);

	int socketMongo = enviarA(puertoEIPMongo, &sabotaje->tripulanteSabotaje->idTripulante, ID_SABOTAJE);
	close(socketMongo);

	avisarTerminoPlanificacion(sabotaje->tripulanteSabotaje);
}


void ponerEnReady(t_tripulante* unTripulante){
	cambiarDeEstado(unTripulante, READY);
}


void ponerEnSabotaje(t_tripulante* unTripulante){
	cambiarDeEstado(unTripulante, SABOTAJE);
}


// FUNCION 2
void recibirPrimerTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, TRIPULANTE);

	log_info(logDiscordiador, "tripulanteId: %d envie a MIRAM mi info principal",
			tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);


	close(miRAMsocket);
}

// FUNCION 3
void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = enviarA(puertoEIPRAM, tripulante, SIGUIENTE_TAREA);

	log_info(logDiscordiador, "TRIPULANTE: %d - VOY A BUSCAR PROX TAREA A MI RAM",
		    			tripulante->idTripulante);

	recibirTareaDeMiRAM(miRAMsocket,tripulante);

	close(miRAMsocket);
}

//FUNCION 4
void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

	if(paqueteRecibido->codigoOperacion == TAREA){
		free(tripulante->instruccionAejecutar->nombreTarea);
		free(tripulante->instruccionAejecutar);
	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);

	    log_info(logDiscordiador, "El tripulante %d recibio la tarea %s",
	    			tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);

		if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_NULA") == 0){
			eliminarTripulante(tripulante->idTripulante);
			log_info(logDiscordiador,"El tripulante %d ya no le quedan tareas por hacer", tripulante->idTripulante);
		}

		if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_ERROR") == 0){
			eliminarTripulante(tripulante->idTripulante);
			log_info(logDiscordiador,"El tripulante %d no ha podido ser alocado en memoria "
					"porque no hay espacio", tripulante->idTripulante);
		}
	}
	else{

	    log_error(logDiscordiador,"El paquete recibido no es una tarea");
	    exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}
