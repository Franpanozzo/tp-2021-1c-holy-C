#include "utils.h"


void crearConfig(t_config** config, char* path){

	*config  = config_create(path);

	if(*config == NULL){

		log_error(logImongo, "La ruta es incorrecta ");

		exit(1);
	}
}//



char * pathLog(){

	char *pathLog = string_new();
	char *fecha = temporal_get_string_time("%d-%m-%y %H:%M:%S");
	string_append(&pathLog, "/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/logs/");
	string_append(&pathLog, "log ");
	string_append(&pathLog, fecha);
	string_append(&pathLog, ".log");
	free(fecha);
	return pathLog;
}//


void asignarTareas(){

	tareas = malloc(sizeof(char*) * 6);

	tareas[0] = strdup("GENERAR_OXIGENO");
	tareas[1] = strdup("CONSUMIR_OXIGENO");
	tareas[2] = strdup("GENERAR_COMIDA");
	tareas[3] = strdup("CONSUMIR_COMIDA");
	tareas[4] = strdup("GENERAR_BASURA");
	tareas[5] = strdup("DESCARTAR_BASURA");
}//


int indiceTarea(t_tarea* tarea){

		int i = 0;

			while(tareas[i] != NULL){

				if(string_contains(tarea->nombreTarea, tareas[i])){

					return i;
				}

				i++;
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

}//


void cargarDatosConfig(){

	datosConfig = malloc(sizeof(t_datosConfig));
	datosConfig->puntoMontaje = config_get_string_value(configImongo,"PUNTO_MONTAJE");
	datosConfig->puerto = (uint32_t)config_get_int_value(configImongo,"PUERTO");
	datosConfig->tiempoSincronizacion = (uint32_t)config_get_int_value(configImongo,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = config_get_string_value(configImongo,"POSICIONES_SABOTAJE");

}//


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

}


bool validarExistenciaFileSystem(char* superBloque, char* blocks, char* raiz){

	return (access(superBloque, F_OK ) != -1) && (access(blocks, F_OK ) != -1) && (access(raiz, F_OK ) != -1);

}//


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
}//


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

}//


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

	log_info(logImongo, "La cantidad de bloques Libres es: %d", cantidadDeBloquesLibres);

	return bloquesAocupar <= cantidadDeBloquesLibres ;

}//


bool verificarSiExiste(char* pathArchivo){

	return access(pathArchivo,F_OK) != -1;

}//


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

}//


