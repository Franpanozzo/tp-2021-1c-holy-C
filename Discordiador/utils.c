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
}

void iniciarSemaforos(){
	sem_init(&semPlanificacion,0,0);
}

void iniciarMutex(){
	pthread_mutex_init(&mutexColaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
	pthread_mutex_init(&mutexColaExec, NULL);
	pthread_mutex_init(&mutexColaBlocked, NULL);
	pthread_mutex_init(&mutexPlanificadorFin, NULL);
	pthread_mutex_init(&mutexLogDiscordiador, NULL);
	pthread_mutex_init(&mutexTotalTripus, NULL);
	pthread_mutex_init(&mutexIdTripulanteBlocked, NULL);
	pthread_mutex_init(&mutexPlanificador, NULL);



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


int leerTotalTripus(){
	lock(mutexTotalTripus);
	int total = totalTripus;
	unlock(mutexTotalTripus);
	return total;
}
void modificarTripulanteBlocked(uint32_t numero){
	lock(mutexIdTripulanteBlocked);
	idTripulanteBlocked = numero;
	unlock(mutexIdTripulanteBlocked);
}

uint32_t leerTripulanteBlocked(){
	lock(mutexIdTripulanteBlocked);
	uint32_t id = idTripulanteBlocked;
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


t_tripulante* elTripuMasCerca(t_coordenadas lugarSabotaje){

	int diferenciaX  = 0;
	int diferenciaY = 0;
	int diferenciaMasCerca = 0;
	int diferenciaComparado = 0;

	t_list * listaDeComparacion = list_create();
	list_add_all(listaDeComparacion, colaReady->elements);
	list_add_all(listaDeComparacion, colaExec->elements);

	t_list_iterator * iterator = list_iterator_create(listaDeComparacion);
	t_tripulante * tripulante;
	t_tripulante * tripulanteMasCerca;

	tripulanteMasCerca = (t_tripulante *) list_iterator_next(iterator);
	//log_info(logDiscordiador, "\n %p \n",tripulanteMasCerca);
	diferenciaX = diferencia(tripulanteMasCerca->coordenadas.posX, lugarSabotaje.posX);
	diferenciaY = diferencia(tripulanteMasCerca->coordenadas.posY, lugarSabotaje.posY);

	diferenciaMasCerca = diferenciaX + diferenciaY;
	log_info(logDiscordiador,"diferencia : %d de tripulanteid: %d", diferenciaMasCerca, tripulanteMasCerca->idTripulante);
	while(list_iterator_has_next(iterator)){

		tripulante = (t_tripulante *) list_iterator_next(iterator);
		diferenciaX = diferencia(tripulante->coordenadas.posX, lugarSabotaje.posX);
		diferenciaY = diferencia(tripulante->coordenadas.posY, lugarSabotaje.posY);

		diferenciaComparado = diferenciaX + diferenciaY;
		log_info(logDiscordiador, "diferencia: %d de tripulanteid: %d", diferenciaComparado, tripulante->idTripulante);
		//s: x=0,y=0
		//t4: x=1,y=1
		//t2: x=1,y=0-1
		//t1: x=6,y=6

		if(diferenciaComparado < diferenciaMasCerca || (diferenciaComparado == diferenciaMasCerca && tripulante->idTripulante < tripulanteMasCerca->idTripulante)){
			log_info(logDiscordiador,"el tripulanteid: %d con posX: %d posY: %d desaloja al el tripulanteid: %d con posX: %d posY: %d"
			, tripulante->idTripulante, tripulante->coordenadas.posX, tripulante->coordenadas.posY,
			tripulanteMasCerca->idTripulante, tripulanteMasCerca->coordenadas.posX, tripulanteMasCerca->coordenadas.posY);

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
	if(idTripulanteBlocked == NO_HAY_TRIPULANTE_BLOQUEADO && queue_size(colaBlocked) > 0){
		lock(mutexColaBlocked);
		t_tripulante* tripulanteBlocked = (t_tripulante*) queue_peek(colaBlocked);
		unlock(mutexColaBlocked);
		idTripulanteBlocked = tripulanteBlocked->idTripulante;
		log_info(logDiscordiador,"------EL TRIPU BLOQUEADO ES %d-----", idTripulanteBlocked);
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


void iterarCola(t_queue* cola, t_estado estado){

	t_list_iterator* list_iterator = list_iterator_create(cola->elements);

	while(list_iterator_has_next(list_iterator)) {
		t_tripulante* tripulante = list_iterator_next(list_iterator);
		log_info(logDiscordiador,"Tripulante: %d    Patota: %d    Status:%s    ",
				tripulante->idTripulante, tripulante->idPatota, traducirEstado(estado));
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
			sem_post(&tripulante->semaforoInicio);
			break;

		case EXEC:
			lock(mutexColaExec);
			queue_push(colaExec, tripulante);
			unlock(mutexColaExec);
			log_info(logDiscordiador,"El tripulante %d paso a COLA EXEC", tripulante->idTripulante);
			break;

		case BLOCKED:
			if(sabotaje->haySabotaje){
				queue_push(colaSabotaje, tripulante);
				log_info(logDiscordiador,"El tripulante %d paso a COLA SABOTAJE", tripulante->idTripulante);
			}
			else{
				lock(mutexColaBlocked);
				queue_push(colaBlocked, tripulante);
				unlock(mutexColaBlocked);
				log_info(logDiscordiador,"El tripulante %d paso a COLA BLOCKED", tripulante->idTripulante);
			}
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
	//log_info(logDiscordiador, "--------------------------codigoOp es: %d -----------------------------", paqueteRecibido->codigoOperacion);
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


char* esperarConfirmacionDePatotaEnRAM(int server_socket) {

	t_paquete* paqueteRecibido = recibirPaquete(server_socket);

	char* mensajeConfirmacion = deserializarString(paqueteRecibido);

	eliminarPaquete(paqueteRecibido);
	log_info(logDiscordiador,"La confirmacion es %s",mensajeConfirmacion);


	return mensajeConfirmacion;
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
//	log_info(logDiscordiador,"La tarea %s no es IO",tarea);
	return 0;
}


uint32_t calculoCiclosExec(t_tripulante* tripulante){

	uint32_t desplazamientoEnX = diferencia(tripulante->instruccionAejecutar->coordenadas.posX, tripulante->coordenadas.posX);
	uint32_t desplazamientoEnY = diferencia(tripulante->instruccionAejecutar->coordenadas.posY, tripulante->coordenadas.posY);

	if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
		return  desplazamientoEnX + desplazamientoEnY + 1;
	}

	return desplazamientoEnX + desplazamientoEnY + tripulante->instruccionAejecutar->tiempo;
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


void listarTripulantes(){
	log_info(logDiscordiador,"Estado de la nave: %s", temporal_get_string_time("%d-%m-%y %H:%M:%S"));

	iterarCola(colaNew, NEW);
	iterarCola(colaReady, READY);
	iterarCola(colaExec, EXEC);
	iterarCola(colaBlocked, BLOCKED);

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


void eliminarTripulante(uint32_t id){

	int cantidad = 0;

	t_list * listaDeComparacion = list_create();

	list_add_all(listaDeComparacion, colaReady->elements);
	list_add_all(listaDeComparacion, colaExec->elements);
	list_add_all(listaDeComparacion, colaNew->elements);
	list_add_all(listaDeComparacion, colaBlocked->elements);

	t_list_iterator * iterator = list_iterator_create(listaDeComparacion);

	t_tripulante * tripulante;


	while(list_iterator_has_next(iterator)) {

		tripulante = list_iterator_next(iterator);

		if(tripulante->idTripulante == id){

			tripulante->estado = END;

			cantidad++;
		}

	}

	list_iterator_destroy(iterator);
	list_destroy(listaDeComparacion);


	if(cantidad == 0){
		log_info(logDiscordiador,"No se ha encontrado un tripulante con el Id: %d",id);
	}
	if(cantidad == 1){
		lock(mutexTotalTripus);
		totalTripus--;
		unlock(mutexTotalTripus);
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
