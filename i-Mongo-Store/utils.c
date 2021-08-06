#include "utils.h"


void crearConfig(t_config** config, char* path){

	*config  = config_create(path);
	if(*config == NULL){
		log_error(logImongo, "La ruta es incorrecta ");
		exit(1);
	}
}

char * pathLog(){

	char *pathLog = string_new();
	char *fecha = temporal_get_string_time("%d-%m-%y %H:%M:%S");
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/logs/");
	string_append(&pathLog, "log ");
	string_append(&pathLog, fecha);
	string_append(&pathLog, ".log");
	free(fecha);
	return pathLog;
}


char* crearDestinoApartirDeRaiz(char* destino){

	char* raiz = string_new();

	string_append(&raiz, datosConfig->puntoMontaje);
	string_append(&raiz, "/");
	string_append(&raiz, destino);

	return raiz;
}


void asignarTareas(){

	tareas = malloc(sizeof(char*) * 6);

	tareas[0] = strdup("GENERAR_OXIGENO");
	tareas[1] = strdup("CONSUMIR_OXIGENO");
	tareas[2] = strdup("GENERAR_COMIDA");
	tareas[3] = strdup("CONSUMIR_COMIDA");
	tareas[4] = strdup("GENERAR_BASURA");
	tareas[5] = strdup("DESCARTAR_BASURA");
}


int indiceTarea(t_tarea* tarea){

	for(int i=0; tareas[i] != NULL; i++){
		if(string_contains(tarea->nombreTarea, tareas[i])){
			return i;
		}
	}
	return -1;
}


void cargarPaths(){

	superBloque->path = crearDestinoApartirDeRaiz("SuperBloque.ims");
	pathBloque = crearDestinoApartirDeRaiz("Blocks.ims");
	pathFiles = crearDestinoApartirDeRaiz("Files");
	oxigeno->path = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	comida->path = crearDestinoApartirDeRaiz("Files/Comida.ims");
	basura->path = crearDestinoApartirDeRaiz("Files/Basura.ims");
	pathBitacoras = crearDestinoApartirDeRaiz("Files/Bitacora");

}


