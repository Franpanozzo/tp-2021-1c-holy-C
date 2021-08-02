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
		if(flag ==0)
			log_info(logImongo, "Se sincronizo la memoria con éxito");
		else if(flag == -1)
			log_error(logImongo, "No se sincronizo bien la memoria");
		else
			log_error(logImongo, "La sincronizacion esta arrojando cualquier valor");
	}
}

/*
void crearConfigTarea(tarea* structTarea){

	if(structTarea->configSeCreo != true){
		structTarea->config  = config_create(structTarea->path);

		if(structTarea->config == NULL){

			log_error(logImongo, "La ruta es incorrecta ");

			exit(1);
		}
	}
}
*/


char * reconstruirArchivo(t_list * bloques){
	int cantidadBloques = list_size(bloques);
	int tamanio = cantidadBloques*superBloque->block_size;

	char * copiaFile = malloc(tamanio+1);

	for(int i=0; i<cantidadBloques;i++){
		int * bloque = list_get(bloques, i);
		int offsetMemoria =  (*bloque) * superBloque->block_size;
		int offsetFile = i * superBloque->block_size;
		log_info(logImongo,"offset memoria %d",offsetMemoria);
		log_info(logImongo,"offset file %d",offsetFile);
		lock(&mutexMemoriaSecundaria);
		memcpy(copiaFile+offsetFile,copiaMemoriaSecundaria + offsetMemoria ,min(superBloque->block_size, tamanio - offsetFile));
		unlock(&mutexMemoriaSecundaria);
	}
	copiaFile[tamanio] = '\0';
	log_info(logImongo,"archivo reconstruido es: %s",copiaFile);
	return copiaFile;
}


char * obtenerMD5(t_list * bloques){
	int md5Size = 32;

	char* copiaFile = reconstruirArchivo(bloques);
	log_info(logImongo,"ACA 1");
	char * comando = string_from_format("echo -n \"%s\" | md5sum", copiaFile);
	char * md5 = malloc(md5Size +2);
	FILE * file = popen(comando, "r");

	log_info(logImongo,"ACA 2");
	fgets(md5, md5Size+1, file);
	log_info(logImongo,"ACA 3");
	/*
	fclose(file);
	free(comando);
	free(copiaFile);*/
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

	return avisoTarea;
}


int deserializarAvisoSabotaje(void* stream){

	int *id = malloc(sizeof(uint32_t));
	memcpy(id, stream, sizeof(uint32_t));

	return *id;
}


/*
char* leerBitacora(int idTripulante){

	char* bitacora;

	char* path = string_from_format("%s/Tripulante%d.ims", pathBitacoras,idTripulante);
	t_config* config;

	if(verificarSiExiste(path)){

		bitacora = string_new();
		config = config_create(path);
		char** arrayBloques = config_get_array_value(config,"BLOCKS");
		t_list* listaBloques = convertirEnLista(arrayBloques);
		char* contenidoBloque;
		char* contenidoBloqueSinFrag;

		for(int i=0; i<list_size(listaBloques); i++){

			contenidoBloque = datosBloque(* (int*)list_get(listaBloques, i));
	        log_info(logImongo,"El contenido de la biacora q se va a leer es: %s", contenidoBloque);
			contenidoBloqueSinFrag = string_substring(contenidoBloque, 0, strlen(contenidoBloque));
			string_append(&bitacora, contenidoBloqueSinFrag);
		}
	}
	else{
		bitacora = string_from_format("No existe la bitacora del tripulante %d", idTripulante);
	}

	return bitacora;
}
*/


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


void liberarConfiguracion(){
	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);
}


t_list* listaCoordenadasSabotaje() {

    char* stringCoordenadas = datosConfig->posicionesSabotaje;
    stringCoordenadas = string_substring(stringCoordenadas, 1, strlen(stringCoordenadas)-2 ); //Le saco los corchetes
    //Aca puede ir un log info para chequear que se haya hecho
    char** arrayStringsCoordenadas = string_split(stringCoordenadas, ",");

    t_list* listaCoordenadas = list_create();
    int i = 0;
    while(arrayStringsCoordenadas[i] != NULL) {

        char** parCoordenadas = string_split(stringCoordenadas, "|");

        t_coordenadas* coordenadasSabotaje = malloc(sizeof(t_coordenadas));

        coordenadasSabotaje->posX = (uint32_t) atoi(parCoordenadas[0]);
        coordenadasSabotaje->posY = (uint32_t) atoi(parCoordenadas[1]);

        list_add(listaCoordenadas, coordenadasSabotaje);

        liberarDoblesPunterosAChar(parCoordenadas);
        i++;
    }

    liberarDoblesPunterosAChar(arrayStringsCoordenadas);
    free(stringCoordenadas);

    return listaCoordenadas;
}


