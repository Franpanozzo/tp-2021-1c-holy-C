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

	int tamanio = sizeof(t_file);

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
	//comida = malloc(tamanio);
	//basura = malloc(tamanio);
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

		    log_info(logImongo, "Tamanio total del archivo: %lu bytes\n", sb.st_size);
		    log_info(logImongo, "Ultimo estado:       %s", ctime(&sb.st_ctime));
		    log_info(logImongo, "Ultimo acceso:         %s", ctime(&sb.st_atime));
		    log_info(logImongo, "Ultima modificacion del archivo:   %s", ctime(&sb.st_mtime));
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

    	//log_info(logImongo, "%c ", bitmap[i]);

   }

/*
  for(int i = sizeBitArray; i<bitarray_get_max_bit(superBloque->bitmap);i++){

	  int valor =  bitarray_test_bit(superBloque->bitmap, i);

	      	log_info(logImongo, "%d ", valor);

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

void actualizarEstructurasFile(t_file* file, t_config* config, pthread_mutex_t* mutex){

	file->bloquesQueOcupa = config_get_string_value(config,"BLOCKS");
	file->cantidadBloques = config_get_int_value(config,"BLOCK_COUNT");
	file->caracterLlenado = config_get_string_value(config,"CARACTER_LLENADO");
	file->md5_archivo = config_get_string_value(config,"MD5_ARCHIVO");
	file->tamanioArchivo = config_get_int_value(config,"SIZE");

}


void actualizarPosicionesFile(t_file* archivo, int* arrayDePosiciones, t_config* config, int bloquesAocupar){

	char* bloquesQueTenia = archivo->bloquesQueOcupa;

	log_info(logImongo, "Los bloques que tenia son: %s \n", bloquesQueTenia);

	int tamanio = string_length(bloquesQueTenia);

	log_info(logImongo, "El tamanio de los bloques que tenia es: %d \n", tamanio);

	char* bloquesActuales;

	if(tamanio > 0){

		bloquesActuales = string_substring(bloquesQueTenia,0,tamanio - 1);
	}

	else if(tamanio == 0){

		bloquesActuales = string_new();

	}

	else{

		log_info(logImongo, "No puede tener tamanio negativo un string");

	}

	log_info(logImongo, "Ahora el bloque quedo como: %s \n", bloquesActuales);

	int i = 0;

	while ( i < bloquesAocupar){

		if(tamanio != 0){

		//CASO EN EL QUE EL STRING TE VIENE ASI: [2
		string_append(&bloquesActuales,",");

		}
		else{
		//CASO EN EL QUE EL STRING TE VIENE VACIO
		string_append(&bloquesActuales,"[");
		tamanio++;

		}


		log_info(logImongo, "%s \n", bloquesActuales);
		char * posicion = string_itoa(*(arrayDePosiciones + i));
		string_append(&bloquesActuales,posicion);
		log_info(logImongo, "%s \n", bloquesActuales);
		i++;

	}
	string_append(&bloquesActuales,"]");
	log_info(logImongo, "%s \n", bloquesActuales);
/*
	int j = 0;
	int cantidadComas = 0;


	while(bloquesActuales[j] != '\0'){

		if(bloquesActuales[j] == ','){

			cantidadComas++;

			}

		j++;
	}
*/
	archivo->bloquesQueOcupa = bloquesActuales;
	//archivo->cantidadBloques = cantidadComas + 1;
	//archivo->tamanioArchivo = (cantidadComas + 1) * superBloque->block_size;

	//config_set_value(config,"BLOCK_COUNT",string_itoa(archivo->cantidadBloques));
	//config_set_value(config,"SIZE",string_itoa(archivo->tamanioArchivo));
	config_set_value(config,"BLOCKS",archivo->bloquesQueOcupa);
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


