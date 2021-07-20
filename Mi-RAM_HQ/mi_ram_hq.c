#include "mi_ram_hq.h"


sem_t habilitarPatotaEnRam;

int main(void) {

	char* pathDelLogRam = pathLogRam();
	logMiRAM = iniciarLogger(pathDelLogRam,"Mi-Ram",1);

	char* pathDelLogMemoria = pathLogMemoria();
	logMemoria = iniciarLogger(pathDelLogMemoria,"Memoria",1);

	cargar_configuracion();
	iniciarMemoria();

	sem_init(&habilitarPatotaEnRam,0,1);

    int serverSock = iniciarConexionDesdeServidor(configRam.puerto);

    //Abro un thread manejar las conexiones
    pthread_t manejo_tripulante;
    pthread_create(&manejo_tripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
    pthread_join(manejo_tripulante, (void**) NULL);

    //falta hacer funcion para destruir las tareas de la lista de tareas del pcb
    //falta hacer funcion para destruir los pcb de las lista de pcbs
    //Prototipo:

     //eliminarListaTCB(listaTCB);
     //eliminarListaPCB(listaPCB);

     return EXIT_SUCCESS;
}


void atenderTripulantes(int* serverSock) {

    while(1){

    	int* tripulanteSock = malloc(sizeof(int));
		*tripulanteSock = esperarTripulante(*serverSock);

		pthread_t t;

		//pthread_t t = malloc(sizeof(pthread_t));

		pthread_create(&t, NULL, (void*) manejarTripulante, (void*) tripulanteSock);

		pthread_detach(t);
		//free(t);
		//Para hacerle free hay que pasarlo por parametro en pthread_create

		//pthread_join(t, (void**) NULL);
    }
}


int esperarTripulante(int serverSock) {

    struct sockaddr_in serverAddress;

    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (void*) &serverAddress, &len);

    log_info(logMiRAM, "Se conecto un cliente!\n");

    return socket_tripulante;
}

//CUANDO CREAS UN HILO HAY QUE PASAR SI O SI UN PUNTERO

void manejarTripulante(int *tripulanteSock) {

    t_paquete* paquete = recibirPaquete(*tripulanteSock);

    deserializarSegun(paquete,*tripulanteSock);
    free(tripulanteSock);
    //no estoy si close libera la memoria,
    //porque no recibe el puntero si no la
    //info que contiene la direccion de memoria
}


void deserializarSegun(t_paquete* paquete, int tripulanteSock){

	log_info(logMiRAM,"Deserializando el cod_op %d", paquete->codigoOperacion);

	int res;

	switch(paquete->codigoOperacion){
		case PATOTA:
			log_info(logMiRAM,"Voy a deserializar una patota");
			res = deserializarInfoPCB(paquete);
			log_info(logMiRAM,"El resultado de la operacion es: %d",res);
			if(res)
				mandarConfirmacionDisc("OK", tripulanteSock);
			else
				mandarConfirmacionDisc("ERROR", tripulanteSock);
			break;
		case TRIPULANTE:
		{
			log_info(logMiRAM,"Voy a deserializar un tripulante");
			t_tarea* tarea = deserializarTripulante(paquete);
			if(tarea != NULL)
			{
				log_info(logMiRAM,"Se le va a mandar al tripulante la tarea de: %s",tarea->nombreTarea);
				mandarTarea(tarea, tripulanteSock);
			}
			else
			{
				log_info(logMiRAM,"Se procede a enviar la TAREA ERROR");
				t_tarea* tareaError = tarea_error();
				mandarTarea(tareaError, tripulanteSock);
			}
			break;
		}
		case EXPULSAR:
			log_info(logMiRAM,"SE VA A EXPULSAR UN TRIPULANTE");
			deserializarExpulsionTripulante(paquete);
			break;

		case ESTADO_TRIPULANTE:

			log_info(logMiRAM,"Voy a actualizar un tripulante");
			res = recibirActualizarTripulante(paquete);
			log_info(logMiRAM,"El resultado de la operacion es: %d",res);
			if(res)
				mandarConfirmacionDisc("OK", tripulanteSock);
			else
				mandarConfirmacionDisc("ERROR", tripulanteSock);
			break;

		case SIGUIENTE_TAREA:
		{
			log_info(logMiRAM, "Se le asigna la prox tarea a un tripulante");
			t_tarea* tarea = deserializarSolicitudTarea(paquete);
			log_info(logMiRAM,"Se le va a mandar al tripulante la tarea de: %s",tarea->nombreTarea);

			if(tarea)
				mandarTarea(tarea, tripulanteSock);
			else
			{
				mandarTarea(tarea_error(), tripulanteSock);
			}
			break;
		}
		default:
			log_info(logMiRAM,"No se puede deserializar ese tipo de estructura negro \n");
			exit(1);
		}
	eliminarPaquete(paquete);
	close(tripulanteSock);
}