void cargarDatosConfig(){

	datosConfig = malloc(sizeof(t_datosConfig));
	datosConfig->puntoMontaje = config_get_string_value(configImongo,"PUNTO_MONTAJE");
	datosConfig->puerto = (uint32_t)config_get_int_value(configImongo,"PUERTO");
	datosConfig->tiempoSincronizacion = (uint32_t)config_get_int_value(configImongo,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = config_get_string_value(configImongo,"POSICIONES_SABOTAJE");

	puertoEIPDisc = malloc(sizeof(puertoEIP));
	puertoEIPDisc->puerto = config_get_int_value(configImongo,"PUERTO_DISC");
	puertoEIPDisc->IP = strdup(config_get_string_value(configImongo,"IP_DISC"));
}


void iniciarMutex(){

	pthread_mutex_init(&mutexMemoriaSecundaria,NULL);
	pthread_mutex_init(&mutexBitMap,NULL);
	pthread_mutex_init(&mutexEstructurasFile,NULL);
	pthread_mutex_init(&oxigeno->mutex,NULL);
	pthread_mutex_init(&comida->mutex,NULL);
	pthread_mutex_init(&basura->mutex,NULL);
	pthread_mutex_init(&mutexExisteBitacora,NULL);
	pthread_mutex_init(&mutexCantEscriturasPendientes,NULL);
	pthread_mutex_init(&mutexHaySabotaje,NULL);
	pthread_mutex_init(&mutexSP,NULL);
	pthread_mutex_init(&mutexSemaforosTareas,NULL);
}


int leerHaySabotaje(){
	lock(&mutexHaySabotaje);
	bool sabotaje = haySabotaje;
	unlock(&mutexHaySabotaje);
	return sabotaje;
}


void modificarHaySabotaje(bool valor){
	lock(&mutexHaySabotaje);
	haySabotaje = valor;
	unlock(&mutexHaySabotaje);
}


int leerCantEscriturasPendientes(){
	lock(&mutexCantEscriturasPendientes);
	int cant = cantEscriturasPendientes;
	unlock(&mutexCantEscriturasPendientes);
	return cant;
}


void modificarCantEscriturasPendientes(int cant){
	lock(&mutexCantEscriturasPendientes);
	cantEscriturasPendientes = cant;
	unlock(&mutexCantEscriturasPendientes);
}


bool validarExistenciaFileSystem(char* superBloque, char* blocks, char* raiz){
	return (access(superBloque, F_OK ) != -1) && (access(blocks, F_OK ) != -1) && (access(raiz, F_OK ) != -1);
}


void detallesArchivo(int fileDescriptor){

	struct stat sb;

	if (fstat(fileDescriptor, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	log_info(logImongo, "Tamanio total del archivo: %lu bytes\n", sb.st_size);
	log_info(logImongo, "Ultimo estado:       %s", ctime(&sb.st_ctime));
	log_info(logImongo, "Ultimo acceso:         %s", ctime(&sb.st_atime));
	log_info(logImongo, "Ultima modificacion del archivo:   %s", ctime(&sb.st_mtime));
}


void crearMemoria(int fd){

	int size = superBloque->block_size * superBloque->blocks;

	int result = lseek(fd, size - 1,SEEK_SET);

	 if (result == -1) {
	      close(fd);
	      log_info(logImongo,"Anda mal el file descriptor del disco secundario");
	      exit(1);
	  }

	result = write(fd,"",1);

	if (result < 0) {
		close(fd);
	    log_info(logImongo,"Anda mal generar con "" el espacio en el disco secundario");
	    exit(1);
	}

	lseek(fd, 0,SEEK_SET);

	memoriaSecundaria = mmap(NULL,size, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);

	lock(&mutexMemoriaSecundaria);
	copiaMemoriaSecundaria= malloc(size);
	memcpy(copiaMemoriaSecundaria,memoriaSecundaria, size);
	unlock(&mutexMemoriaSecundaria);

	log_info(logImongo,"Se ha creado la memoria secundaria con la capacidad %d con su copia para sincronizar", size);

	detallesArchivo(fd);
}


bool verificarSiExiste(char* pathArchivo){
	return access(pathArchivo,F_OK) != -1;
}


int min(int a,int b){

    if(a>b){
        return b;
    }
    else{
        return a;
    }
}


int max(int a,int b){

    if(a>b){
        return a;
    }
    else{
        return b;
    }
}


void sincronizarMemoriaSecundaria(){

	int tiempoEspera = datosConfig->tiempoSincronizacion;

	int size = superBloque->block_size * superBloque->blocks;

	while(1){

		sleep(tiempoEspera);

		lock(&mutexMemoriaSecundaria);
		memcpy(memoriaSecundaria,copiaMemoriaSecundaria,size);
		unlock(&mutexMemoriaSecundaria);

		int flag = msync(memoriaSecundaria,size,MS_SYNC);

		if(flag ==0){
			log_info(logImongo, "Se sincronizo la memoria con éxito");
		}
		else if(flag == -1){
			log_error(logImongo, "No se sincronizo bien la memoria");
		}
		else{
			log_error(logImongo, "La sincronizacion esta arrojando cualquier valor");
		}
	}
}


char * reconstruirArchivo(t_list * bloques){

	uint32_t cantidadBloques = list_size(bloques);
	uint32_t tamanio = cantidadBloques*superBloque->block_size;

	char* lista = convertirListaEnString(bloques);

	//log_info(logImongo,"Los numeros dentro de la lista de bloque son es: ----%s---- y la cantidad de bloques es: ----%d----",lista,cantidadBloques);

	free(lista);

	char * copiaFile = malloc(tamanio + 1);

	for(int i=0; i<cantidadBloques;i++){
		uint32_t * bloque = list_get(bloques, i);
		//log_info(logImongo,"Numero del bloque es %d",*bloque);
		uint32_t offsetMemoria =  (*bloque) * superBloque->block_size;
		uint32_t offsetFile = i * superBloque->block_size;
		//log_info(logImongo,"offset memoria %d",offsetMemoria);
		//log_info(logImongo,"offset file %d",offsetFile);
		lock(&mutexMemoriaSecundaria);
		memcpy(copiaFile + offsetFile, copiaMemoriaSecundaria + offsetMemoria, superBloque->block_size);
		unlock(&mutexMemoriaSecundaria);
	}
	copiaFile[tamanio] = '\0';
	log_info(logImongo,"archivo reconstruido es: %s",copiaFile);
	return copiaFile;

}


char * obtenerMD5(t_list * bloques){

	uint32_t md5Size = 32;

	char* copiaFile = reconstruirArchivo(bloques);
	char * comando = string_from_format("echo -n \"%s\" | md5sum", copiaFile);
	char * md5 = malloc(md5Size +2);
	FILE * file = popen(comando, "r");

	fgets(md5, md5Size+1, file);

	fclose(file);
	free(comando);
	free(copiaFile);

	return md5;
}


t_list* convertirEnLista(char** arrayValores){

	t_list* lista = list_create();

	for(uint32_t i=0; *(arrayValores + i) != NULL; i++){

		uint32_t* valor = malloc(sizeof(uint32_t));
		*valor = atol(*(arrayValores + i));

		list_add(lista, valor);
	}

	return lista;
}


char* convertirListaEnString(t_list* listaEnteros){

	char* stringLista = string_new();
	string_append(&stringLista, "[");

	if(!list_is_empty(listaEnteros)){

		for(uint32_t i=0; i < list_size(listaEnteros) - 1; i++){

			string_append_with_format(&stringLista, "%d, ",
					*(long int*)(list_get(listaEnteros, i)));
		}

		string_append_with_format(&stringLista, "%d", *(int*)(list_get(listaEnteros, list_size(listaEnteros) - 1)));
	}

	string_append(&stringLista, "]");

	return stringLista;
}


t_desplazamiento* deserializarDesplazamiento(void* stream){

	t_desplazamiento* desplazamiento = malloc(sizeof(t_desplazamiento));

	int offset = 0;

	memcpy(&(desplazamiento->idTripulante), stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(desplazamiento->inicio.posX), stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(desplazamiento->inicio.posY), stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(desplazamiento->fin.posX), stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(desplazamiento->fin.posY), stream + offset, sizeof(uint32_t));

	return desplazamiento;
}


t_avisoTarea* deserializarAvisoTarea(void* stream){

	t_avisoTarea* avisoTarea = malloc(sizeof(t_avisoTarea));
	uint32_t tamanioNombreTarea;
	int offset=0;

	memcpy(&(avisoTarea->idTripulante), stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&tamanioNombreTarea, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	avisoTarea->nombreTarea = malloc(tamanioNombreTarea);
	memcpy(avisoTarea->nombreTarea, stream + offset,  tamanioNombreTarea);
	offset += tamanioNombreTarea;
	memcpy(&(avisoTarea->numero), stream + offset, sizeof(int));

	return avisoTarea;
}


uint32_t deserializarID(void* stream){

	uint32_t *id = malloc(sizeof(uint32_t));
	memcpy(id, stream, sizeof(uint32_t));

	uint32_t idTripulante = *id;

	free(id);

	return idTripulante;
}


void crearBitacora(char* idTripulante){

	t_bitacora_tripulante* bitacoraTripulante = malloc(sizeof(t_bitacora_tripulante));
	bitacoraTripulante->path = pathBitacoraTripulante(idTripulante);
	bitacoraTripulante->tamanioArchivo = 0;
	bitacoraTripulante->bloques = list_create();
	pthread_mutex_init(&bitacoraTripulante->mutex, NULL);

	FILE* b = fopen(bitacoraTripulante->path,"wb");
	fclose(b);

	actualizarBitacora(bitacoraTripulante);

	dictionary_put(bitacoras, idTripulante, bitacoraTripulante);
}


char* pathBitacoraTripulante(char* idTripulante){
	return string_from_format("%s/Tripulante%s.ims", pathBitacoras, idTripulante);
}


void liberarEstructuraDatosConfig(){
	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);
}


t_list* listaCoordenadasSabotaje() {

	//char** arrayStringsCoordenadas = config_get_array_value(configImongo, "POSICIONES_SABOTAJE");

    char* stringCoordenadas = datosConfig->posicionesSabotaje;
    stringCoordenadas = string_substring(stringCoordenadas, 1, strlen(stringCoordenadas)-2 ); //Le saco los corchetes
    //Aca puede ir un log info para chequear que se haya hecho
    char** arrayStringsCoordenadas = string_split(stringCoordenadas, ",");

    t_list* listaCoordenadas = list_create();
    int i = 0;
    while(arrayStringsCoordenadas[i] != NULL) {

        char** parCoordenadas = string_split(arrayStringsCoordenadas[i], "|");

        t_coordenadas* coordenadasSabotaje = malloc(sizeof(t_coordenadas));

        coordenadasSabotaje->posX = (uint32_t) atoi(parCoordenadas[0]);
        coordenadasSabotaje->posY = (uint32_t) atoi(parCoordenadas[1]);

        log_info(logImongo, "SE CARGO LA COORDENADA %d|%d A LA LISTA",
        		coordenadasSabotaje->posX, coordenadasSabotaje->posY);

        list_add(listaCoordenadas, coordenadasSabotaje);

        liberarDoblesPunterosAChar(parCoordenadas);
        i++;
    }

    liberarDoblesPunterosAChar(arrayStringsCoordenadas);
    free(stringCoordenadas);

    return listaCoordenadas;
}


void sabotaje(int signal) {

	modificarHaySabotaje(true);
	t_coordenadas* coordenadasSabotaje = list_get(listaPosicionesSabotaje, proximoPosSabotaje);

	proximoPosSabotaje++;
	if(proximoPosSabotaje == list_size(listaPosicionesSabotaje)){
		log_info(logImongo, "ENTRO ACA");
		proximoPosSabotaje = 0;
	}

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) coordenadasSabotaje, COORDENADAS_SABOTAJE);

	log_info(logImongo,"\nSABOTAJE - AVISANDO A DISCORDADOR QUE ES EN POS: X:%d - Y:%d\n",
			coordenadasSabotaje->posX, coordenadasSabotaje->posY);

	int socketDisc = iniciarConexionDesdeClienteHacia((void*) puertoEIPDisc);
	enviarPaquete(paqueteEnviado, socketDisc);
	close(socketDisc);
}


void generarTarea(t_file* archivo, uint32_t cantidad){

	lock(&archivo->mutex);
	uint32_t fragmentacionArchivo = fragmentacion(archivo->tamanioArchivo);//SACA LA FRAGMENTACION DE ACUERDO AL ARCHIVO
	log_info(logImongo, "El archivo %s tiene una fragmentacion de %d", archivo->path, fragmentacionArchivo);
	uint32_t bloque = 0;

	lock(&mutexBitMap);
	t_list* bloquesAocupar = buscarBloques(max(0, cantidad - fragmentacionArchivo));//BUSCA LOS BLOQUES LIBRES BIT ARRAY Y LOS METE EN UNA LISTA
	actualizarSuperBloque();
	unlock(&mutexBitMap);

	if(!list_is_empty(bloquesAocupar) || cantidad <= fragmentacionArchivo){//SI LOS BLOQUES A OCUPAR NO SON 0 Y SI CANTIDAD DE CARACTERES A GENERAR ES MENOR O IGUAL A LA FRAGMENTACION DEL ARCHIVO

		archivo->tamanioArchivo += cantidad;//SE ACTUALIZA LA ESTRUCTURA CON EL TAMAÑO NUEVO PORQUE EN ESTE CASO LO UNICO QUE SE MODIFICA ES EL TAMAÑO
		log_info(logImongo, "Se agrego %d bytes al archivo %s y paso a tener un tamanio de %d",
				cantidad, archivo->path, archivo->tamanioArchivo);



		if(fragmentacionArchivo > 0){// SI LA FRAGMENTACION ES MAYOR A 0

			bloque = * (uint32_t*) list_get(archivo->bloques, list_size(archivo->bloques) - 1);//ESCRIBIR EN EL ULTIMO BLOQUE DE LA LISTA
			char* aGuardarUltimoBloque = string_repeat(*archivo->caracterLlenado, min(fragmentacionArchivo, cantidad));
			escribirBloque(bloque, aGuardarUltimoBloque ,superBloque->block_size - fragmentacionArchivo);//SE ESCRIBE EL EQUIVALENTE A LA FRAGMENTACION EN EL ULTIMO BLOQUE
			cantidad -= fragmentacionArchivo;//SE LE RESTA LA FRAGMENTACION RECIEN OCUPADA A LA CANTIDAD DE CARACTERES QUE SE IBAN A CONSUMIR DE UN PRINCIPIO
			free(aGuardarUltimoBloque);
		}

		while(!list_is_empty(bloquesAocupar)){//MIENTRAS EXISTAN BLOQUES A OCUPAR

			list_add(archivo->bloques, list_remove(bloquesAocupar, 0));//AGREGA LOS BLOQUES QUE VA A GUARDAR A LA ESTRUCTURA ADMINISTRATIVA
			bloque = * (uint32_t*) list_get(archivo->bloques, list_size(archivo->bloques) - 1);// AGARRA EL NUMERO DE BLOQUE A GUARDAR
			log_info(logImongo, "Se agrego el bloque %d al archivo %s", bloque, archivo->path);
			char* aGuardar = string_repeat(*archivo->caracterLlenado, min(superBloque->block_size, cantidad));
			escribirBloque(bloque, aGuardar, 0);// SE ESCRIBE EN EL BLOQUE, EL 0 SIGNIFICA BLOQUE ENTERO
			//ocuparBloque(bloque);//ACTUALIZA BIT ARRAY
			cantidad -= superBloque->block_size;// DECREMENTA LA CANTIDAD DE CARACTERES A GUARDAR (LA CANTIDAD DEL TAMAÑO DEL BLOQUE)
			free(aGuardar);
		}


		actualizarFile(archivo);//ACTUALIZA META DATA CON ESTRUCTURAS ADMINISTRATIVAS
	}
	else{
		log_error(logImongo,"--- NO HAY BLOQUES DISPONIBLES PARA LA TAREA ---");
		unlock(&mutexBitMap);
	}

	unlock(&archivo->mutex);

	list_destroy(bloquesAocupar);
}


void consumirTarea(t_file* archivo, uint32_t cantidad){

	lock(&archivo->mutex);

	cantidad = min(cantidad, archivo->tamanioArchivo);// FIJA LA VARIABLE CANTIDAD COMO EL MINIMO ENTRE LO QUE HAY Y LO QUE HAY Q CONSUMIR, ES DECIR, SI HAY Q CONSUMIR 7 Y HAY 5, SE CONSUMEN 5
	uint32_t fragmentacionArchivo = fragmentacion(archivo->tamanioArchivo);// GUARDAMOS LA FRAGMENTACIÓN A CUANDO QUEREMOS CONSUMIR
	log_info(logImongo, "El archivo %s tiene una fragmentacion de %d", archivo->path, fragmentacionArchivo);

	archivo->tamanioArchivo -= cantidad;//LE RESTAMOS AL ARCHIVO LA CANTIDAD QUE SE VA A CONSUMIR
	log_info(logImongo, "Se consumio %d bytes del archivo %s y paso a tener un tamanio de %d",
			cantidad, archivo->path, archivo->tamanioArchivo);

	unlock(&mutexBitMap);

	if(fragmentacionArchivo > 0){ // SI LA FRAGMENTACION ES MAYOR A 0

		uint32_t* bloqueFragmentacion = (uint32_t*) list_remove(archivo->bloques, list_size(archivo->bloques) - 1);
		uint32_t tamanioUltBloque = superBloque->block_size - fragmentacionArchivo; // GUARDO EN UNA VARIABLE EL TAMAÑO DEL ÚLTIMO BLOQUE
		consumirBloque(*bloqueFragmentacion, min(cantidad, tamanioUltBloque), tamanioUltBloque);//  SE CONSUME DEL ULTIMO BLOQUE EL MINIMO ENTRE LA CANTIDAD A CONSUMIR Y EL TAMANIO QUE HAY
		if(cantidad >= tamanioUltBloque){ //SI LA CANTIDAD A CONSUMIR ES MAYOR O IGUAL A LO QUE HABIA EN EL ULTIMO BLOQUE
			cantidad -= tamanioUltBloque; //LE RESTO LO QUE CONSUMI DEL ULTIMO BLOQUE A LO QUE ME QUEDA POR CONSUMIR
			liberarBloque(*bloqueFragmentacion);//LIBERO BLOQUE EN EL BITMAP
			free(bloqueFragmentacion);
		}
		else{
			cantidad = 0;// ESTE CASO ES CUANDO LA FRAGMENTACION ALCANZO A CONSUMIR, NO QUEDA MAS CANATIDAD POR CONSUMIR
			list_add(archivo->bloques, bloqueFragmentacion);
		}
	}

	while(cantidad > 0){//MIENTRAS HAYA CARACTERES POR CONSUMIR

		uint32_t* bloque = (uint32_t*) list_remove(archivo->bloques, list_size(archivo->bloques) - 1);// SE AGARRA EL BLOQUE QUE ESTA EN LA ULTIMA POSICION
		consumirBloque(*bloque, min(cantidad, superBloque->block_size), superBloque->block_size);// SE CONSUME SIEMPRE LA CANTIDAD DEL BLOQUE COMO MAXIMO
		if(cantidad >= superBloque->block_size){//SI LA CANTIDAD A CONSUMIR ES MAYOR A LA DEL BLOQUE
			cantidad -= superBloque->block_size; // LE RESTO EL TAMAÑO DEL BLOQUE
			liberarBloque(*bloque);
			free(bloque);

		}
		else{
			cantidad = 0;//SI SOLO ES LA FRAGMENTACION INTERNA LO QUE SE CONSUMIO, NO SE PUEDE CONSUMIR MAS
			list_add(archivo->bloques, bloque);// COMO SE SACO EL ULTIMO BLOQUE, SE VUELVE A AGREGAR EN LA ESTRUCTURA ADMINISTRATIVA
		}
	}

	actualizarSuperBloque();// SE ACTUALIZA SUPERBLOQUE

	unlock(&mutexBitMap);

	actualizarFile(archivo);//SE ACTUALIZA LA METADATA

	unlock(&archivo->mutex);
}


// LA CREACION DEL ARCHIVO LA HAGO APARTE
void escribirBitacora(char* idTripulante, char* mensaje){

	t_bitacora_tripulante* bitacora = dictionary_get(bitacoras, idTripulante);
	int tamanioMensaje = strlen(mensaje);
	uint32_t offsetMensaje = 0;
	uint32_t bloque = 0;

	lock(&bitacora->mutex);

	uint32_t fragmentacionArchivo = fragmentacion(bitacora->tamanioArchivo);
	//log_info(logImongo, "La bitacora del tripulante %s tiene una fragmentacion de %d", idTripulante, fragmentacionArchivo);
/*
	log_info(logImongo, "Se va a escribir el mensaje '%s' de %d bytes en "
			"la bitacora del tripulante %s", mensaje, tamanioMensaje, idTripulante);
*/
	lock(&mutexBitMap);
	t_list* bloquesAocupar = buscarBloques(max(0, tamanioMensaje - fragmentacionArchivo));
	actualizarSuperBloque();
	unlock(&mutexBitMap);

	if(!list_is_empty(bloquesAocupar) || tamanioMensaje <= fragmentacionArchivo){

		bitacora->tamanioArchivo += tamanioMensaje;

		if(fragmentacionArchivo > 0){

			bloque = * (uint32_t*) list_get(bitacora->bloques, list_size(bitacora->bloques) - 1);
			char* contenidoUltimoBloque = string_substring_until(mensaje, min(fragmentacionArchivo, tamanioMensaje));
			escribirBloque(bloque, contenidoUltimoBloque, superBloque->block_size - fragmentacionArchivo);
			offsetMensaje = min(fragmentacionArchivo, tamanioMensaje);
			free(contenidoUltimoBloque);

		}

		while(!list_is_empty(bloquesAocupar)){

			char* fragmentoAescribir = string_substring(mensaje,
					offsetMensaje, min(tamanioMensaje, superBloque->block_size));
			list_add(bitacora->bloques, list_remove(bloquesAocupar, 0));
			bloque = * (uint32_t*) list_get(bitacora->bloques, list_size(bitacora->bloques) - 1);
			escribirBloque(bloque, fragmentoAescribir, 0);
			//ocuparBloque(bloque);
			offsetMensaje += superBloque->block_size;
			free(fragmentoAescribir);
		}

		actualizarBitacora(bitacora);
	}
	else{

		log_error(logImongo,"--- NO HAY BLOQUES DISPONIBLES PARA LA BITACORA ---");

	}

	unlock(&bitacora->mutex);

	list_destroy(bloquesAocupar);

}


uint32_t fragmentacion(uint32_t tamanioArchivo){

	uint32_t bytesSobrantes = tamanioArchivo % superBloque->block_size;
	if(bytesSobrantes > 0)
		return superBloque->block_size - bytesSobrantes;
	else{
		return 0;
	}
}


void escribirBloque(uint32_t bloque, char* contenido, uint32_t tamanioBloque){

	uint32_t posicionEnBites = bloque * superBloque->block_size + tamanioBloque;

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites , contenido, strlen(contenido));
	unlock(&mutexMemoriaSecundaria);

	/*
	log_info(logImongo, "Escribio '%s' en el bloque %d a partir de la posicion %d",
			contenido, bloque, tamanioBloque);
	*/
}


