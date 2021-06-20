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

	superBloque->block_size = config_get_int_value(config,"BLOCK_SIZE");
	superBloque->blocks = config_get_int_value(config,"BLOCKS");
	datosConfig->puntoMontaje = strdup(config_get_string_value(config,"PUNTO_MONTAJE"));
	datosConfig->puerto = config_get_int_value(config,"PUERTO");
	datosConfig->tiempoSincronizacion = config_get_int_value(config,"TIEMPO_SINCRONIZACION");
	datosConfig->posicionesSabotaje = strdup(config_get_string_value(config,"POSICIONES_SABOTAJE"));

}

void liberarConfiguracion(){

	free(datosConfig->puntoMontaje);
	free(datosConfig->posicionesSabotaje);
	free(datosConfig);

}