void mandarConfirmacionDisc(char* aMandar, int socket) {

	t_paquete* aEnviar = armarPaqueteCon((void*) aMandar, STRING);
	log_info(logMiRAM,"Mandando confirmacion de %s a Discordiador",aMandar);
	enviarPaquete(aEnviar,socket);
}


void cargar_configuracion() {
	t_config* config = config_create("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/mi_ram_hq.config"); //Leo el archivo de configuracion

	if (config == NULL) {
		perror("Archivo de configuracion de RAM no encontrado");
		return;
	}

	configRam.tamanioMemoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	configRam.esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	configRam.tamanioPagina = config_get_int_value(config, "TAMANIO_PAGINA");
	configRam.tamanioSwap = config_get_int_value(config, 	"TAMANIO_SWAP");
	configRam.pathSwap = config_get_string_value(config, 	"PATH_SWAP");
	configRam.algoritmoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	configRam.criterioSeleccion = config_get_string_value(config, "CRITERIO_SELECCION");
	configRam.puerto = config_get_int_value(config, "PUERTO");

	//config_destroy(config);
}


char* asignar_bytes(int cant_frames)
{
    char* buf;
    int bytes;
    if(cant_frames < 8)
        bytes = 1;
    else
    {
        double c = (double) cant_frames;
        bytes = (int) ceil(c/8.0);
    }
    //log_info(logMemoria,"BYTES: %d\n", bytes);
    buf = malloc(bytes);
    memset(buf,0,bytes);
    return buf;
}


void iniciarMemoria() {

	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		tablasPaginasPatotas = list_create();
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		tablasSegmentosPatotas = list_create();
	}

    signal(SIGUSR1, hacerDump);
    signal(SIGUSR2, compactarMemoria);
    log_info(logMemoria,"PID: %d",process_getpid());

    pthread_mutex_init(&mutexMemoria, NULL);
    pthread_mutex_init(&mutexEscribirMemoria, NULL);
    pthread_mutex_init(&mutexEscribirMemoriaVirtual, NULL);
    pthread_mutex_init(&mutexTablasPaginas, NULL);
    pthread_mutex_init(&mutexTablasSegmentos, NULL);
    pthread_mutex_init(&mutexBuscarLugarLibre, NULL);
    pthread_mutex_init(&mutexExpulsionTripulante, NULL);
    pthread_mutex_init(&mutexTablaSegmentosPatota, NULL);
    pthread_mutex_init(&mutexTablaPaginasPatota, NULL);
    pthread_mutex_init(&mutexBitarray, NULL);
    pthread_mutex_init(&mutexAlojados, NULL);
    pthread_mutex_init(&mutexTiempo, NULL);


	sem_init(&habilitarExpulsionEnRam,0,1);

	tiempo = 0;

	log_info(logMemoria, "TAMANIO RAM: %d", configRam.tamanioMemoria);

	memoria_principal = malloc(configRam.tamanioMemoria);

	//memset(memoria_principal,'$',configRam.tamanioMemoria);

	cant_frames_ppal = configRam.tamanioMemoria / configRam.tamanioPagina;
    log_info(logMemoria, "RAM FRAMES: %d", cant_frames_ppal);
    char* data = asignar_bytes(cant_frames_ppal);
    frames_ocupados_ppal = bitarray_create_with_mode(data, cant_frames_ppal/8, MSB_FIRST);

    cant_frames_virtual = configRam.tamanioSwap / configRam.tamanioPagina;
    log_info(logMemoria, "SWAP FRAMES: %d\n",cant_frames_virtual);
    char* data2 = asignar_bytes(cant_frames_virtual);
    frames_ocupados_virtual = bitarray_create_with_mode(data2, cant_frames_virtual/8, MSB_FIRST);

    lugaresLibres = list_create();
    t_lugarLibre* lugarInicial = malloc(sizeof(t_lugarLibre));
    lugarInicial->inicio = 0;
    lugarInicial->bytesAlojados = configRam.tamanioMemoria;
    list_add(lugaresLibres,lugarInicial);

	log_info(logMemoria,"El esquema usado es %s", configRam.esquemaMemoria);

}


