#ifndef VARIABLES_H_
#define VARIABLES_H_


#include "i_mongo_store.h"


typedef struct{

	uint32_t block_size;
	uint32_t blocks;
	t_bitarray* bitmap;
	char* path;

}t_superBloque;


typedef struct{

	char* puntoMontaje;
	int puerto;
	int tiempoSincronizacion;
	char* posicionesSabotaje;

}t_datosConfig;


typedef struct{

	uint32_t tamanioArchivo;
	t_list* bloques;
	char* caracterLlenado;
	char* md5_archivo;
	pthread_mutex_t mutex;
	char * path;

}t_file2;


typedef struct{

	uint32_t tamanioArchivo;
	t_list* bloques;
	pthread_mutex_t mutex;
	char * path;

}t_bitacora_tripulante;


puertoEIP* puertoEIPDisc;

pthread_mutex_t mutexMemoriaSecundaria;
pthread_mutex_t mutexBitMap;
pthread_mutex_t mutexEstructurasFile;
pthread_mutex_t mutexExisteBitacora;
pthread_mutex_t mutexCantEscriturasPendientes;
pthread_mutex_t mutexHaySabotaje;
pthread_t manejoTripulante;
pthread_t hiloSincronizador;

t_config* configImongo;
t_log* logImongo;
t_datosConfig* datosConfig;
char* bitArray;
char** tareas;

int cantEscriturasPendientes;
bool haySabotaje;

t_superBloque* superBloque;
t_file2* oxigeno;
t_file2* comida;
t_file2* basura;
t_dictionary * bitacoras;

sem_t sabotajeResuelto;
sem_t semTarea;

char* memoriaSecundaria;
char* copiaMemoriaSecundaria;
char* pathFiles;
char* pathBitacoras;
char* pathBloque;

t_list* listaPosicionesSabotaje;
int proximoPosSabotaje;

#endif