void guardarEnMemoriaSecundaria(t_tarea* tarea, int* posicionesQueOcupa,char* caracterLlenado, int bloquesAocupar){

	int flag = tarea->parametro;

	log_info(logImongo, "flag = %d \n", flag);

	char caracter = caracterLlenado[0];

	log_info(logImongo, "caracter = %c \n", caracter);

	char** palabraAguardar = malloc(sizeof(char*) * bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){

		palabraAguardar[i] = string_repeat(caracter,min(superBloque->block_size,flag));
		log_info(logImongo, "Lo que pesa cada char es %d ", strlen(palabraAguardar[i]));
		log_info(logImongo, "palabra = %s \n", palabraAguardar[i]);
		flag -= superBloque->block_size;
		log_info(logImongo, "flag = %d \n", flag);
	}

	for(int i=0; i<bloquesAocupar; i++){

		int offset = posicionesQueOcupa[i] * superBloque->block_size;

		log_info(logImongo, "offset = %d \n", offset);

		lock(&mutexMemoriaSecundaria);
		memcpy(copiaMemoriaSecundaria + offset ,palabraAguardar[i],strlen(palabraAguardar[i]));
		unlock(&mutexMemoriaSecundaria);

		log_info(logImongo, "Lo que termino guardando fue %d ", superBloque->block_size);
	}

	actualizarBitArray(posicionesQueOcupa, bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){
		free(palabraAguardar[i]);
	}
	free(palabraAguardar);

}


void generarTarea(tarea* structTarea, t_tarea* _tarea, int* tripulanteSock){
	int caracteresAOcupar = _tarea->parametro;
	int bloquesAocupar = (int) ceil((float) caracteresAOcupar / (float) superBloque->block_size);

	log_info(logImongo, "Los bloues a ocupar de la tarea: %s son: %d ",_tarea->nombreTarea, bloquesAocupar);
	lock(structTarea->mutex);
	if(bloquesLibres(bloquesAocupar)){

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",_tarea->nombreTarea);

		if(verificarSiExiste(structTarea->path)){


			if(structTarea->configSeCreo != true){

				structTarea->configSeCreo = true;
				structTarea->config  = config_create(structTarea->path);

				if(structTarea->config == NULL){

					log_error(logImongo, "La ruta es incorrecta ");

					exit(1);
				}
			}


			actualizarEstructurasFile(structTarea->file, structTarea->config, structTarea->mutex);

			log_info(logImongo,"Los bloques que ocupa al momento son: %s \n", structTarea->file->bloquesQueOcupa);


			structTarea->file->cantidadBloques += bloquesAocupar;
			//char*cantidadBloques = string_itoa(structTarea->file->cantidadBloques);
			config_set_value(structTarea->config,"BLOCK_COUNT",string_itoa(structTarea->file->cantidadBloques));
			//free(cantidadBloques);
			structTarea->file->tamanioArchivo += caracteresAOcupar;
			//char* tamanioArchivo = string_itoa(structTarea->file->tamanioArchivo);
			config_set_value(structTarea->config,"SIZE",string_itoa(structTarea->file->tamanioArchivo));
			//free(tamanioArchivo);
			config_save(structTarea->config);


		}

		else{

			FILE* archivoOxigeno = fopen(structTarea->path,"wb");
			fclose(archivoOxigeno);

			if(structTarea->configSeCreo != true){
				structTarea->configSeCreo = true;

				structTarea->config  = config_create(structTarea->path);

				if(structTarea->config == NULL){

					log_error(logImongo, "La ruta es incorrecta ");

					exit(1);
				}
			}

			config_set_value(structTarea->config,"CARACTER_LLENADO",structTarea->file->caracterLlenado);
			config_set_value(structTarea->config,"BLOCKS","");
			config_set_value(structTarea->config,"MD5_ARCHIVO","");
			//char* cantidadBloques =  string_itoa(bloquesAocupar);
			config_set_value(structTarea->config,"BLOCK_COUNT",string_itoa(bloquesAocupar));
			//free(cantidadBloques);
			//char* tamanioArchivo = string_itoa(caracteresAOcupar);
			config_set_value(structTarea->config,"SIZE",string_itoa(caracteresAOcupar));
			//free(tamanioArchivo);
			config_save(structTarea->config);



			actualizarEstructurasFile(structTarea->file, structTarea->config, structTarea->mutex);

		}


		int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

		//lock(&mutexEstructuraOxigeno);
		actualizarPosicionesFile(structTarea->file,posicionesQueOcupa,structTarea->config,bloquesAocupar);
		//unlock(&mutexEstructuraOxigeno);

		guardarEnMemoriaSecundaria(_tarea,posicionesQueOcupa,structTarea->file->caracterLlenado,bloquesAocupar);
		actualizarMD5(structTarea);
		config_set_value(structTarea->config,"MD5_ARCHIVO",structTarea->file->md5_archivo);
		config_save(structTarea->config);
		mandarOKAdiscordiador(tripulanteSock);


		free(posicionesQueOcupa);

	}

	else{

		mandarErrorAdiscordiador(tripulanteSock);

		log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", _tarea->nombreTarea);

	}
	unlock(structTarea->mutex);
}


