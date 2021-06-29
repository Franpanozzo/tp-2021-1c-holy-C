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
	pathOxigeno = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	pathComida = crearDestinoApartirDeRaiz("Files/Comida.ims");
	pathBasura = crearDestinoApartirDeRaiz("Files/Basura.ims");
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
	oxigeno = malloc(tamanio);
	comida = malloc(tamanio);
	basura = malloc(tamanio);
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

	copiaMemoriaSecundaria= malloc(size);
	memcpy(copiaMemoriaSecundaria,memoriaSecundaria, size);

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

	for(int i=0; i<superBloque->blocks;i++){

	flag += !bitarray_test_bit(superBloque->bitmap,i);


		if(flag == bloquesAocupar){

		return true;

		}

	}

	return false ;

}//


bool verificarSiExiste(char* pathArchivo){

	return access(pathArchivo,F_OK) != -1;

}//


int* obtenerArrayDePosiciones(int bloquesAocupar){

	int flag = 0;

	int bloquesLibres = 0;

	int* cadaBloqueAocupar = malloc(sizeof(int) * bloquesAocupar);

	for(int i=0; i<superBloque->blocks;i++){

		flag = bitarray_test_bit(superBloque->bitmap,i);

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

    	int valor =  bitarray_test_bit(superBloque->bitmap, i);

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

  config_set_value(configSuperBloque,"BITMAP",bitmap);

  config_save(configSuperBloque);

  free(bitmap);


}

void actualizarEstructurasFile(t_file* file){

	file->bloquesQueOcupa = config_get_string_value(configOxigeno,"BLOCKS");
	file->cantidadBloques = config_get_int_value(configOxigeno,"BLOCK_COUNT");
	file->caracterLlenado = config_get_string_value(configOxigeno,"CARACTER_LLENADO");
	file->md5_archivo = config_get_string_value(configOxigeno,"MD5_ARCHIVO");
	file->tamanioArchivo = config_get_int_value(configOxigeno,"SIZE");
}


void actualizarPosicionesFile(t_file* archivo, int* arrayDePosiciones, t_config* config, int bloquesAocupar){

	char* bloquesQueTenia = archivo->bloquesQueOcupa;

	printf("%s \n", bloquesQueTenia);

	int tamanio = string_length(bloquesQueTenia);

	printf("%d \n", tamanio);

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

	printf("%s \n", bloquesActuales);

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

		printf("%s \n", bloquesActuales);
		string_append(&bloquesActuales,string_itoa(*(arrayDePosiciones + i)));
		printf("%s \n", bloquesActuales);
		i++;

	}
	string_append(&bloquesActuales,"]");
	printf("%s \n", bloquesActuales);

	archivo->bloquesQueOcupa = bloquesActuales;
	archivo->cantidadBloques += bloquesAocupar;
	archivo->tamanioArchivo = archivo->cantidadBloques * superBloque->block_size;

	config_set_value(config,"BLOCK_COUNT",archivo->bloquesQueOcupa);
	config_set_value(config,"SIZE",string_itoa(archivo->tamanioArchivo));
	config_set_value(config,"BLOCKS",string_itoa(archivo->cantidadBloques));
	config_save(config);

	free(bloquesQueTenia);

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

	sleep(tiempoEspera);

	int size = superBloque->block_size * superBloque->blocks;

	memcpy(memoriaSecundaria,copiaMemoriaSecundaria,size);

	msync(memoriaSecundaria,size,MS_SYNC);
}


void actualizarBitArray(int* posicionesQueOcupa, int bloquesAocupar){

	for(int i=0; i<bloquesAocupar; i++){
		bitarray_set_bit(superBloque->bitmap, posicionesQueOcupa[i]);
	}

	actualizarStringBitMap(bloquesAocupar);

}


void guardarEnMemoriaSecundaria(t_tarea* tarea, int* posicionesQueOcupa,char* caracterLlenado, int bloquesAocupar){

	int flag = tarea->parametro;

	printf("flag = %d \n", flag);

	char caracter = caracterLlenado[0];

	printf("caracter = %c \n", caracter);

	char** palabraAguardar = malloc(sizeof(char*)*bloquesAocupar);

	for(int i = 0; i < bloquesAocupar;i++){

		palabraAguardar[i] = string_repeat(caracter,min(superBloque->block_size,flag));
		printf("palabra = %s \n", palabraAguardar[i]);
		flag -= superBloque->block_size;
		printf("flag = %d \n", flag);
	}

	for(int i=0; i<bloquesAocupar; i++){
		int offset = posicionesQueOcupa[i] * superBloque->block_size;
		printf("offset = %d \n", offset);
		memcpy(copiaMemoriaSecundaria + offset ,palabraAguardar[i], superBloque->block_size);
	}

	actualizarBitArray(posicionesQueOcupa, bloquesAocupar);


}


void generarOxigeno(t_tarea* tarea, int* tripulanteSock){

	int bloquesAocupar = (int) ceil((float) tarea->parametro / (float) superBloque->block_size);

	if(bloquesLibres(bloquesAocupar)){

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",tarea->nombreTarea);

		if(verificarSiExiste(pathOxigeno)){

			crearConfig(&configOxigeno,pathOxigeno);

			actualizarEstructurasFile(oxigeno);

			bloquesAocupar += oxigeno->cantidadBloques;

		}
		else{

			FILE* archivoOxigeno = fopen(pathOxigeno,"wb");
			fclose(archivoOxigeno);

			crearConfig(&configOxigeno,pathOxigeno);

			config_set_value(configOxigeno,"CARACTER_LLENADO","O");
			config_set_value(configOxigeno,"BLOCKS","");
			config_set_value(configOxigeno,"MD5_ARCHIVO","");

		}

		config_set_value(configOxigeno,"BLOCK_COUNT",string_itoa(bloquesAocupar));
		config_set_value(configOxigeno,"SIZE",string_itoa(bloquesAocupar*superBloque->block_size));
		config_save(configOxigeno);

		actualizarEstructurasFile(oxigeno);

		int* posicionesQueOcupa = obtenerArrayDePosiciones(bloquesAocupar);


		actualizarPosicionesFile(oxigeno,posicionesQueOcupa,configOxigeno,bloquesAocupar);


		guardarEnMemoriaSecundaria(tarea,posicionesQueOcupa,oxigeno->caracterLlenado,bloquesAocupar);

		//mandarOKAdiscordiador(tripulanteSock);

	}
	else{

		//mandarErrorAdiscordiador(tripulanteSock);

		log_info(logImongo,"No hay mas espacio en el disco para la tarea %s", tarea->nombreTarea);

	}

}


void consumirOxigeno(t_tarea* tarea, int* tripulanteSock){

}


void generarComida(t_tarea* tarea, int* tripulanteSock){

}


void consumirComida(t_tarea* tarea, int* tripulanteSock){

}


void generarBasura(t_tarea* tarea, int* tripulanteSock){

}


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

	liberarStructTareas(oxigeno);
	liberarStructTareas(comida);
	liberarStructTareas(basura);

}


void liberarTareas(){

	for(int i=0; i<6; i++){
		free(tareas[i]);
	}
	free(tareas);
}//
