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
	char* bloquesQueOcupa;
	char* caracterLlenado;
	char* md5_archivo;

}t_file;


typedef struct{

	char* puntoMontaje;
	int puerto;
	int tiempoSincronizacion;
	char* posicionesSabotaje;

}t_datosConfig;

typedef struct {
	t_config* config;
	t_file* file;
	pthread_mutex_t* mutex;
	char * path;
	bool configSeCreo;
}tarea;

pthread_mutex_t mutexSuperBloque;
pthread_mutex_t mutexMemoriaSecundaria;
pthread_mutex_t mutexBitMap;
pthread_mutex_t mutexEstructurasFile;
pthread_mutex_t mutexOxigeno;
pthread_mutex_t mutexComida;
pthread_mutex_t mutexBasura;
pthread_mutex_t mutexEstructuraOxigeno;
pthread_mutex_t mutexEstructuraComida;
pthread_mutex_t mutexEstructuraBasura;
pthread_t manejoTripulante;
pthread_t hiloSincronizador;
t_config* configImongo;
t_config* configSuperBloque;
t_config* configOxigeno;
t_config* configComida;
t_config* configBasura;
t_log* logImongo;
t_datosConfig* datosConfig;
t_superBloque* superBloque;
char* bitArray;
tarea* oxigeno;
tarea* comida;
tarea* basura;
char* memoriaSecundaria;
char* copiaMemoriaSecundaria;
char** tareas;
char* pathSuperBloque;
char* pathBloque;
char* pathOxigeno;
char* pathComida;
char* pathBasura;
char* pathFiles;
char* pathBitacora;


#endif