int recibirActualizarTripulante(t_paquete* paquete) {
	tcb* nuevoTCB = malloc(sizeof(tcb));

	uint32_t idPatota;
	t_estado estado;

	void* stream = paquete->buffer->stream;

	int offset=0;

	memcpy(&(idPatota),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->idTripulante),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(estado),stream + offset,sizeof(t_estado));
	offset += sizeof(t_estado);
	nuevoTCB->estado = asignarEstadoTripu(estado);

	memcpy(&(nuevoTCB->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	log_info(logMiRAM,"Recibi el tribulante de id %d de la  patota %d en estado %c, en pos X: %d | pos Y: %d",
			nuevoTCB->idTripulante, idPatota, nuevoTCB->estado, nuevoTCB->posX, nuevoTCB->posY);

	int confirmacion = actualizarTripulante(nuevoTCB,idPatota);

	free(nuevoTCB);
	return confirmacion;

}


t_tarea* deserializarSolicitudTarea(t_paquete* paquete) {

	uint32_t idPatota;
	uint32_t idTripulante;
	void * stream = paquete->buffer->stream;
	int offset = 0;
	memcpy(&(idPatota), stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(idTripulante), stream + offset, sizeof(uint32_t));

	t_tarea* tarea = asignarProxTarea(idPatota, idTripulante);

	if(strcmp(tarea->nombreTarea, "TAREA_NULA") == 0)
	{
		expulsarTripulante(idTripulante, idPatota);
	}

	return tarea;
}


void deserializarExpulsionTripulante(t_paquete* paquete) {

	uint32_t idPatota;
	uint32_t idTripu;

	void* stream = paquete->buffer->stream;

	int offset=0;

	memcpy(&(idPatota),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(idTripu),stream + offset,sizeof(uint32_t));

	log_info(logMiRAM, "Se procede a eliminar de la memoria el tripulante %d de la patota %d", idTripu, idPatota);


	sem_wait(&habilitarExpulsionEnRam);
	//lock(&mutexExpulsionTripulante);
	expulsarTripulante(idTripu,idPatota);
	//unlock(&mutexExpulsionTripulante);
}


int deserializarInfoPCB(t_paquete* paquete) {

	pcb* nuevoPCB = malloc(sizeof(pcb));
	void* stream = paquete->buffer->stream;
	uint32_t tamanio;
	int offset = 0;

	memcpy(&(nuevoPCB->pid),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(tamanio),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	char* stringTareas = malloc(tamanio);
	memcpy(stringTareas,stream + offset,tamanio);

	char* tareasDelimitadas = delimitarTareas(stringTareas);

	free(stringTareas);

	//sem_wait(&habilitarPatotaEnRam);
	return guardarPCB(nuevoPCB,tareasDelimitadas);

}


char* delimitarTareas(char* stringTareas) {

    char* tareasDelimitadas = string_new();

    char** arrayDeTareas = string_split(stringTareas,"\n");

    for(int i =0; arrayDeTareas[i] != NULL; i++){

        string_trim(&arrayDeTareas[i]);
        string_append(&arrayDeTareas[i],"|");
        string_append(&tareasDelimitadas, arrayDeTareas[i]);
    }

    tareasDelimitadas = string_substring_until(tareasDelimitadas, strlen(tareasDelimitadas) - 1);

    liberarDoblesPunterosAChar(arrayDeTareas);

    log_info(logMiRAM, "Tareas delimitadas: %s", tareasDelimitadas);

    return tareasDelimitadas;
}


t_tarea* deserializarTripulante(t_paquete* paquete) {

	tcb* nuevoTCB = malloc(sizeof(tcb));

	uint32_t idPatota;
	t_estado estado;

	void* stream = paquete->buffer->stream;

	int offset=0;

	memcpy(&(idPatota),stream,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->idTripulante),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(estado),stream + offset,sizeof(t_estado));
	offset += sizeof(t_estado);
	nuevoTCB->estado = asignarEstadoTripu(estado);

	memcpy(&(nuevoTCB->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	log_info(logMiRAM,"Recibi el tribulante de id %d de la  patota %d en estado %c",nuevoTCB->idTripulante, idPatota, nuevoTCB->estado);

	if(nuevoTCB->idTripulante == -1){
		sem_post(&habilitarPatotaEnRam);
		return tarea_error();
	}

	t_tarea* tarea = guardarTCB(nuevoTCB,idPatota);

	free(nuevoTCB);

	return tarea;

	//NOS QUEDAMOS ACA
	//BUSCAR EN RAM LA TAREA A DARLE, YA ENTRA EN JUEGO EL TEMA DE IR A BUSCAR A LAS PAGINAS SEGUN DESPLAZ ETC.
}


char asignarEstadoTripu(t_estado estado) {

	if(estado == NEW) return 'N';
	if(estado == READY) return 'R';
	if(estado == EXEC) return 'E';
	if(estado == BLOCKED) return 'B';
	else{
		log_error(logMiRAM, "El estado recibido del tripulante es invalido");
		exit(1);
	}

}


void mandarTarea(t_tarea* tarea, int socketTrip) {

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tarea, TAREA);

	log_info(logMiRAM,"Tarea enviada\n");

	enviarPaquete(paqueteEnviado, socketTrip);

	free(tarea->nombreTarea);
	free(tarea);
}


t_tarea* tarea_error() {
	t_tarea* tareaError = malloc(sizeof(t_tarea));
	tareaError->nombreTarea = strdup("TAREA_ERROR");
	tareaError->parametro = 0;
	tareaError->coordenadas.posX = 0;
	tareaError->coordenadas.posY = 0;
	tareaError->tiempo = 0;

	return tareaError;
}

t_tarea* tarea_nula() {
	t_tarea* tareaError = malloc(sizeof(t_tarea));
	tareaError->nombreTarea = strdup("TAREA_NULA");
	tareaError->parametro = 0;
	tareaError->coordenadas.posX = 0;
	tareaError->coordenadas.posY = 0;
	tareaError->tiempo = 0;

	return tareaError;
}



tcb* cargarEnTripulante(void* bufferTripu) {
	tcb* nuevoTCB = malloc(sizeof(tcb));
	int offset = 0;

	memcpy(&(nuevoTCB->idTripulante),bufferTripu + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->estado),bufferTripu + offset,sizeof(char));
	offset += sizeof(char);

	memcpy(&(nuevoTCB->posX),bufferTripu + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->posY),bufferTripu + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->proximaAEjecutar),bufferTripu + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(nuevoTCB->dlPatota),bufferTripu + offset,sizeof(uint32_t));

	return nuevoTCB;

}


