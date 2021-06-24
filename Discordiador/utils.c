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

void iniciarListas(){

	listaNew = malloc(sizeof(t_lista));
	listaReady = malloc(sizeof(t_lista));
	listaExec = malloc(sizeof(t_lista));
	listaBlocked = malloc(sizeof(t_lista));
	listaSabotaje = malloc(sizeof(t_lista));

	listaNew->elementos = list_create();
	listaReady->elementos = list_create();
	listaExec->elementos = list_create();
	listaBlocked->elementos = list_create();
	listaSabotaje->elementos = list_create();

	pthread_mutex_init(&listaNew->mutex, NULL);
	pthread_mutex_init(&listaReady->mutex, NULL);
	pthread_mutex_init(&listaExec->mutex, NULL);
	pthread_mutex_init(&listaBlocked->mutex, NULL);
	pthread_mutex_init(&listaSabotaje->mutex, NULL);
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

int leerPlanificacion(){
	lock(mutexPlanificador);
	int _planificacion = planificacion;
	unlock(mutexPlanificador);
	return _planificacion;
}


void modificarPlanificacion(int estadoPlanificacion){
	lock(mutexPlanificador);
	planificacion = estadoPlanificacion;
	unlock(mutexPlanificador);
}


int totalTripulantes(){
	lock(listaNew->mutex);
	lock(listaReady->mutex);
	lock(listaExec->mutex);
	lock(listaBlocked->mutex);
	lock(listaSabotaje->mutex);

	int total = list_size(listaNew->elementos) + list_size(listaReady->elementos) +
			list_size(listaExec->elementos) + list_size(listaBlocked->elementos) + list_size(listaSabotaje->elementos);

	unlock(listaNew->mutex);
	unlock(listaReady->mutex);
	unlock(listaExec->mutex);
	unlock(listaBlocked->mutex);
	unlock(listaSabotaje->mutex);

	return total;
}


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


t_tripulante* masCercaAlSabotaje(void* primerTripulante, void* segundoTripulante){
	t_tripulante* tripulante1 = (t_tripulante*) primerTripulante;
	t_tripulante* tripulante2 = (t_tripulante*) segundoTripulante;

	if(distancia(tripulante1->coordenadas, sabotaje->coordenadas) <
			distancia(tripulante2->coordenadas, sabotaje->coordenadas))
		return tripulante1;
	else
		return tripulante2;
}


void esperarTerminarTripulante(t_tripulante* unTripulante){
//	t_tripulante* tripulante = (t_tripulante*) unTripulante;
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


void casoBlocked(){
	if(idTripulanteBlocked == NO_HAY_TRIPULANTE_BLOQUEADO && list_size(listaBlocked->elementos) > 0){
		lock(listaBlocked->mutex);
		t_tripulante* tripulanteBlocked = (t_tripulante*) list_get(listaBlocked->elementos, 0);
		unlock(listaBlocked->mutex);
		idTripulanteBlocked = tripulanteBlocked->idTripulante;
		log_info(logDiscordiador,"------EL TRIPU BLOQUEADO ES %d-----", idTripulanteBlocked);
	}
}




void iterarCola(t_lista* lista, t_estado estado){

	t_list_iterator* list_iterator = list_iterator_create(lista->elementos);

	while(list_iterator_has_next(list_iterator)) {
		t_tripulante* tripulante = list_iterator_next(list_iterator);
		log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Status:%s    ",
				tripulante->idTripulante, tripulante->idPatota, traducirEstado(tripulante->estado));
	}

	list_iterator_destroy(list_iterator);
}


void meterEnLista(void* dato, t_lista* lista){
	lock(lista->mutex);
	list_add(lista->elementos, dato);
	unlock(lista->mutex);
}

void* sacarDeLista(t_lista* lista){
	lock(lista->mutex);
	void* dato = list_remove(lista->elementos, 0);
	unlock(lista->mutex);
	return dato;
}

void pasarAlistaSabotaje(t_lista* lista){
	lock(lista->mutex);
	list_sort(lista->elementos, tripulanteDeMenorId);
	list_add_all(listaSabotaje->elementos, lista->elementos);
	list_clean(lista->elementos);
	unlock(lista->mutex);
}

void pasarDeLista(t_tripulante* tripulante){
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

		default:
			log_error(logDiscordiador,"No se reconoce el estado", tripulante->idTripulante);
			exit(1);
	}
}


bool tripulanteDeMenorId(void* tripulante1, void* tripulante2){
	t_tripulante* tripulanteMenorId = (t_tripulante*) tripulante1;
	t_tripulante* tripulanteMayorId = (t_tripulante*) tripulante2;

	return tripulanteMenorId->idTripulante < tripulanteMayorId->idTripulante;
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
		return  distancia(tripulante->instruccionAejecutar->coordenadas, tripulante->coordenadas) + 1;
	}
	return distancia(tripulante->instruccionAejecutar->coordenadas, tripulante->coordenadas) +
			tripulante->instruccionAejecutar->tiempo;
}