void actualizarMD5(tarea* structTarea){
	// arrayBloques = ["1","3"]  array de strings
	char ** arrayBloques = string_get_string_as_array(structTarea->file->bloquesQueOcupa);
	char * copiaFile = malloc(structTarea->file->tamanioArchivo);

	for(int i=0; i<structTarea->file->cantidadBloques;i++){
		int offsetMemoria = atoi(*(arrayBloques + i)) * superBloque->block_size;
		int offsetFile = i * superBloque->block_size;
		log_info(logImongo, "%d\n",offsetMemoria);
		log_info(logImongo, "%d\n",offsetFile);
		memcpy(copiaFile+offsetFile,memoriaSecundaria + offsetMemoria ,superBloque->block_size);
		//memcpy(copiaFile+offsetFile,memoriaSecundaria + offsetMemoria ,min(superBloque->block_size,( structTarea->file->tamanioArchivo - offsetFile)));
	}
	char* nombreFile = string_from_format("%s.md5.tmp",structTarea->file->caracterLlenado);
	FILE * file = fopen(nombreFile,"w");
	log_info(logImongo,"-------------Archivo construido para md5: %s---------", copiaFile);
	fputs(copiaFile, file);
	fclose(file);

	char * comando = string_from_format("md5sum %s", nombreFile);
	//md5sum


	int md5Size = 32;
	char * md5 = malloc(md5Size);
	file = popen(comando, "r");
	//fscanf(file,"%s",md5);

	fgets(md5, md5Size, file);
	log_info(logImongo,"md5: %s \n",md5);
	//remove(nombreFile);
	structTarea->file->md5_archivo = md5;


}


void generarOxigeno(t_tarea* _tarea, int* tripulanteSock){

	int bloquesAocupar = (int) ceil((float) _tarea->parametro / (float) superBloque->block_size);

	log_info(logImongo, "Los bloues a ocupar de la tarea: %s son: %d ",_tarea->nombreTarea, bloquesAocupar);
	lock(oxigeno->mutex);
	if(bloquesLibres(bloquesAocupar)){

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",_tarea->nombreTarea);

		if(verificarSiExiste(pathOxigeno)){


			if(oxigeno->configSeCreo != true){

				oxigeno->configSeCreo = true;
				oxigeno->config  = config_create(pathOxigeno);

				if(oxigeno->config == NULL){

					log_error(logImongo, "La ruta es incorrecta ");

					exit(1);
				}
			}


			actualizarEstructurasFile(oxigeno->file, oxigeno->config, oxigeno->mutex);

			log_info(logImongo,"Los bloques que ocupa al momento son: %s \n", oxigeno->file->bloquesQueOcupa);


			oxigeno->file->cantidadBloques += bloquesAocupar;
			config_set_value(oxigeno->config,"BLOCK_COUNT",string_itoa(oxigeno->file->cantidadBloques));
			config_set_value(oxigeno->config,"SIZE",string_itoa(oxigeno->file->cantidadBloques*superBloque->block_size));
			config_save(oxigeno->config);


		}

		else{

			FILE* archivoOxigeno = fopen(pathOxigeno,"wb");
			fclose(archivoOxigeno);

			if(oxigeno->configSeCreo != true){
				oxigeno->configSeCreo = true;

				oxigeno->config  = config_create(pathOxigeno);

				if(oxigeno->config == NULL){

					log_error(logImongo, "La ruta es incorrecta ");

					exit(1);
				}
			}

			config_set_value(oxigeno->config,"CARACTER_LLENADO","O");
			config_set_value(oxigeno->config,"BLOCKS","");
			config_set_value(oxigeno->config,"MD5_ARCHIVO","");
			config_set_value(oxigeno->config,"BLOCK_COUNT",string_itoa(bloquesAocupar));
			config_set_value(oxigeno->config,"SIZE",string_itoa(bloquesAocupar*superBloque->block_size));
			config_save(oxigeno->config);



			actualizarEstructurasFile(oxigeno->file, oxigeno->config, oxigeno->mutex);

		}


		int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);

		//lock(&mutexEstructuraOxigeno);
		actualizarPosicionesFile(oxigeno->file,posicionesQueOcupa,oxigeno->config,bloquesAocupar);
		//unlock(&mutexEstructuraOxigeno);

		guardarEnMemoriaSecundaria(_tarea,posicionesQueOcupa,oxigeno->file->caracterLlenado,bloquesAocupar);

		mandarOKAdiscordiador(tripulanteSock);


		free(posicionesQueOcupa);

	}

	else{

		mandarErrorAdiscordiador(tripulanteSock);

		log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", _tarea->nombreTarea);

	}
	unlock(oxigeno->mutex);
}


