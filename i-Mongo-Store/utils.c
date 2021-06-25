#include "utils.h"


void crearConfig(t_config* config, char* path){

	config  = config_create(path);

	if(config == NULL){

		log_error(logImongo, "La ruta es incorrecta ");

		exit(1);
	}
}

void cargarTodosLosConfig(){

	crearConfig(configImongo,"/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/i_mongo_store.config");
	crearConfig(configSuperBloque,pathSuperBloque);
	crearConfig(configOxigeno,pathOxigeno);
	crearConfig(configComida,pathComida);
	crearConfig(configBasura,pathBasura);

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
	tareas = malloc(sizeof(char*) * 3);

	tareas[0] = strdup("GENERAR_OXIGENO");
	tareas[1] = strdup("CONSUMIR_OXIGENO");
	tareas[2] = strdup("GENERAR_COMIDA");
	tareas[3] = strdup("CONSUMIR_COMIDA");
	tareas[4] = strdup("GENERAR_BASURA");
	tareas[5] = strdup("DESCARTAR_BASURA");
}


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


void cargarConfiguracion(){

	datosConfig = malloc(sizeof(t_datosConfig));
	datosConfig->puntoMontaje = config_get_string_value(configImongo,"PUNTO_MONTAJE");
	datosConfig->puerto = (uint32_t)config_get_int_value(configImongo,"PUERTO");
	datosConfig->tiempoSincronizacion = (uint32_t)config_get_int_value(configImongo,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = config_get_string_value(configImongo,"POSICIONES_SABOTAJE");

	pathSuperBloque = crearDestinoApartirDeRaiz("SuperBloque.ims");
	pathBloque = crearDestinoApartirDeRaiz("Blocks.ims");
	pathFiles = crearDestinoApartirDeRaiz("Files");
	pathOxigeno = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	pathComida = crearDestinoApartirDeRaiz("Files/Comida.ims");
	pathBasura = crearDestinoApartirDeRaiz("Files/Basura.ims");
	pathBitacora = crearDestinoApartirDeRaiz("Files/Bitacora");

}


bool validarExistenciaFileSystem(char* superBloque, char* blocks, char* raiz){

	return (access(superBloque, F_OK ) != -1) && (access(blocks, F_OK ) != -1) && (access(raiz, F_OK ) != -1);

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

}

/*
void sincronizarMemoria(void* dato){

	int size = superBloque->block_size * superBloque->blocks;

	char copiaMemoriaSecundaria= malloc(size);
	memcpy(copiaMemoriaSecundaria,memoriaSecundaria, size);

	char* datoAingresar = (char*) dato;

	int sizeDato = strlen(datoAingresar);

	memcpy(copiaMemoriaSecundaria,datoAingresar,sizeDato);

	memcpy(memoriaSecundaria,copiaMemoriaSecundaria,size);

	msync(memoriaSecundaria,size,MS_SYNC);
}
*/

void mandarErrorAdiscordiador(int* tripulanteSock){

	char* error = strdup("ERROR");

	t_paquete* paquete = armarPaqueteCon(error,STRING);

	enviarPaquete(paquete,*tripulanteSock);
}

void mandarOKAdiscordiador(int* tripulanteSock){

	char* error = strdup("OK");

	t_paquete* paquete = armarPaqueteCon(error,STRING);

	enviarPaquete(paquete,*tripulanteSock);
}

int bloquesLibres(){

	int flag = 0;

	for(int i=0; i<superBloque->blocks;i++){

	flag += bitarray_test_bit(superBloque->bitmap,i);

	}

	return superBloque->blocks - flag;
}


bool verificarSiExiste(char* nombreArchivo){

	return access(nombreArchivo,F_OK) != 1;

}


void generarOxigeno(t_tarea* tarea, int* tripulanteSock){

	int bloquesAocupar = (int) ceil(tarea->parametro / superBloque->block_size);

	int bloques = bloquesLibres(bloquesAocupar);

	if(bloquesAocupar >= bloques){

		log_info(logImongo,"Se comprobó que hay espacio en el disco para la tarea %s",tarea->nombreTarea);

		if(verificarSiExiste(pathOxigeno)){

			oxigeno->bloquesQueOcupa = config_get_string_value(configOxigeno,"BLOCKS");
			oxigeno->cantidadBloques = config_get_int_value(configOxigeno,"BLOCK_COUNT");
			oxigeno->caracterLlenado = config_get_string_value(configOxigeno,"CARACTER_LLENADO");
			oxigeno->md5_archivo = config_get_string_value(configOxigeno,"MD5_ARCHIVO");
			oxigeno->tamanioArchivo = config_get_int_value(configOxigeno,"SIZE");

			bloquesAocupar += oxigeno->cantidadBloques;



		}
		else{

			config_set_value(configOxigeno,"CARACTER_LLENADO","O");


		}

		config_set_value(configOxigeno,"BLOCK_COUNT",string_itoa(bloquesAocupar));
		config_set_value(configOxigeno,"SIZE",string_itoa(bloquesAocupar*superBloque->block_size));
		config_save(configOxigeno);


	}
	else{

		mandarErrorAdiscordiador(tripulanteSock);

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

void liberarTodosLosConfig(){

	config_destroy(configImongo);
	config_destroy(configOxigeno);
	config_destroy(configComida);
	config_destroy(configBasura);
	config_destroy(configSuperBloque);
}

void liberarConfiguracion(){

	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);

}


void liberarTareas(){

	for(int i=0; i<6; i++){
		free(tareas[i]);
	}
	free(tareas);
}
