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


char* crearDestinoApartirDeRaiz(char* destino){

	char* raiz = string_new();

	string_append(&raiz, datosConfig->puntoMontaje);
	string_append(&raiz, "/");
	string_append(&raiz, destino);

	return raiz;
}


void cargarPaths(){

	pathSuperBloque = crearDestinoApartirDeRaiz("SuperBloque.ims");
	pathBloque = crearDestinoApartirDeRaiz("Blocks.ims");
	pathFiles = crearDestinoApartirDeRaiz("Files");
	oxigeno->path = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	comida->path = crearDestinoApartirDeRaiz("Files/Comida.ims");
	basura->path = crearDestinoApartirDeRaiz("Files/Basura.ims");
	pathBitacora = crearDestinoApartirDeRaiz("Files/Bitacora");
}


void cargarDatosConfig(){

	datosConfig = malloc(sizeof(t_datosConfig));
	datosConfig->puntoMontaje = config_get_string_value(configImongo,"PUNTO_MONTAJE");
	datosConfig->puerto = (uint32_t)config_get_int_value(configImongo,"PUERTO");
	datosConfig->tiempoSincronizacion = (uint32_t)config_get_int_value(configImongo,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = config_get_string_value(configImongo,"POSICIONES_SABOTAJE");
}


void mallocTareas(){

	oxigeno = malloc(sizeof(tarea));
	oxigeno->configSeCreo = 0;
	oxigeno->mutex = malloc(sizeof(pthread_mutex_t));
	oxigeno->file= malloc(sizeof(t_file));
	oxigeno->file->caracterLlenado = "O";

	pthread_mutex_init(oxigeno->mutex, NULL);

	comida = malloc(sizeof(tarea));
	comida->configSeCreo = 0;
	comida->mutex = malloc(sizeof(pthread_mutex_t));
	comida->file= malloc(sizeof(t_file));
	comida->file->caracterLlenado = "C";

	pthread_mutex_init(comida->mutex, NULL);

	basura = malloc(sizeof(tarea));
	basura->configSeCreo = 0;
	basura->mutex = malloc(sizeof(pthread_mutex_t));
	basura->file= malloc(sizeof(t_file));
	basura->file->caracterLlenado = "B";

	pthread_mutex_init(basura->mutex, NULL);
}


void iniciarMutex(){

	pthread_mutex_init(&mutexSuperBloque,NULL);
	pthread_mutex_init(&mutexMemoriaSecundaria,NULL);
	pthread_mutex_init(&mutexBitMap,NULL);
	pthread_mutex_init(&mutexEstructurasFile,NULL);
	pthread_mutex_init(&mutexOxigeno,NULL);
	pthread_mutex_init(&mutexComida,NULL);
	pthread_mutex_init(&mutexBasura,NULL);
	pthread_mutex_init(&mutexEstructuraOxigeno,NULL);
	pthread_mutex_init(&mutexEstructuraComida,NULL);
	pthread_mutex_init(&mutexEstructuraBasura,NULL);
	pthread_mutex_init(&mutexBitacora,NULL);
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

	printf("Tamanio total del archivo: %lu bytes\n", sb.st_size);
	printf("Ultimo estado:       %s", ctime(&sb.st_ctime));
	printf("Ultimo acceso:         %s", ctime(&sb.st_atime));
	printf("Ultima modificacion del archivo:   %s", ctime(&sb.st_mtime));
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


void mandarErrorAdiscordiador(int* tripulanteSock){

	char* error = strdup("ERROR");

	t_paquete* paquete = armarPaqueteCon(error,STRING);

	enviarPaquete(paquete,*tripulanteSock);

	free(error);
}


void mandarOKAdiscordiador(int* tripulanteSock){

	char* confirmacion = strdup("OK");

	t_paquete* paquete = armarPaqueteCon(confirmacion,STRING);

	enviarPaquete(paquete,*tripulanteSock);

	free(confirmacion);
}


bool bloquesLibres(int bloquesAocupar){

	int flag = 0;

	int cantidadDeBloquesLibres = 0;

	for(int i=0; i<superBloque->blocks;i++){

		lock(&mutexBitMap);
		flag = bitarray_test_bit(superBloque->bitmap,i);
		unlock(&mutexBitMap);

		if(flag == 0){
			cantidadDeBloquesLibres++;
		}
	}

	log_info(logImongo, "La cantidad de bloques libres es: %d", cantidadDeBloquesLibres);

	return bloquesAocupar <= cantidadDeBloquesLibres ;
}


bool verificarSiExiste(char* pathArchivo){

	return access(pathArchivo,F_OK) != -1;
}


int* obtenerArrayDePosiciones(int bloquesAocupar){

	int flag = 0;

	int bloquesLibres = 0;

	int* cadaBloqueAocupar = malloc(sizeof(int) * bloquesAocupar);

	for(int i=0; i<superBloque->blocks;i++){

		lock(&mutexBitMap);
		flag = bitarray_test_bit(superBloque->bitmap,i);
		unlock(&mutexBitMap);

		if(flag == 0){
			*(cadaBloqueAocupar + bloquesLibres) = i;
			bloquesLibres++;
		}

		if(bloquesLibres == bloquesAocupar){
			break;
		}
	}

	return cadaBloqueAocupar;
}


void actualizarStringBitMap(){

	int sizeBitArray = superBloque->blocks;

	char* bitmap = malloc(sizeBitArray + 1);

	for(int i=0; i<sizeBitArray;i++){

	  	lock(&mutexBitMap);
    	int valor =  bitarray_test_bit(superBloque->bitmap, i);
    	unlock(&mutexBitMap);

    	bitmap[i] = valor + '0';
   }

/*
  for(int i = sizeBitArray; i<bitarray_get_max_bit(superBloque->bitmap);i++){

	  int valor =  bitarray_test_bit(superBloque->bitmap, i);

	      	printf("%d ", valor);

  }
 PARA ANALIZAR LOS BLOQUES MUERTOS SI NO SON MULTIPLOS DE 8
*/

  bitmap[sizeBitArray] = '\0';

  //lock(&mutexSuperBloque);
  config_set_value(configSuperBloque,"BITMAP",bitmap);

  config_save(configSuperBloque);
  //unlock(&mutexSuperBloque);
  free(bitmap);
}


void actualizarEstructurasFile(tarea* structTarea){

	if(structTarea->configSeCreo != true){
		structTarea->configSeCreo = true;

		if(strcmp(config_get_string_value(structTarea->config,"BLOCKS"),"") == 0){
			structTarea->file->bloques = list_create();
		}
		else{
			char ** stringAsArray = config_get_array_value(structTarea->config,"BLOCKS");
			structTarea->file->bloques = convertirEnLista(stringAsArray);
			for(int i=0;i<list_size(structTarea->file->bloques);i++)
				free(* (stringAsArray+i));
			free(stringAsArray);
		}

		structTarea->file->cantidadBloques = config_get_int_value(structTarea->config,"BLOCK_COUNT");
		structTarea->file->md5_archivo = config_get_string_value(structTarea->config,"MD5_ARCHIVO");
		structTarea->file->tamanioArchivo = config_get_int_value(structTarea->config,"SIZE");
	}
}


void actualizarPosicionesFile(tarea* structTarea, int* arrayDePosiciones, int bloquesAocupar){

	for(int i = 0; i<bloquesAocupar; i++){
		int * bloque = (arrayDePosiciones+i);
		list_add(structTarea->file->bloques,bloque);
	}
	char* bloques = convertirEnString(structTarea->file->bloques);

	structTarea->file->cantidadBloques += bloquesAocupar;

	log_info(logImongo,"Se procede a actualizar las estructuras y archivos con los valores nuevos: ----%s--------%d----",bloques,structTarea->file->cantidadBloques);

	config_set_value(structTarea->config,"BLOCKS",bloques);
	char * cantidadBloques = string_itoa(structTarea->file->cantidadBloques);
	config_set_value(structTarea->config,"BLOCK_COUNT",cantidadBloques);
	config_save(structTarea->config);
	free(cantidadBloques);
	free(bloques);

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


void actualizarBitArray(int* posicionesQueOcupa, int bloquesAocupar){

	for(int i=0; i<bloquesAocupar; i++){

		lock(&mutexBitMap);
		bitarray_set_bit(superBloque->bitmap, posicionesQueOcupa[i]);
		unlock(&mutexBitMap);
	}

	actualizarStringBitMap(bloquesAocupar);
}


void guardarEnMemoriaSecundaria(int* posicionesQueOcupa,char* caracterLlenado, int bloquesAocupar, int caracteresAguardar){

	int flag = caracteresAguardar;

	log_info(logImongo,"flag = %d", flag);

	char caracter = caracterLlenado[0];

	log_info(logImongo,"caracter = %c", caracter);

	char** palabraAguardar = malloc(sizeof(char*) * bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){

		palabraAguardar[i] = string_repeat(caracter,min(superBloque->block_size,flag));
		log_info(logImongo,"Lo que pesa cada char es %d", strlen(palabraAguardar[i]));
		log_info(logImongo,"palabra = %s", palabraAguardar[i]);
		flag -= superBloque->block_size;
		log_info(logImongo,"flag = %d", flag);
	}

	for(int i=0; i<bloquesAocupar; i++){

		int offset = posicionesQueOcupa[i] * superBloque->block_size;

		log_info(logImongo,"offset = %d", offset);

		lock(&mutexMemoriaSecundaria);

		memcpy(copiaMemoriaSecundaria + offset ,palabraAguardar[i], strlen(palabraAguardar[i]));
		unlock(&mutexMemoriaSecundaria);

		log_info(logImongo,"Lo que termino guardando fue %d", superBloque->block_size);
	}

	actualizarBitArray(posicionesQueOcupa, bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){
		free(palabraAguardar[i]);
	}

	free(palabraAguardar);
}


char* datosBloque(int numeroBloque){

	int offset = (numeroBloque * superBloque->block_size);

	log_info(logImongo,"El corrimiento para guardar es: %d", offset);

	char* bloque = malloc(superBloque->block_size + 1);
	lock(&mutexMemoriaSecundaria);
	memcpy(bloque,copiaMemoriaSecundaria + offset,(int)superBloque->block_size);
	unlock(&mutexMemoriaSecundaria);

	bloque[superBloque->block_size] = '\0';

	return bloque;
}


int fragmentacionDe(int ultimoBloqueTarea){

	char* bloque = datosBloque(ultimoBloqueTarea);

	int tamanioBloque = string_length(bloque);

	log_info(logImongo,"El tamanio del bloque es: %d", tamanioBloque);

	int fragmentacion = ((int) superBloque->block_size) - tamanioBloque;

	log_info(logImongo,"La fragmentacion del bloque es: %d", fragmentacion);

	free(bloque);

	return fragmentacion;
}


int ultimoBloqueDeLa(tarea* structTarea){

	int indiceUltimoBloque = list_size( structTarea->file->bloques) - 1;

	int * ultimoBloque = list_get( structTarea->file->bloques,indiceUltimoBloque);

	return *ultimoBloque;
}


t_info*  nuevosBloquesAocupar(tarea* structTarea,t_tarea* _tarea){

	t_info* capacidad = malloc(sizeof(t_info));

	int ultimoBloqueDeLaTarea = ultimoBloqueDeLa(structTarea);

	int fragmentacionInterna = fragmentacionDe(ultimoBloqueDeLaTarea);

	char* bloque = malloc(superBloque->block_size + 1);

	if(_tarea->parametro > fragmentacionInterna){

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d, "
				"se procede a devolver el ultimo bloque entero y mas los bloques a ocupar",
				_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		for(int i=0; i<superBloque->block_size;i++){

			bloque[i] = structTarea->file->caracterLlenado[0];

		}

		bloque[superBloque->block_size] = '\0';

		log_info(logImongo,"El bloque que se va a guardar es: %s", bloque);

		lock(&mutexMemoriaSecundaria);
		memcpy(copiaMemoriaSecundaria + (ultimoBloqueDeLaTarea * superBloque->block_size),
				bloque,superBloque->block_size);
		unlock(&mutexMemoriaSecundaria);

		free(bloque);

		capacidad->bloquesNuevosAocupar  = (int) ceil((float)
				(_tarea->parametro - fragmentacionInterna) / (float) superBloque->block_size);

		capacidad->caracteresAguardar = _tarea->parametro - fragmentacionInterna;

		return capacidad;

	}
	else if(_tarea->parametro == fragmentacionInterna){

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d, "
				"se procede a devolver el ultimo bloque entero, pero no hay mas bloques que ocupar",
				_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		log_info(logImongo,"El bloque que se va a guardar es: ");

		for(int i=0; i<superBloque->block_size;i++){

			bloque[i] = structTarea->file->caracterLlenado[0];
		}

		bloque[superBloque->block_size] = '\0';

		log_info(logImongo,"El bloque que se va a guardar es: %s", bloque);
	}
	else{

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d,"
				"se procede a devolver el ultimo bloque incompleto, y no hay mas bloques que ocupar",
				_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		for(int i=0; i<((superBloque->block_size - fragmentacionInterna) + _tarea->parametro);i++){

			bloque[i] = structTarea->file->caracterLlenado[0];
		}

		bloque[superBloque->block_size] = '\0';

		for(int i = 0; bloque[i] != '\0'; i++){

			if(bloque[i] != structTarea->file->caracterLlenado[0]){

				bloque[i] = '\0';
			}
		}

		log_info(logImongo,"El bloque que se va a guardar es: %s", bloque);

	}
	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + (ultimoBloqueDeLaTarea * superBloque->block_size),bloque,superBloque->block_size);
	unlock(&mutexMemoriaSecundaria);

	free(bloque);
	capacidad->bloquesNuevosAocupar  = 0;

	capacidad->caracteresAguardar = 0;

	return capacidad;
}


void crearConfigTarea(tarea* structTarea){

	if(structTarea->configSeCreo != true){
		structTarea->config  = config_create(structTarea->path);

		if(structTarea->config == NULL){

			log_error(logImongo, "La ruta es incorrecta ");

			exit(1);
		}
	}
}


void inicializarTarea(tarea* structTarea, int bloquesAocupar, int caracteresAOcupar){
	config_set_value(structTarea->config,"CARACTER_LLENADO",structTarea->file->caracterLlenado);
	config_set_value(structTarea->config,"BLOCKS","");
	config_set_value(structTarea->config,"MD5_ARCHIVO","");
	char* cantidadBloques =  string_itoa(bloquesAocupar);
	config_set_value(structTarea->config,"BLOCK_COUNT","0");
	free(cantidadBloques);
	char* tamanioArchivo = string_itoa(caracteresAOcupar);
	config_set_value(structTarea->config,"SIZE",tamanioArchivo);
	free(tamanioArchivo);
	config_save(structTarea->config);
}

void generarTarea(tarea* structTarea, t_tarea* _tarea){

	int caracteresAOcupar = _tarea->parametro;
	int bloquesAocupar = (int) ceil((float) caracteresAOcupar / (float) superBloque->block_size);//TENER EN CUENTA QUE ESTO TESTEA SIN TENER EN CUENTA LA FRAGMENTACION SI HAY BLOQUES LIBRES OJO!!!!

	log_info(logImongo, "Los bloques a ocupar de la tarea: %s son: %d ",_tarea->nombreTarea, bloquesAocupar);
	lock(structTarea->mutex);
	if(bloquesLibres(bloquesAocupar)){ //SI UNA TAREA TIENE ESPACIO EN EL DISCO

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",_tarea->nombreTarea);

		if(verificarSiExiste(structTarea->path) && structTarea->file->cantidadBloques > 0){ // SI NO ES LA PRIMERA VEZ QUE VIENE ESA TAREA

			crearConfigTarea(structTarea);

			actualizarEstructurasFile(structTarea);

			char* mostrarBloquesLog = convertirEnString(structTarea->file->bloques);

			log_info(logImongo,"Los bloques que ocupaba antes son: %s que serian %d bloques",
					mostrarBloquesLog, structTarea->file->cantidadBloques);
			free(mostrarBloquesLog);
			structTarea->file->tamanioArchivo += caracteresAOcupar;
			char* tamanioArchivo = string_itoa(structTarea->file->tamanioArchivo);
			config_set_value(structTarea->config,"SIZE",tamanioArchivo);
			config_save(structTarea->config);

			t_info* capacidad;

			capacidad = nuevosBloquesAocupar(structTarea,_tarea);
			actualizarMD5(structTarea);
			bloquesAocupar = capacidad->bloquesNuevosAocupar;

			if(bloquesAocupar > 0){

				log_info(logImongo,"La cantidad de bloques que ocupa ahora son: %d", bloquesAocupar);

				int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

				log_info(logImongo, "Las posiciones que va a ocupar en disco rigido la tarea %s,"
						"de %d bytes, DESPUES del filtro de la fragmentacion interna son: ",
						_tarea->nombreTarea, _tarea->parametro);

				for(int i=0; i<bloquesAocupar;i++){

					log_info(logImongo, "%d, " , posicionesQueOcupa[i]);
				}

				actualizarPosicionesFile(structTarea,posicionesQueOcupa,bloquesAocupar);

				log_info(logImongo,"Se porcede a guardar en memoria secundaria %d bloques", bloquesAocupar);
				guardarEnMemoriaSecundaria(posicionesQueOcupa,structTarea->file->caracterLlenado,bloquesAocupar,capacidad->caracteresAguardar);

				actualizarMD5(structTarea);
				free(posicionesQueOcupa);
			}

			free(tamanioArchivo);
			free(capacidad);

		}
		else{ //SI ES LA PRIMERA VEZ QUE VIENE ESA TAREA

			FILE* archivoOxigeno = fopen(structTarea->path,"wb");
			fclose(archivoOxigeno);

			crearConfigTarea(structTarea);

			inicializarTarea(structTarea, bloquesAocupar, caracteresAOcupar);

			actualizarEstructurasFile(structTarea);

			log_info(logImongo,"La tarea %s es la primera vez que ingresa,"
					"los bloques que va a ocupar son: %d", _tarea->nombreTarea,bloquesAocupar);

			int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

			log_info(logImongo, "Las posiciones que va a ocupar en disco rigido la tarea %s,"
					"de %d bytes son: ",_tarea->nombreTarea, _tarea->parametro);

			for(int i=0; i<bloquesAocupar;i++){

				log_info(logImongo, "%d, ",posicionesQueOcupa[i]);
			}

			actualizarPosicionesFile(structTarea,posicionesQueOcupa,bloquesAocupar);

			log_info(logImongo,"Se procede a guardar en memoria secundaria %d bloques", bloquesAocupar);
			guardarEnMemoriaSecundaria(posicionesQueOcupa,structTarea->file->caracterLlenado,bloquesAocupar,_tarea->parametro);

			actualizarMD5(structTarea);
			config_save(structTarea->config);

			//free(posicionesQueOcupa);
		}
	}
	else{ //SI UNA TAREA NO TIENE ESPACIO EN EL DISCO

		log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", _tarea->nombreTarea);
	}

	unlock(structTarea->mutex);
}


void actualizarMD5(tarea* structTarea){

	char* md5 = obtenerMD5(structTarea->file->bloques);

	//char * freemd5 = structTarea->file->md5_archivo;
	structTarea->file->md5_archivo = md5;

	log_info(logImongo,"md5:%s ", structTarea->file->md5_archivo);


	//free(freemd5);


	config_set_value(structTarea->config,"MD5_ARCHIVO",structTarea->file->md5_archivo);
	config_save(structTarea->config);
}


void guardarStringEnMemoriaSecundaria(int* posicionesQueOcupa, char* contenido, int cantBloquesAocupar){

	log_info(logImongo,"El contenido a guardar es: %s",contenido);

    int offsetMemoria = 0;
    int offsetContenido = 0;

    for(int i = 0; i < cantBloquesAocupar;i++){

        char* contenidoBloque = string_substring(contenido,
        		offsetContenido, min(string_length(contenido) - offsetContenido, superBloque->block_size));

        log_info(logImongo,"Procesando...: %s",contenidoBloque);

        offsetContenido += superBloque->block_size;

        log_info(logImongo,"El offset del contenido es...: %d",offsetContenido);

        offsetMemoria = posicionesQueOcupa[i] * superBloque->block_size;

        log_info(logImongo,"El offset de la memoria es...: %d",offsetMemoria);

        lock(&mutexMemoriaSecundaria);
        memcpy(copiaMemoriaSecundaria + offsetMemoria , contenidoBloque, strlen(contenidoBloque));
        unlock(&mutexMemoriaSecundaria);


        free(contenidoBloque);
    }
    actualizarBitArray(posicionesQueOcupa, cantBloquesAocupar);
}


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
	char * comando = string_from_format("echo -n \"%s\" | md5sum", copiaFile);
	char * md5 = malloc(md5Size +2);
	FILE * file = popen(comando, "r");

	fgets(md5, md5Size+1, file);

	fclose(file);
	free(comando);
	free(copiaFile);
	return md5;
}


bool alcanzanCaracteresParaConsumir(int caracteresEnDisco, int caracteresAremover){

	return caracteresEnDisco > caracteresAremover;
}


void limpiarBitArray(int* bloquesAlimpiar, int cantidadBloquesALimpiar){

	for(int i=0; i<cantidadBloquesALimpiar; i++){

		lock(&mutexBitMap);
		log_info(logImongo, "Se procede a limpiar la posicion %d del bitarray",bloquesAlimpiar[i]);
		bitarray_clean_bit(superBloque->bitmap, bloquesAlimpiar[i]);
		unlock(&mutexBitMap);

	}

	actualizarStringBitMap(cantidadBloquesALimpiar);

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


char* convertirEnString(t_list* listaEnteros){

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


void consumirTarea(tarea* structTarea, t_tarea* _tarea){

	lock(structTarea->mutex);

	if(verificarSiExiste(structTarea->path)){

		log_info(logImongo,"Existe la tarea: %s, se procede a realizarla",_tarea->nombreTarea);

		int caracteresAconsumir = 0;

		//ERROR 1 en la segunda condicion del if
		if(alcanzanCaracteresParaConsumir(structTarea->file->tamanioArchivo, _tarea->parametro)
				&& strcmp(_tarea->nombreTarea,"DESCARTAR_BASURA") != 0){

			log_info(logImongo,"Los caracteres a suprimir son menores que los que estan "
					"actualmente en disco, se procede a operar");

			caracteresAconsumir = _tarea->parametro;

		}
		else{

			log_info(logImongo,"Los caracteres a suprimir son mayores que los que estan"
					"actualmente en disco, se procede a suprimir todos los que hay en disco");

			caracteresAconsumir = structTarea->file->tamanioArchivo;

		}

		t_list* posicionesAlimpiar = list_create();

		int caracteresParaConsumir = caracteresAconsumir;

		for(int i= list_size(structTarea->file->bloques) - 1; caracteresAconsumir > 0 && i >= 0; i--){

			log_info(logImongo,"Los caracteres a consumir son: %d", caracteresAconsumir);

			int posiciones = *((int*)list_get(structTarea->file->bloques,i));

			char* datos = datosBloque(posiciones);

			log_info(logImongo,"Los datos que estan en la posicion %d actualmente son: %s: ", posiciones,datos);

			int cantidadCaracteres = string_length(datos);

			log_info(logImongo,"La cantidad de caracteres de %s es: %d", datos,cantidadCaracteres);

			free(datos);

			char* datosAguardar;

			int caracteresDeLaPasada;

			if(caracteresAconsumir >= cantidadCaracteres){// vos queres consumir 13 caracteres y hay 8

				datosAguardar = string_repeat('\0',superBloque->block_size);
				log_info(logImongo,"Los datos a guardar son: ---- %s ----", datosAguardar);
				list_add(posicionesAlimpiar,list_remove(structTarea->file->bloques, i));
				structTarea->file->cantidadBloques -= 1;
			}
			else{//vos queres consumir 2 y hay 4

				caracteresDeLaPasada = max(0,(cantidadCaracteres - caracteresAconsumir));
				log_info(logImongo,"Esta pasada quuedaron en el bloque %d caracteres", caracteresDeLaPasada);
				datosAguardar = string_repeat(structTarea->file->caracterLlenado[0],caracteresDeLaPasada);
				log_info(logImongo,"Los datos a guardar son: ---- %s ----", datosAguardar);
				char* relleno = string_repeat('\0', superBloque->block_size - caracteresDeLaPasada);
				log_info(logImongo,"El tamaño del relleno es : %d", superBloque->block_size-caracteresDeLaPasada);
				string_append(&datosAguardar, relleno);
				log_info(logImongo,"Los datos a guardar FINALMENTE son: ---- %s ----", datosAguardar);
				free(relleno);
			}

			log_info(logImongo,"El bloque que voy a guardar en memoria es %s ", datosAguardar);

			int offset = posiciones * superBloque->block_size;

			log_info(logImongo,"El offset para guardar en memoria es %d ", offset);
			lock(&mutexMemoriaSecundaria);
			memcpy(copiaMemoriaSecundaria + offset, datosAguardar,superBloque->block_size);
			unlock(&mutexMemoriaSecundaria);

			free(datosAguardar);

			caracteresAconsumir-=cantidadCaracteres;

			log_info(logImongo,"Los caracteres que quedan por consumir son: %d ", max(0,caracteresAconsumir));
		}

		char* bloquesMetaData = convertirEnString(structTarea->file->bloques);

		log_info(logImongo,"Se van a poner en metadata los bloques: %s",bloquesMetaData);

		if(list_size(posicionesAlimpiar) > 0){

		// SE PUEDE BORRAR
		//DE ACA
			log_info(logImongo, "El tamanio de la lista de posiciones a limpiar es: %d", list_size(posicionesAlimpiar));

			int* posicionesAlimpiarBitArray = malloc(sizeof(int) * list_size(posicionesAlimpiar));

			for(int i=0; i<list_size(posicionesAlimpiar);i++){

				posicionesAlimpiarBitArray[i] = *((int*) list_get(posicionesAlimpiar,i));

			}

			log_info(logImongo, "Se van a limpiar en el bitarray las posiciones: ");

			for(int i=0; i<list_size(posicionesAlimpiar); i++){

				log_info(logImongo, "%d ",posicionesAlimpiarBitArray[i]);

			}

		//A ACA
			limpiarBitArray(posicionesAlimpiarBitArray,list_size(posicionesAlimpiar));
			free(posicionesAlimpiarBitArray);
		}

		log_info(logImongo,"Ahora los bloques que ocupa son: %s ", bloquesMetaData);
		structTarea->file->tamanioArchivo -= caracteresParaConsumir;
		log_info(logImongo,"El tamanio del archivo ahora es: %d ", structTarea->file->tamanioArchivo);
		log_info(logImongo,"La cantidad de bloques que ocupa son: %d ", structTarea->file->cantidadBloques);
		config_set_value(structTarea->config,"BLOCKS",bloquesMetaData);
		char * cantidadBloques = string_itoa(structTarea->file->cantidadBloques);
		config_set_value(structTarea->config,"BLOCK_COUNT",cantidadBloques);
		free(cantidadBloques);
		char* tamanioArchivo = string_itoa(structTarea->file->tamanioArchivo);
		config_set_value(structTarea->config,"SIZE",tamanioArchivo);
		free(tamanioArchivo);
		free(bloquesMetaData);
		//actualizarMD5(structTarea);
		config_save(structTarea->config);

		/*void destruirPuntero(int* numero){
			if(numero!=NULL)
				free(numero);
		}
		*/
		//list_iterate(posicionesAlimpiar, (void*) destruirPuntero);
		list_destroy(posicionesAlimpiar);
	}
	else{

		log_info(logImongo,"No existe la tarea: %s",_tarea->nombreTarea);

	}

	unlock(structTarea->mutex);
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


void escribirEnBitacora(char* mensaje, int idTripulante){
/*
	lock(&mutexBitacora);

	char* path = string_from_format("%s/Tripulante%d.ims", pathBitacora,idTripulante);

	log_info(logImongo,"El archivo a crear tiene el path: %s",path);

	t_config* config;

	if(!verificarSiExiste(pathBitacora)){

		log_info(logImongo,"No existe la carpeta Bitacora, se procede a crearla");

		mkdir(pathBitacora,0777);

	}

	if(!(verificarSiExiste(path))){

		log_info(logImongo,"Es la primera vez que el tripulante ID: %d escribe en disco", idTripulante);

		FILE* tripulante = fopen(path,"wb");

		log_info(logImongo,"1");

		fclose(tripulante);

		config = config_create(path);

		log_info(logImongo,"3");

		config_set_value(config,"BLOCKS","[]");

		log_info(logImongo,"4");

		config_set_value(config,"SIZE","0");

		config_save(config);

	}

	else{

		log_info(logImongo,"Ya existia el archivo del tripulante de ID: %d", idTripulante);

		config = config_create(path);

	}
	char** bloquesQueOcupaba = config_get_array_value(config,"BLOCKS");
	t_list * blocksQueOcupaba = convertirEnLista(bloquesQueOcupaba);

//	int posicionUltimoBloque = * (int*)list_get(blocksQueOcupaba, list_size(blocksQueOcupaba) - 1);

//	char* contenidoUltimoBloque = contenidoBloque2(posicionUltimoBloque, fragmentacionDe(posicionUltimoBloque));

	int ultimoBloque = * (int*)list_get(max(list_size(blocksQueOcupaba) -1, 0));

	int fragmentacionUltimoBloque = fragmentacionDe(ultimoBloque);

	int bloquesAocupar;
	int* posicionesQueOcupa;

	if(fragmentacionUltimoBloque != 0){
		char * contenidoUltimoBloque = datosBloque(ultimoBloque);
		string_append(&contenidoUltimoBloque,mensaje);

		int primeraVez = strlen(contenidoUltimoBloque)>0;

		log_info(logImongo,"el contido luego de fragmentacion es: %s",contenidoUltimoBloque);
		int tamanio = strlen(contenidoUltimoBloque) - fragmentacionUltimoBloque;
		log_info(logImongo,"el tamanio luego de fragmentacion es: %d", tamanio);
		bloquesAocupar = (int) ceil((float) tamanio  / (float) superBloque->block_size);
		log_info(logImongo,"los bloques a ocupar luego de fragmentacion son: %d", bloquesAocupar);



		if(bloquesAocupar>=0 && primeraVez){
			posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar + 1);
		}
		else if(bloquesAocupar>0 && !primeraVez){
			posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);
			int* posicionesNuevasAocupar = malloc(bloquesAocupar + 1);

			posicionesNuevasAocupar[0] = ultimoBloque;

			for(int i = 1; i<bloquesAocupar; i++){
				posicionesNuevasAocupar[i] = posicionesQueOcupa[i];
			}

			free(posicionesQueOcupa);
			posicionesQueOcupa = posicionesNuevasAocupar;

		}
		else if(bloquesAocupar==0 && !primeraVez){
			posicionesQueOcupa = malloc(sizeof(int));
			posicionesQueOcupa[0] = ultimoBloque;
		}
		log_info(logImongo,"los bloques a ocupar son: %d",bloquesAocupar);

	}
	else{
		bloquesAocupar = (int) ceil((float) strlen(mensaje)  / (float) superBloque->block_size);
		posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);
	}

	log_info(logImongo,"Los bloques a ocupar de la bitacora del tripulante %d son: %d",idTripulante,bloquesAocupar);

	if(bloquesLibres(bloquesAocupar)){

		log_info(logImongo,"Existen bloques libres por ocupar, se procede a "
				"escribir en disco la bitacora del tripulante ID: %d", idTripulante);


		guardarStringEnMemoriaSecundaria(posicionesQueOcupa,mensaje, bloquesAocupar);

    	actualizarBitArray(posicionesQueOcupa, bloquesAocupar);

		int cantidadDeCaracteresQueHabia = config_get_int_value(config,"SIZE");

		log_info(logImongo,"La cantidad de caracteres que habia en disco de la "
				"bitacora del tripulante %d eran: %d", idTripulante,cantidadDeCaracteresQueHabia);

		config_set_value(config,"SIZE",string_itoa(string_length(mensaje) + cantidadDeCaracteresQueHabia));

		log_info(logImongo,"La cantidad de caracteres que hay ahora del "
				"tripulante %d son: %d", idTripulante,cantidadDeCaracteresQueHabia);

		for(int i=0; i<bloquesAocupar;i++){

			list_add(blocksQueOcupaba, posicionesQueOcupa + i);
		}
		char * bloquesQueOcupa = convertirEnString(blocksQueOcupaba);
		config_set_value(config, "BLOCKS", bloquesQueOcupa);
		config_save(config);

		free(bloquesQueOcupaba);
		//free(bloquesNuevos);

		mandarOKAdiscordiador(tripulanteSock);
	}
	else{

		log_error(logImongo,"No hay mas espacio en el disco para escribir en bitacora");

		mandarErrorAdiscordiador(tripulanteSock);
	}

	unlock(&mutexBitacora);
*/
}


void liberarConfiguracion(){

	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);
}


void liberarStructTareas(t_file* file){

	list_destroy(file->bloques);
	free(file->caracterLlenado);
	free(file->md5_archivo);
	free(file);
}

void liberarTodosLosStructTareas(){

	//liberarStructTareas(oxigeno);
	//liberarStructTareas(comida);
	//liberarStructTareas(basura);
}


void liberarTareas(){

	for(int i=0; i<6; i++){
		free(tareas[i]);
	}
	free(tareas);
}
