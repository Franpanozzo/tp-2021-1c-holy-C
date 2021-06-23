#include "utils.h"


void crearConfig(){

	config  = config_create("/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/i_mongo_store.config");

	if(config == NULL){

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


int indiceTarea(t_tarea* tarea){

	int i = 0;

	while(todasLasTareasIO[i] != NULL){

		if(strcmp(todasLasTareasIO[i],tarea->nombreTarea) == 0){
			return i;
		}

		i++;
	}

	return -1;
}


void crearTareasIO(){
	todasLasTareasIO = malloc(sizeof(char*) * 6);

	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");
}


void cargarConfiguracion(){

	datosConfig = malloc(sizeof(t_datosConfig));
	superBloque = malloc(sizeof(t_superBloque));

	superBloque->block_size = (uint32_t) config_get_int_value(config,"BLOCK_SIZE");
	superBloque->blocks = (uint32_t)config_get_int_value(config,"BLOCKS");
	int sizeBitArray = superBloque->block_size * superBloque->blocks / 8;
	bitArray = malloc(sizeBitArray);
	superBloque->bitmap = bitarray_create_with_mode(bitArray,sizeBitArray ,MSB_FIRST);

	for(int i=0; i<sizeBitArray;i++){

		bitarray_clean_bit(superBloque->bitmap, i);
		//int valor =  bitarray_test_bit(superBloque->bitmap, i);
		//printf("%d ", valor);
	}

    log_info(logImongo,"Se inicializo un bitmap con %d posiciones", bitarray_get_max_bit(superBloque->bitmap));

	datosConfig->puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");
	datosConfig->puerto = (uint32_t)config_get_int_value(config,"PUERTO");
	datosConfig->tiempoSincronizacion = (uint32_t)config_get_int_value(config,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = config_get_string_value(config,"POSICIONES_SABOTAJE");

	char* pathOxigeno = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	char* pathComida = crearDestinoApartirDeRaiz("Files/Comida.ims");
	char* pathBasura = crearDestinoApartirDeRaiz("Files/Basura.ims");

	setearFiles(oxigeno,pathOxigeno);
	setearFiles(comida,pathComida);
	setearFiles(basura,pathBasura);

}

char* crearDestinoApartirDeRaiz(char* destino){

	char* raiz = string_new();

	string_append(&raiz, datosConfig->puntoMontaje);
	string_append(&raiz, "/");
	string_append(&raiz, destino);

	return raiz;

}


bool validarExistenciaFileSystem(char* superBloque, char* blocks, char* raiz){


	return (access(superBloque, F_OK ) != -1) && (access(blocks, F_OK ) != -1) && (access(raiz, F_OK ) != -1);

}

void setearFiles(t_file* archivoFile, char* path){


	archivoFile->tamanioArchivo = archivoFile->cantidadBloques * superBloque->block_size;
	archivoFile->cantidadBloques = list_size(archivoFile->bloquesQueOcupa);
	//archivoFile->md5_archivo = FALTA EL MD5 del archivo

}


void liberarConfiguracion(){

	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);

}