void consumirBloque(uint32_t bloque, uint32_t cantidadConsumir, uint32_t tamanioBloque){

	uint32_t cantBytesFinalesEnBloque = tamanioBloque - cantidadConsumir;
	uint32_t posicionEnBites = bloque * superBloque->block_size + cantBytesFinalesEnBloque;

	char* consumirCantidad = string_repeat('\0', cantidadConsumir);

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites ,consumirCantidad , cantidadConsumir);
	unlock(&mutexMemoriaSecundaria);

	free(consumirCantidad);

	log_info(logImongo, "Se consumio %d del bloque %d y quedo %d bytes en el bloque",
			cantidadConsumir, bloque, cantBytesFinalesEnBloque);
}


t_list* buscarBloques(uint32_t cantBytes){

	int cantBloques = (int) ceil((float) cantBytes / (float) superBloque->block_size);

	log_info(logImongo, "La cant de bloques a ocupar es %d", cantBloques);

	t_list* bloquesAocupar = list_create();

	for(int i=0; i<superBloque->blocks && list_size(bloquesAocupar) < cantBloques; i++){

		if(bitarray_test_bit(superBloque->bitmap,i) == 0){
			uint32_t* bloque = malloc(sizeof(uint32_t));
			*bloque = i;
			list_add(bloquesAocupar, bloque);
			ocuparBloque(*bloque);
			log_info(logImongo, "Se va a ocupar el bloque %d", i);
		}
	}

	if(list_size(bloquesAocupar) < cantBloques){ // CASO EN EL Q NO ALCANZARON LOS BLOQUES PARA ESCRIBIR TODA LA TAREA

		void eliminarEntero(uint32_t* n){
			free(n);
		}

		list_clean_and_destroy_elements(bloquesAocupar, (void*) eliminarEntero);
	}

	return bloquesAocupar;
}