void cargarDLTripulante(void* bufferTripu, tcb* tcbAGuardar) {
	int offset=13;

	memcpy(&(tcbAGuardar->proximaAEjecutar),bufferTripu + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(&(tcbAGuardar->dlPatota),bufferTripu + offset,sizeof(uint32_t));

}


t_tarea* armarTarea(char* string){

	t_tarea* tarea = malloc(sizeof(t_tarea));

	    char** arrayParametros = string_split(string,";");

	    if(string_contains(arrayParametros[0]," ")){

	    	char** arrayPrimerElemento = string_split(arrayParametros[0]," ");
	    	tarea->nombreTarea = strdup(arrayPrimerElemento[0]);
	    	tarea->parametro= (int) atoi(arrayPrimerElemento[1]);
			liberarDoblesPunterosAChar(arrayPrimerElemento);

	    } else {
	        tarea->nombreTarea = strdup(arrayParametros[0]);
	    }
	    tarea->coordenadas.posX = (uint32_t) atoi(arrayParametros[1]);
	    tarea->coordenadas.posY = (uint32_t) atoi(arrayParametros[2]);
	    tarea->tiempo = (uint32_t) atoi(arrayParametros[3]);
	    liberarDoblesPunterosAChar(arrayParametros);

	    return tarea;
}




void* meterEnBuffer(void* aGuardar, tipoEstructura tipo, int* aMeter, int *datoAdicional) {

	void* buffer;
	int offset = 0;

	switch(tipo)
	{
		case PCB:
		{
			pcb* pcbAGuardar = (pcb*) aGuardar;
			*aMeter = 8;
			buffer = malloc(*aMeter);
			*datoAdicional = -1;
			log_info(logMemoria,"DL TAREA: %d \n", pcbAGuardar->dlTareas);

			memcpy(buffer, &(pcbAGuardar->pid), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(pcbAGuardar->dlTareas), sizeof(uint32_t));
			break;
		}
		case TCB:
		{
			tcb* tcbAGuardar = (tcb*) aGuardar;
			*aMeter = 21;
			buffer = malloc(*aMeter);
			*datoAdicional = tcbAGuardar->idTripulante;
			memcpy(buffer + offset, &(tcbAGuardar->idTripulante),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->estado),sizeof(char));
			offset += sizeof(char);
			memcpy(buffer + offset, &(tcbAGuardar->posX),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->posY),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->proximaAEjecutar),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->dlPatota),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			break;
		}
		case TAREAS:
		{
			char* tareas = (char*) aGuardar;
			*aMeter = strlen(tareas) + 1;
			buffer = malloc(*aMeter);
			*datoAdicional = -1;
			memcpy(buffer, tareas, strlen(tareas) + 1);
			break;
		}
		default:
			log_error(logMemoria,"No puedo guardar eso en una pagina negro");
			exit(1);
	}

	return buffer;
}