void sabotaje(int signal) {

	/*
  t_coordenadas* coordenadasSabotaje = list_get(listaPosicionesSabotaje, proximoPosSabotaje);

  proximoPosSabotaje++;
  if(proximoPosSabotaje == list_size(listaPosicionesSabotaje)) proximoPosSabotaje = 0;

  t_paquete* paqueteEnviado = armarPaqueteCon((void*) coordenadasSabotaje, COORDENADAS_SABOTAJE);

  log_info(logImongo,"\nSABOTAJE - AVISANDO A DISCORDADOR QUE ES EN POS: X:%d - Y:%d\n", coordenadasSabotaje->posX, coordenadasSabotaje->posY);

  int socketDisc = iniciarConexionDesdeClienteHacia((void*) puertoEIPDisc);

  enviarPaquete(paqueteEnviado, socketDisc);
  close(socketDisc);
*/
  buscarSabotaje();
}


void generarTarea2(t_file2 archivo, uint32_t cantidad){

	lock(archivo->mutex);

	uint32_t fragmentacionArchivo = fragmentacion(archivo->tamanioArchivo);
	uint32_t bloque = 0;

	lock(&mutexBitMap);
	t_list* bloquesAocupar = buscarBloques2(max(0, cantidad - fragmentacionArchivo));

	if(!list_is_empty(bloquesAocupar)){

		archivo->tamanioArchivo += cantidad;

		if(fragmentacionArchivo > 0){

			bloque = * (int*) list_get(archivo->bloques, list_size(archivo->bloques) - 1);
			escribirBloqueEmpezado(bloque, string_repeat(*archivo->caracterLlenado, fragmentacionArchivo));
			cantidad = max(0, cantidad - fragmentacionArchivo);
		}

		t_list* bloquesAocupar = buscarBloques2(cantidad); //si la cantdad es cero, devuelve una lista vacia

		while(!list_is_empty(bloquesAocupar)){

			list_add(archivo->bloques, list_remove(bloquesAocupar, 0));
			bloque = * (int*) list_get(archivo->bloques, list_size(archivo->bloques) - 1);
			escribirBloque(bloque, string_repeat(*archivo->caracterLlenado, min(superBloque->block_size, cantidad)));
			ocuparBloque(bloque);
			cantidad -= superBloque->block_size;
		}

		actualizarSuperBloque();
		unlock(&mutexBitMap);

		archivo->md5_archivo = obtenerMD5(archivo->bloques);

		actualizarFile(archivo);
	}
	else{
		log_error(logImongo,"--- NO HAY BLOQUES DISPONIBLES PARA LA TAREA ---");
	}

	unlock(archivo->mutex);
}


void consumirTarea2(t_file2 archivo, uint32_t cantidad){

	lock(archivo->mutex);

	cantidad = min(cantidad, archivo->tamanioArchivo);
	uint32_t fragmentacionArchivo = fragmentacion(archivo);
	uint32_t* bloque;

	archivo->tamanioArchivo -= cantidad;

	unlock(&mutexBitMap);

	if(fragmentacionArchivo > 0){

		bloque = (int*) list_remove(archivo->bloques, list_size(archivo->bloques) - 1);
		consumirBloqueEmpezado(*bloque, superBloque->block_size - fragmentacionArchivo);
		liberarBloque(*bloque);
		cantidad = max(0, cantidad - superBloque->block_size + fragmentacionArchivo);
	}

	while(cantidad > 0){

		bloque =(int*) list_remove(archivo->bloques, list_size(archivo->bloques) - 1);
		consumirBloque(bloque, min(cantidad, superBloque->block_size));
		cantidad -= superBloque->block_size;
		if(cantidad > 0){
			liberarBloque(*bloque);
		}
		//free(bloque);
	}

	actualizarSuperBloque();
	unlock(&mutexBitMap);

	archivo->md5_archivo = obtenerMD5(archivo->bloques);

	actualizarFile(archivo);

	unlock(archivo->mutex);
}


// LA CREACION DEL ARCHIVO LA HAGO APARTE
void escribirBitacora2(char* idTripulante, char* mensaje){

	t_bitacora_tripulante* bitacora = dictionary_get(bitacoras, idTripulante);

	lock(&bitacora->mutex);

	uint32_t fragmentacionArchivo = fragmentacion(bitacora->tamanioArchivo);
	int tamanioMensaje = strlen(mensaje);
	uint32_t bloque = 0;

	lock(&mutexBitMap);
	t_list* bloquesAocupar = buscarBloques(tamanioMensaje); //si la cantdad es cero, devuelve una lista vacia

	if(!list_is_empty(bloquesAocupar)){

		bitacora->tamanioArchivo += tamanioMensaje;

		if(fragmentacionArchivo > 0){

			bloque = * (int*) list_get(bitacora->bloques, list_size(bitacora->bloques) - 1);
			char* contenidoUltimoBloque = string_substring_until(mensaje, fragmentacionArchivo);
			escribirBloqueEmpezado(bloque, contenidoUltimoBloque);
			ocuparBloque(bloque);
			tamanioMensaje = max(0, tamanioMensaje - fragmentacionArchivo);
		}

		while(!list_is_empty(bloquesAocupar)){

			char* fragmentoAescribir = string_substring_until(mensaje, min(tamanioMensaje, superBloque->block_size));
			list_add(bitacora->bloques, list_remove(bloquesAocupar, 0));
			bloque = * (int*) list_get(bitacora->bloques, list_size(bitacora->bloques) - 1);
			escribirBloque(bloque, fragmentoAescribir);
			ocuparBloque(bloque);
			tamanioMensaje -= superBloque->block_size;
			free(fragmentoAescribir);
		}

		actualizarSuperBloque();
		unlock(&mutexBitMap);

		actualizarBitacora(bitacora);
	}
	else{
		log_error(logImongo,"--- NO HAY BLOQUES DISPONIBLES PARA LA BITACORA ---");
	}

	unlock(&bitacora->mutex);
}