void ocuparBloque(uint32_t bloque){

	bitarray_set_bit(superBloque->bitmap, bloque);
	//log_info(logImongo, "El bloque %d ahora esta ocupado", bloque);
}


void liberarBloque(uint32_t bloque){

	bitarray_clean_bit(superBloque->bitmap, bloque);
	log_info(logImongo, "El bloque %d ahora esta libre", bloque);
}


void actualizarFile(t_file* archivo){

	t_config* configFile = config_create(archivo->path);

	char* stringBloques = convertirListaEnString(archivo->bloques);
	char* stringCantBloques = string_itoa(list_size(archivo->bloques));
	char* stringSize = string_itoa(archivo->tamanioArchivo);
	char* md5 = obtenerMD5(archivo->bloques);

	config_set_value(configFile, "BLOCKS", stringBloques);
	config_set_value(configFile, "SIZE", stringSize);
	config_set_value(configFile, "MD5_ARCHIVO", md5);
	config_set_value(configFile, "BLOCK_COUNT", stringCantBloques);
	config_set_value(configFile, "CARACTER_LLENADO", archivo->caracterLlenado);

	free(md5);
	free(stringBloques);
	free(stringCantBloques);
	free(stringSize);

	config_save(configFile);
	config_destroy(configFile);

}


void actualizarBitacora(t_bitacora_tripulante* bitacora){

	t_config* configBitacora = config_create(bitacora->path);

	char* stringBloques = convertirListaEnString(bitacora->bloques);
	char* stringSize = string_itoa(bitacora->tamanioArchivo);

	config_set_value(configBitacora, "BLOCKS", stringBloques);
	config_set_value(configBitacora, "SIZE", stringSize);

	free(stringBloques);
	free(stringSize);

	config_save(configBitacora);
	config_destroy(configBitacora);
}


void actualizarSuperBloque(){

	lock(&mutexSP);

	char* stringBitmap = convertirBitmapEnString(superBloque->bitmap);

	t_config* configSB;

	crearConfig(&configSB,superBloque->path);

	config_set_value(configSB, "BITMAP", stringBitmap);

	free(stringBitmap);

	config_save(configSB);
	config_destroy(configSB);

	unlock(&mutexSP);
}


char* convertirBitmapEnString(t_bitarray* bitmap){

	char* stringBitmap = string_new();

	for(int i=0; i< bitarray_get_max_bit(bitmap); i++){

		if(bitarray_test_bit(bitmap, i))
			string_append(&stringBitmap, "1");
		else
			string_append(&stringBitmap, "0");
	}

	return stringBitmap;
}