int guardarPCB(pcb* pcbAGuardar, char* stringTareas) {

	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		return guardarPCBPag(pcbAGuardar, stringTareas);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		return guardarPCBSeg(pcbAGuardar, stringTareas);
	}
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}


t_tarea* guardarTCB(tcb* tcbAGuardar, int idPatota) {
	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		return guardarTCBPag(tcbAGuardar, idPatota);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		return guardarTCBSeg(tcbAGuardar, idPatota);
	}
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}


void expulsarTripulante(int idTripu,int idPatota) {
	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		expulsarTripulantePag(idTripu, idPatota);
	}
	else if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		expulsarTripulanteSeg(idTripu, idPatota);
	}
	else {
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
	}
	sem_post(&habilitarExpulsionEnRam);
}


t_tarea* asignarProxTarea(int idPatota, int idTripu) {

	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		return asignarProxTareaPag(idPatota, idTripu);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		return asignarProxTareaSeg(idPatota, idTripu);
	}

	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}


int actualizarTripulante(tcb* tcbAGuardar, int idPatota){
	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		return actualizarTripulantePag(tcbAGuardar, idPatota);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		return actualizarTripulanteSeg(tcbAGuardar, idPatota);
	}
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}


void hacerDump(int signal) {
	if(strcmp(configRam.esquemaMemoria,"PAGINACION") == 0)
	{
		dumpPag();
	}
	else if(strcmp(configRam.esquemaMemoria,"SEGMENTACION") == 0)
	{
		dumpSeg();
	}
	else {
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
	}
}

char * pathLogRam(){
	char *pathLog = string_new();
	char *fecha = temporal_get_string_time("%d-%m-%y %H:%M:%S");
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/logsRam/");
	string_append(&pathLog, "logRam_ ");
	string_append(&pathLog, fecha);
	string_append(&pathLog, ".log");
	free(fecha);
	return pathLog;
}

char * pathLogMemoria(){
	char *pathLog = string_new();
	char *fecha = temporal_get_string_time("%d-%m-%y %H:%M:%S");
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/logsMemoria/");
	string_append(&pathLog, "logMemoria_ ");
	string_append(&pathLog, fecha);
	string_append(&pathLog, ".log");
	free(fecha);
	return pathLog;
}