void consumirOxigeno(t_tarea* tarea, int* tripulanteSock){

}

/*
void generarComida(t_tarea* tarea, int* tripulanteSock){

	int bloquesAocupar = (int) ceil((float) tarea->parametro / (float) superBloque->block_size);

		if(bloquesLibres(bloquesAocupar)){

			log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",tarea->nombreTarea);

			if(verificarSiExiste(pathComida)){

				crearConfig(&configComida,pathComida);

				//actualizarEstructurasFile(comida, configComida);

				log_info(logImongo, "Los bloques que ocupa al momento son: %s \n", comida->bloquesQueOcupa);

				comida->cantidadBloques += bloquesAocupar;

				config_set_value(configComida,"BLOCK_COUNT",string_itoa(comida->cantidadBloques));
				config_set_value(configComida,"SIZE",string_itoa(comida->cantidadBloques*superBloque->block_size));
				config_save(configComida);

			}
			else{

				FILE* archivoComida = fopen(pathComida,"wb");
				fclose(archivoComida);

				crearConfig(&configComida,pathComida);

				config_set_value(configComida,"CARACTER_LLENADO","C");
				config_set_value(configComida,"BLOCKS","");
				config_set_value(configComida,"MD5_ARCHIVO","");
				config_set_value(configComida,"BLOCK_COUNT",string_itoa(bloquesAocupar));
				config_set_value(configComida,"SIZE",string_itoa(bloquesAocupar*superBloque->block_size));
				config_save(configComida);

				//actualizarEstructurasFile(comida, configComida);

			}


			int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);


			actualizarPosicionesFile(comida,posicionesQueOcupa,configComida,bloquesAocupar);


			guardarEnMemoriaSecundaria(tarea,posicionesQueOcupa,comida->caracterLlenado,bloquesAocupar);

			mandarOKAdiscordiador(tripulanteSock);

		}
		else{

			mandarErrorAdiscordiador(tripulanteSock);

			log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", tarea->nombreTarea);

		}

}
*/

void consumirComida(t_tarea* tarea, int* tripulanteSock){

}

/*
void generarBasura(t_tarea* tarea, int* tripulanteSock){

	int bloquesAocupar = (int) ceil((float) tarea->parametro / (float) superBloque->block_size);

			if(bloquesLibres(bloquesAocupar)){

				log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",tarea->nombreTarea);

				if(verificarSiExiste(pathBasura)){

					crearConfig(&configBasura,pathBasura);

					//actualizarEstructurasFile(basura, configBasura);

					log_info(logImongo, "Los bloques que ocupa al momento son: %s \n", basura->bloquesQueOcupa);

					basura->cantidadBloques += bloquesAocupar;

					config_set_value(configBasura,"BLOCK_COUNT",string_itoa(basura->cantidadBloques));
					config_set_value(configBasura,"SIZE",string_itoa(basura->cantidadBloques*superBloque->block_size));
					config_save(configBasura);

				}
				else{

					FILE* archivoBasura = fopen(pathBasura,"wb");
					fclose(archivoBasura);

					crearConfig(&configBasura,pathBasura);

					config_set_value(configBasura,"CARACTER_LLENADO","B");
					config_set_value(configBasura,"BLOCKS","");
					config_set_value(configBasura,"MD5_ARCHIVO","");
					config_set_value(configBasura,"BLOCK_COUNT",string_itoa(bloquesAocupar));
					config_set_value(configBasura,"SIZE",string_itoa(bloquesAocupar*superBloque->block_size));
					config_save(configBasura);

					//actualizarEstructurasFile(basura, configBasura);

				}


				int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);


				actualizarPosicionesFile(basura,posicionesQueOcupa,configBasura,bloquesAocupar);


				guardarEnMemoriaSecundaria(tarea,posicionesQueOcupa,basura->caracterLlenado,bloquesAocupar);

				//mandarOKAdiscordiador(tripulanteSock);

			}
			else{

				//mandarErrorAdiscordiador(tripulanteSock);

				log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", tarea->nombreTarea);

			}

}
*/

void descartarBasura(t_tarea* tarea, int* tripulanteSock){

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
