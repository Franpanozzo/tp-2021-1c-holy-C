#ifndef UTILS_H_
#define UTILS_H_


#include "variables.h"
#include "i_mongo_store.h"


void crearConfig(t_config**, char*);
char * pathLog();
void cargarDatosConfig();
void liberarConfiguracion();
int indiceTarea(t_tarea*);
char* crearDestinoApartirDeRaiz(char*);
bool validarExistenciaFileSystem(char*, char*,char*);
void crearMemoria(int);
void generarOxigeno(t_tarea*,int*);
void consumirOxigeno(t_tarea*,int*);
void generarTarea(tarea*, t_tarea*, int*);
void generarComida(t_tarea*,int*);
void consumirComida(t_tarea*,int*);
void generarBasura(t_tarea*,int*);
void actualizarEstructurasFile(t_file*, t_config*, pthread_mutex_t *);
void descartarBasura(t_tarea*,int*);
void mandarErrorAdiscordiador(int*);
int* obtenerArrayDePosiciones(int);
bool bloquesLibres(int);
void actualizarPosicionesFile(t_file*, int*, t_config*,int);
int min(int ,int );
void actualizarStringBitMap();
void cargarPaths();
void detallesArchivo(int);
bool verificarSiExiste(char*);
void mallocTareas();
void liberarStructTareas(t_file*);
void liberarTodosLosStructTareas();
void sincronizarMemoriaSecundaria();
void iniciarMutex();


#endif