void actualizarStringBitMap(){

int sizeBitArray = superBloque->blocks;

char* bitmap = malloc(sizeBitArray + 1);

  for(int i=0; i<sizeBitArray;i++){

	  	lock(&mutexBitMap);
    	int valor =  bitarray_test_bit(superBloque->bitmap, i);
    	unlock(&mutexBitMap);

    	bitmap[i] = valor + '0';

    	//printf("%c ", bitmap[i]);

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

void actualizarEstructurasFile(t_file* file, t_config* config){

	file->bloquesQueOcupa = config_get_string_value(config,"BLOCKS");
	file->cantidadBloques = config_get_int_value(config,"BLOCK_COUNT");
	//file->caracterLlenado = config_get_string_value(config,"CARACTER_LLENADO");
	file->md5_archivo = config_get_string_value(config,"MD5_ARCHIVO");
	file->tamanioArchivo = config_get_int_value(config,"SIZE");

}


void actualizarPosicionesFile(t_file* archivo, int* arrayDePosiciones, t_config* config, int bloquesAocupar){

	char* bloquesQueTenia = archivo->bloquesQueOcupa;

	log_info(logImongo,"Los bloques que tenia son: %s \n", bloquesQueTenia);

	int tamanio = string_length(bloquesQueTenia);

	log_info(logImongo,"El tamanio de los bloques que tenia es: %d \n", tamanio);

	char* bloquesActuales;

	if(tamanio > 0){

		bloquesActuales = string_substring(bloquesQueTenia,0,tamanio - 1);

		log_info(logImongo,"%s", bloquesActuales);
	}

	else if(tamanio == 0){

		bloquesActuales = string_new();
		log_info(logImongo,"%s", bloquesActuales);

	}

	else{

		log_info(logImongo, "No puede tener tamanio negativo un string");

	}

	log_info(logImongo,"Ahora el bloque quedo como: %s \n", bloquesActuales);

	int i = 0;

	while ( i < bloquesAocupar){

		if(tamanio != 0){

		//CASO EN EL QUE EL STRING TE VIENE ASI: [2
		string_append(&bloquesActuales,",");
		log_info(logImongo,"%s", bloquesActuales);

		}
		else{
		//CASO EN EL QUE EL STRING TE VIENE VACIO
		string_append(&bloquesActuales,"[");
		log_info(logImongo,"%s", bloquesActuales);
		tamanio++;

		}


		log_info(logImongo,"%s \n", bloquesActuales);
		char * posicion = string_itoa(*(arrayDePosiciones + i));
		string_append(&bloquesActuales,posicion);
		log_info(logImongo,"%s \n", bloquesActuales);
		i++;

	}
	string_append(&bloquesActuales,"]");
	log_info(logImongo,"%s \n", bloquesActuales);

	archivo->bloquesQueOcupa = bloquesActuales;
	archivo->cantidadBloques += bloquesAocupar;

	log_info(logImongo,"Se procede a actualizar las estructuras y archivos con los valores nuevos: ----%s--------%d----",archivo->bloquesQueOcupa,archivo->cantidadBloques);

	config_set_value(config,"BLOCKS",archivo->bloquesQueOcupa);
	config_set_value(config,"BLOCK_COUNT",string_itoa(archivo->cantidadBloques));
	config_save(config);

}//

int min(int a,int b){

    if(a>b){

        return b;

    }

    else{

        return a;

    }

}//
int max(int a,int b){

    if(a>b){

        return a;

    }

    else{

        return b;

    }

}//

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

	printf("flag = %d \n", flag);

	char caracter = caracterLlenado[0];

	printf("caracter = %c \n", caracter);

	char** palabraAguardar = malloc(sizeof(char*) * bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){

		palabraAguardar[i] = string_repeat(caracter,min(superBloque->block_size,flag));
		printf("Lo que pesa cada char es %d ", strlen(palabraAguardar[i]));
		printf("palabra = %s \n", palabraAguardar[i]);
		flag -= superBloque->block_size;
		printf("flag = %d \n", flag);
	}

	for(int i=0; i<bloquesAocupar; i++){

		int offset = posicionesQueOcupa[i] * superBloque->block_size;

		printf("offset = %d \n", offset);

		lock(&mutexMemoriaSecundaria);

		memcpy(copiaMemoriaSecundaria + offset ,palabraAguardar[i], strlen(palabraAguardar[i]));
		unlock(&mutexMemoriaSecundaria);

		printf("Lo que termino guardando fue %d ", superBloque->block_size);
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

	memcpy(bloque,copiaMemoriaSecundaria + offset,(int)superBloque->block_size);

	bloque[superBloque->block_size] = '\0';

	return bloque;
}


int fragmentacionDe(int ultimoBloqueTarea){

	char* bloque = datosBloque(ultimoBloqueTarea);

	int tamanioBloque = string_length(bloque);

	log_info(logImongo,"El tamanio del bloque es: %d", tamanioBloque);

	int fragmentacion = ((int) superBloque->block_size) - tamanioBloque;

	log_info(logImongo,"La fragmentacion del bloque es: %d", fragmentacion);

	//free(bloque);

	return fragmentacion;
}


int ultimoBloqueDeLa(tarea* structTarea){

	char** bloquesQueOcupa = string_get_string_as_array(structTarea->file->bloquesQueOcupa);

	for(int i=0; i<structTarea->file->cantidadBloques;i++){

		log_info(logImongo,"Los bloques que ocupaba antes son: %s",bloquesQueOcupa[i]);

	}

	int ultimoBloque = atoi(*(bloquesQueOcupa + (structTarea->file->cantidadBloques - 1))); // VER BIEN SI ESTA BIEN ESTO

	log_info(logImongo,"El numero del ultimo bloque es: %d",ultimoBloque);

	int i = 0;

	while(bloquesQueOcupa[i] != NULL){

	free(bloquesQueOcupa[i]);

	i++;

	}

	free(bloquesQueOcupa);


	return ultimoBloque;

}


t_info*  nuevosBloquesAocupar(tarea* structTarea,t_tarea* _tarea){

	t_info* capacidad = malloc(sizeof(t_info));

	int ultimoBloqueDeLaTarea = ultimoBloqueDeLa(structTarea);

	int fragmentacionInterna = fragmentacionDe(ultimoBloqueDeLaTarea);

	char* bloque = malloc(superBloque->block_size + 1);

	if(_tarea->parametro > fragmentacionInterna){

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d , se procede a devolver el ultimo bloque entero y mas los bloques a ocupar",_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		for(int i=0; i<superBloque->block_size;i++){

			bloque[i] = structTarea->file->caracterLlenado[0];

		}

		bloque[superBloque->block_size] = '\0';

		log_info(logImongo,"El bloque que se va a guardar es: %s", bloque);

		lock(&mutexMemoriaSecundaria);
		memcpy(copiaMemoriaSecundaria + (ultimoBloqueDeLaTarea * superBloque->block_size),bloque,superBloque->block_size);
		unlock(&mutexMemoriaSecundaria);

		free(bloque);

		capacidad->bloquesNuevosAocupar  = (int) ceil((float) (_tarea->parametro - fragmentacionInterna) / (float) superBloque->block_size);

		capacidad->caracteresAguardar = _tarea->parametro - fragmentacionInterna;

		return capacidad;

	}
	else if(_tarea->parametro == fragmentacionInterna){

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d , se procede a devolver el ultimo bloque entero, pero no hay mas bloques que ocupar",_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		log_info(logImongo,"El bloque que se va a guardar es: ");

		for(int i=0; i<superBloque->block_size;i++){

			bloque[i] = structTarea->file->caracterLlenado[0];

		}

		bloque[superBloque->block_size] = '\0';

		log_info(logImongo,"El bloque que se va a guardar es: %s", bloque);

	}
	else{

		log_info(logImongo,"Lo que ocupa la tarea es : %d  caracteres %s y la fragmentacion es: %d , se procede a devolver el ultimo bloque incompleto, y no hay mas bloques que ocupar",_tarea->parametro,structTarea->file->caracterLlenado, fragmentacionInterna);

		for(int i=0; i<((superBloque->block_size - fragmentacionInterna) + _tarea->parametro);i++){

			bloque[i] = structTarea->file->caracterLlenado[0];

		}

		bloque[superBloque->block_size] = '\0';

		int i = 0;

		while(bloque[i] != '\0'){

			if(bloque[i] != structTarea->file->caracterLlenado[0]){

				bloque[i] = '\0';

			}

			i++;
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

		structTarea->configSeCreo = true;
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

void generarTarea(tarea* structTarea, t_tarea* _tarea, int* tripulanteSock){

	int caracteresAOcupar = _tarea->parametro;
	int bloquesAocupar = (int) ceil((float) caracteresAOcupar / (float) superBloque->block_size);

	log_info(logImongo, "Los bloques a ocupar de la tarea: %s son: %d ",_tarea->nombreTarea, bloquesAocupar);
	lock(structTarea->mutex);
	if(bloquesLibres(bloquesAocupar)){ //SI UNA TAREA TIENE ESPACIO EN EL DISCO

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",_tarea->nombreTarea);

		if(verificarSiExiste(structTarea->path)){ // SI NO ES LA PRIMERA VEZ QUE VIENE ESA TAREA

			crearConfigTarea(structTarea);

			actualizarEstructurasFile(structTarea->file, structTarea->config);

			log_info(logImongo,"Los bloques que ocupaba antes son: %s que serian %d bloques \n", structTarea->file->bloquesQueOcupa, bloquesAocupar);

			structTarea->file->tamanioArchivo += caracteresAOcupar;
			char* tamanioArchivo = string_itoa(structTarea->file->tamanioArchivo);
			config_set_value(structTarea->config,"SIZE",tamanioArchivo);
			config_save(structTarea->config);

			if(structTarea->file->cantidadBloques > 0){

				t_info* capacidad;

				capacidad = nuevosBloquesAocupar(structTarea,_tarea);

				bloquesAocupar = capacidad->bloquesNuevosAocupar;

				if(bloquesAocupar > 0){

					log_info(logImongo,"La cantidad de bloques que ocupa ahora son: %d \n", bloquesAocupar);

					int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

					log_info(logImongo, "Las posiciones que va a ocupar en disco rigido la tarea %s, de %d bytes, DESPUES del filtro de la fragmentacion interna son: ",_tarea->nombreTarea, _tarea->parametro);

					for(int i=0; i<bloquesAocupar;i++){

						log_info(logImongo, "%d, " , posicionesQueOcupa[i]);
					}

					actualizarPosicionesFile(structTarea->file,posicionesQueOcupa,structTarea->config,bloquesAocupar);

					log_info(logImongo,"Se porcede a guardar en memoria secundaria %d bloques", bloquesAocupar);
					guardarEnMemoriaSecundaria(posicionesQueOcupa,structTarea->file->caracterLlenado,bloquesAocupar,capacidad->caracteresAguardar);

					//actualizarMD5(structTarea);
					//config_set_value(structTarea->config,"MD5_ARCHIVO",structTarea->file->md5_archivo);
					//config_save(structTarea->config);
				}

				mandarOKAdiscordiador(tripulanteSock);
				free(tamanioArchivo);
				free(capacidad);


			}

		}

		else{ //SI ES LA PRIMERA VEZ QUE VIENE ESA TAREA

			FILE* archivoOxigeno = fopen(structTarea->path,"wb");
			fclose(archivoOxigeno);

			crearConfigTarea(structTarea);

			inicializarTarea(structTarea, bloquesAocupar, caracteresAOcupar);

			actualizarEstructurasFile(structTarea->file, structTarea->config);

			log_info(logImongo,"La tarea %s es la primera vez que ingresa, los bloques que va a ocupar son: %d \n", _tarea->nombreTarea,bloquesAocupar);

			int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

			log_info(logImongo, "Las posiciones que va a ocupar en disco rigido la tarea %s, de %d bytes son: ",_tarea->nombreTarea, _tarea->parametro);

			for(int i=0; i<bloquesAocupar;i++){

				log_info(logImongo, "%d, ",posicionesQueOcupa[i]);
			}

			actualizarPosicionesFile(structTarea->file,posicionesQueOcupa,structTarea->config,bloquesAocupar);

			log_info(logImongo,"Se procede a guardar en memoria secundaria %d bloques", bloquesAocupar);
			guardarEnMemoriaSecundaria(posicionesQueOcupa,structTarea->file->caracterLlenado,bloquesAocupar,_tarea->parametro);

			//actualizarMD5(structTarea);
			//config_set_value(structTarea->config,"MD5_ARCHIVO",structTarea->file->md5_archivo);
			config_save(structTarea->config);
			mandarOKAdiscordiador(tripulanteSock);

			//free(posicionesQueOcupa);

		}

	}

	else{ //SI UNA TAREA NO TIENE ESPACIO EN EL DISCO

		log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", _tarea->nombreTarea);

		mandarErrorAdiscordiador(tripulanteSock);

	}
	unlock(structTarea->mutex);
}


void actualizarMD5(tarea* structTarea){
    // arrayBloques = ["1","3"]  array de strings
    char ** arrayBloques = string_get_string_as_array(structTarea->file->bloquesQueOcupa);
    void* copiaFile = malloc(structTarea->file->cantidadBloques * superBloque->block_size);

    for(int i=0; i<structTarea->file->cantidadBloques;i++){
        int offsetMemoria = atoi(*(arrayBloques + i)) * superBloque->block_size;
        int offsetFile = i * superBloque->block_size;
        memcpy(copiaFile + offsetFile,memoriaSecundaria + offsetMemoria ,superBloque->block_size);
        //memcpy(copiaFile+offsetFile,memoriaSecundaria + offsetMemoria ,min(superBloque->block_size,( structTarea->file->tamanioArchivo - offsetFile)));
    }
    char* nombreFile = string_from_format("%s.md5.tmp",structTarea->file->caracterLlenado);
    FILE * file = fopen(nombreFile,"w");
    log_info(logImongo,"Archivo construido para md5: %s", (char*) copiaFile);
    fputs((char*) copiaFile, file);
    fclose(file);

    char * comando = string_from_format("md5sum %s", nombreFile);
    //md5sum

    int md5Size = 32;
    char * md5 = malloc(md5Size);
    file = popen(comando, "r");
    fscanf(file,"%s",md5);

    fgets(md5, md5Size, file);
    log_info(logImongo,"md5: %s \n",md5);
    remove(nombreFile);
    structTarea->file->md5_archivo = md5;

    free(copiaFile);

}

bool alcanzanCaracteresParaConsumir(int caracteresEnDisco, int caracteresAremover){

	return caracteresEnDisco > caracteresAremover;

}


void limpiarBitArray(int* bloquesAlimpiar, int cantidadBloquesALimpiar){

	for(int i=0; i<cantidadBloquesALimpiar; i++){

		lock(&mutexBitMap);
		bitarray_clean_bit(superBloque->bitmap, bloquesAlimpiar[i]);
		unlock(&mutexBitMap);

	}

	actualizarStringBitMap(cantidadBloquesALimpiar);

}


int saberUltimoBloqueTarea(tarea* structTarea){

	char** ultimoBloqueTarea = string_get_string_as_array(structTarea->file->bloquesQueOcupa);

	int numeroUltimoBloque = atoi(*(ultimoBloqueTarea + structTarea->file->cantidadBloques));

	for(int i=0; i<structTarea->file->cantidadBloques;i++){

		free(ultimoBloqueTarea[i]);
	}

	free(ultimoBloqueTarea);

	return numeroUltimoBloque;
}

char* suprimirCaracteres(int cantidadAsuprimir, char caracterLlenado){

	char* stringAguardar = malloc(superBloque->block_size);

	for(int i=0; i<superBloque->block_size; i++){

		stringAguardar[i] = '\0';
	}

	for(int i=0; i<cantidadAsuprimir;i++){
		stringAguardar[i] = caracterLlenado;
	}

	for(int i=0; i<cantidadAsuprimir;i++){
			stringAguardar[i] = '\0';
		}

	return stringAguardar;

}


void consumirTarea(tarea* structTarea, t_tarea* _tarea, int* tripulanteSock){

	lock(structTarea->mutex);

	if(verificarSiExiste(structTarea->path)){

		log_info(logImongo,"Existe la tarea: %s, se procede a realizarla",_tarea->nombreTarea);

		int caracteresAconsumir = 0;

		int bloquesAconsumir = 0;

		if(alcanzanCaracteresParaConsumir(structTarea->file->tamanioArchivo, _tarea->parametro)){

			log_info(logImongo,"Los caracteres a suprimir son menores que los que estan actualmente en disco, se procede a operar");

			caracteresAconsumir = _tarea->parametro;

			bloquesAconsumir = (int) ceil((float) caracteresAconsumir / (float) superBloque->block_size);

		}

		else{

			log_info(logImongo,"Los caracteres a suprimir son mayores que los que estan actualmente en disco, se procede a suprimir todos los que hay en disco");

			caracteresAconsumir = structTarea->file->tamanioArchivo;

			bloquesAconsumir = structTarea->file->cantidadBloques;

		}

			char** posiciones = string_get_string_as_array(string_reverse(structTarea->file->bloquesQueOcupa));

			int* posicionesEnNumero = malloc(sizeof(int)* structTarea->file->cantidadBloques);

			for(int i=0; i<structTarea->file->cantidadBloques;i++){

				posicionesEnNumero[i] = atoi(*(posiciones) + i);

				free(posiciones[i]);

			}

			free(posiciones);

			int* posicionesAlimpiar = malloc(sizeof(int)* bloquesAconsumir);

			int i=0;

			while(caracteresAconsumir >= 0){

				char* datos = datosBloque(posicionesEnNumero[i]);
				int cantidadCaracteres = string_length(datos);
				if(caracteresAconsumir < cantidadCaracteres){

					cantidadCaracteres = caracteresAconsumir;

				}
				char* datosAguardar = suprimirCaracteres(cantidadCaracteres,structTarea->file->caracterLlenado[0]);
				int offset = posicionesEnNumero[i] * superBloque->block_size;
				lock(&mutexMemoriaSecundaria);
				memcpy(copiaMemoriaSecundaria + offset, datosAguardar,superBloque->block_size);
				unlock(&mutexMemoriaSecundaria);
				caracteresAconsumir-=cantidadCaracteres;
				posicionesAlimpiar[i] = posicionesEnNumero[i];
				i++;

			}

			char* bloquesMetaData = strdup("[");

			for(int j=i; j<structTarea->file->cantidadBloques;i++){

				if(j!=i){
					string_append(&bloquesMetaData,", ");
				}
				string_append(&bloquesMetaData,string_itoa(posicionesEnNumero[j]));
			}
			string_append(&bloquesMetaData,"]");

			char* bloquesAguardarArchivo = string_reverse(bloquesMetaData);

			limpiarBitArray(posicionesAlimpiar,bloquesAconsumir);

			structTarea->file->bloquesQueOcupa = bloquesAguardarArchivo;
			structTarea->file->cantidadBloques -= bloquesAconsumir;
			structTarea->file->tamanioArchivo -= caracteresAconsumir;

			config_set_value(structTarea->config,"BLOCKS",structTarea->file->bloquesQueOcupa);
			//config_set_value(structTarea->config,"MD5_ARCHIVO","");
			config_set_value(structTarea->config,"BLOCK_COUNT",string_itoa(structTarea->file->cantidadBloques));
			config_set_value(structTarea->config,"SIZE",string_itoa(structTarea->file->tamanioArchivo));
			config_save(structTarea->config);

		mandarOKAdiscordiador(tripulanteSock);

	}

	else{

		log_info(logImongo,"No existe la tarea: %s, se procede a mandar mensaje de error a Discordiador",_tarea->nombreTarea);
		mandarErrorAdiscordiador(tripulanteSock);
	}

	unlock(structTarea->mutex);

}


void descartarBasura(t_tarea* tarea, int* tripulanteSock){
	lock(basura->mutex);
	if(verificarSiExiste(basura->path)){
		log_info(logImongo,"Elimninado basura.ims");
		mandarOKAdiscordiador(tripulanteSock);
	}
	else{
		log_info(logImongo,"No se puede eliminar basura.ims porque no existes");
		mandarErrorAdiscordiador(tripulanteSock);
	}
	unlock(basura->mutex);
}


void liberarConfiguracion(){

	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);

}//


void liberarStructTareas(t_file* file){

	free(file->bloquesQueOcupa);
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
}//
