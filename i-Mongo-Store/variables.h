#ifndef VARIABLES_H_
#define VARIABLES_H_


#include "i_mongo_store.h"


typedef struct{

	uint32_t block_size;
	uint32_t blocks;
	t_bitarray* bitmap;

}t_superBloque;

typedef struct{

	uint32_t tamanioArchivo;
	uint32_t cantidadBloques;
	t_list* bloquesQueOcupa;
	char caracterLlenado;
	char* md5_archivo;

}t_file;

typedef struct{

	char* puntoMontaje;
	int puerto;
	int tiempoSincronizacion;
	char* posicionesSabotaje;

}t_datosConfig;


t_config* config;
t_log* logImongo;
char** todasLasTareasIO;
t_datosConfig* datosConfig;
t_superBloque* superBloque;


#endif