void desplazarse(t_tripulante* tripulante, t_coordenadas destino){

	int diferenciaEnX = diferencia(tripulante->coordenadas.posX, destino.posX);
	int diferenciaEnY = diferencia(tripulante->coordenadas.posY, destino.posY);
	int restaEnX = tripulante->coordenadas.posX - destino.posX;
	int restaEnY = tripulante->coordenadas.posY - destino.posY;
	int desplazamiento = 0;

//	log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",
//			tripulante->posX, tripulante->posY );

	if(diferenciaEnX){
		desplazamiento = restaEnX / diferenciaEnX;
		tripulante->coordenadas.posX = tripulante->coordenadas.posX - desplazamiento;
	}
	else if(diferenciaEnY){
		desplazamiento = restaEnY / diferenciaEnY;
		tripulante->coordenadas.posY = (tripulante->coordenadas.posY - desplazamiento);
	}

//	log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY);

}


uint32_t diferencia(uint32_t numero1, uint32_t numero2){
	return (uint32_t) abs(numero1-numero2);
}


void listarLista(t_lista* lista){

	void imprimirTripulante(void* unTripulante){
		t_tripulante* tripulante = (t_tripulante*) unTripulante;
		log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Estado: %s",
				tripulante->idTripulante, tripulante->idPatota, traducirEstado(tripulante->estado));
	}

	lock(lista->mutex);
	list_iterate(lista->elementos, (void*)imprimirTripulante);
	unlock(lista->mutex);
}


void listarTripulantes(){
	log_info(logDiscordiador,"Estado de la nave: %s", temporal_get_string_time("%d-%m-%y %H:%M:%S"));

	listarLista(listaNew);
	listarLista(listaReady);
	listarLista(listaExec);
	listarLista(listaBlocked);
	/*
	 * list_iterate(listaNew->elementos, (void*)imprimirTripulante);
	list_iterate(listaReady->elementos, (void*)imprimirTripulante);
	list_iterate(listaExec->elementos, (void*)imprimirTripulante);
	list_iterate(listaBlocked->elementos, (void*)imprimirTripulante);
	 */
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

bool esElBuscado(void* unTripulante){
	t_tripulante* tripulante = (t_tripulante*) unTripulante;
	return tripulante->idTripulante == idBuscado;
}


void eliminarTripulante(){

	t_tripulante* tripulanteAeliminar = NULL;
	t_lista* arrayListas[4] = {listaReady, listaExec, listaBlocked, listaNew};

	for(int i=0; tripulanteAeliminar == NULL && i<4; i++){
		lock(arrayListas[i]->mutex);
		tripulanteAeliminar = list_remove_by_condition(arrayListas[i]->elementos, esElBuscado);
		unlock(arrayListas[i]->mutex);
	}
//	(t_tripulante*)
	if(tripulanteAeliminar != NULL){
		int socket = enviarA(puertoEIPRAM, tripulanteAeliminar, EXPULSAR);
		close(socket);
		tripulanteAeliminar->estaVivo = 0;
	}
	else{
		log_info(logDiscordiador,"No se ha encontrado un tripulante con el id %d",idBuscado);
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


bool esElTripulanteSabotaje(void* unTripulante){
	t_tripulante* tripulante = (t_tripulante*) unTripulante;
	return tripulante->idTripulante == sabotaje->tripulanteSabotaje->idTripulante;
}


void elegirTripulanteSabotaje(){
	lock(listaSabotaje->mutex);

	sabotaje->tripulanteSabotaje = (t_tripulante*)
			list_get_minimum(listaSabotaje->elementos, (void*)masCercaAlSabotaje);

	log_info(logDiscordiador,"el tripulante que realiza el sabotaje es el %d",
			sabotaje->tripulanteSabotaje->idTripulante);

	list_remove_by_condition(listaSabotaje->elementos, esElTripulanteSabotaje);

	unlock(listaSabotaje->mutex);

	avisarTerminoPlanificacion(sabotaje->tripulanteSabotaje);
}


void ponerEnReady(void* unTripulante){
	cambiarDeEstado(unTripulante, READY);
}


void ponerEnSabotaje(void* unTripulante){
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
			idBuscado = tripulante->idTripulante;
			eliminarTripulante();
			idBuscado = -1;
			log_info(logDiscordiador,"El tripulante %d ya no le quedan tareas por hacer", tripulante->idTripulante);
		}

		if(strcmp(tripulante->instruccionAejecutar->nombreTarea,"TAREA_ERROR") == 0){
			idBuscado = tripulante->idTripulante;
			eliminarTripulante();
			idBuscado = -1;
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


void cambiarDeEstado(t_tripulante* tripulante, t_estado estado){
	tripulante->estado = estado;
}
