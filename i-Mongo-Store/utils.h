#ifndef UTILS_H_
#define UTILS_H_

#include "variables.h"
#include "i_mongo_store.h"


void crearConfig();
char * pathLog();
void cargarConfiguracion();
void liberarConfiguracion();
int indiceTarea(t_tarea*);
char* crearDestinoApartirDeRaiz(char*);
bool validarExistenciaFileSystem(char*, char*,char*);
void setearFile(t_file*, char*);
void setearTodosLosFiles();
void crearMemoria(int);
void generarOxigeno(t_tarea*);
void consumirOxigeno(t_tarea*);
void generarComida(t_tarea*);
void consumirComida(t_tarea*);
void generarBasura(t_tarea*);
void descartarBasura(t_tarea*);


#endif