uint32_t fragmentacion(uint32_t tamanioArchivo){

	return superBloque->block_size - (tamanioArchivo % superBloque->block_size);
}


void escribirBloque(uint32_t bloque, char* contenido){

	char* dupContenido = string_duplicate(contenido);
	string_append(&dupContenido, "\0");

	uint32_t posicionEnBites = bloque * superBloque->block_size;

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites , contenido, strlen(dupContenido));
	unlock(&mutexMemoriaSecundaria);

	log_info(logImongo, "Escribio en memoria %s en el bloque %d", dupContenido, posicionEnBites);
}


void escribirBloqueEmpezado(uint32_t bloque, char* contenido){

	char* dupContenido = string_duplicate(contenido);
	string_append(&dupContenido, "\0");

	uint32_t posicionEnBites = bloque * superBloque->block_size + superBloque->block_size - strlen(dupContenido);

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites , contenido, strlen(dupContenido));
	unlock(&mutexMemoriaSecundaria);

	log_info(logImongo, "Escribio en memoria %s en el bloque %d", dupContenido, posicionEnBites);
}


void consumirBloqueEmpezado(uint32_t bloque, uint32_t cantidadConsumir){

	uint32_t posicionEnBites = bloque * superBloque->block_size;

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites , string_repeat('\0', cantidadConsumir), cantidadConsumir);
	unlock(&mutexMemoriaSecundaria);
}


void consumirBloque(uint32_t bloque, uint32_t cantidadConsumir){

	uint32_t posicionEnBites = bloque * superBloque->block_size + superBloque->block_size - cantidadConsumir;

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionEnBites , string_repeat('\0', cantidadConsumir), cantidadConsumir);
	unlock(&mutexMemoriaSecundaria);
}


t_list* buscarBloques2(uint32_t cantBytes){

	int cantBloques = (int) ceil((float) cantBytes / (float) superBloque->block_size);

	t_list* bloquesAocupar = list_create();

	for(int i=0; i<superBloque->blocks && list_size(bloquesAocupar) < cantBloques; i++){

		if(bitarray_test_bit(superBloque->bitmap,i) == 0){
			uint32_t* bloque = malloc(sizeof(uint32_t));
			*bloque = i;
			list_add(bloquesAocupar, bloque);
		}
	}

	if(list_size(bloquesAocupar) < cantBloques){

		void eliminarEntero(int* n){
			free(n);
		}

		list_clean_and_destroy_elements(bloquesAocupar, (void*) eliminarEntero);
	}

	return bloquesAocupar;
}


void ocuparBloque(uint32_t bloque){

	bitarray_set_bit(superBloque->bitmap, bloque);
}


void liberarBloque(uint32_t bloque){

	bitarray_clean_bit(superBloque->bitmap, bloque);
}


void actualizarFile(t_file2 archivo){

	t_config* configFile = config_create();

	char* stringBloques = convertirListaEnString(archivo->bloques);
	char* stringCantBloques = string_itoa(list_size(archivo->bloques));
	char* stringSize = string_itoa(archivo->tamanioArchivo);

	config_set_value(configFile, stringBloques, "BLOCKS");
	config_set_value(configFile, stringSize, "SIZE");
	config_set_value(configFile, archivo->md5_archivo, "MD5_ARCHIVO");
	config_set_value(configFile, stringCantBloques, "BLOCK_COUNT");

	config_save(configFile);
	config_destroy(configFile);
}


void actualizarBitacora(t_bitacora_tripulante* bitacora){

	t_config* configBitacora = config_create();

	char* stringBloques = convertirListaEnString(bitacora->bloques);
	char* stringSize = string_itoa(bitacora->tamanioArchivo);

	config_set_value(configBitacora, stringBloques, "BLOCKS");
	config_set_value(configBitacora, stringSize, "SIZE");

	config_save(configBitacora);
	config_destroy(configBitacora);
}


void actualizarSuperBloque(){

	char* stringBitmap = convertirBitmapEnString(superBloque->bitmap);

	t_config* configSB = config_create(superBloque->path);

	config_set_value(configSB, "BITMAP", stringBitmap);
	free(stringBitmap);

	config_save(configSB);
	config_destroy(configSB);
}
