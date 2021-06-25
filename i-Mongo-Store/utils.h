#ifndef UTILS_H_
#define UTILS_H_


#include "variables.h"
#include "i_mongo_store.h"


void crearConfigImongo();
void crearConfigSuperBloque();
void crearConfigOxigenoIMS();
void crearConfigComidaIMS();
void crearConfigBasuraIMS();
char * pathLog();
void cargarConfiguracion();
void liberarConfiguracion();
int indiceTarea(t_tarea*);
char* crearDestinoApartirDeRaiz(char*);
bool validarExistenciaFileSystem(char*, char*,char*);
void crearMemoria(int);
void generarOxigeno(t_tarea*,int*);
void consumirOxigeno(t_tarea*,int*);
void generarComida(t_tarea*,int*);
void consumirComida(t_tarea*,int*);
void generarBasura(t_tarea*,int*);
void descartarBasura(t_tarea*,int*);
void mandarErrorAdiscordiador(int*);
void liberarTodosLosConfig();


#endif
