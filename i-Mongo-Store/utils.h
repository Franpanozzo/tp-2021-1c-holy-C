
#ifndef UTILS_H_
#define UTILS_H_

#include "variables.h"
#include "i_mongo_store.h"


void crearConfig();
char * pathLog();
void crearTareasIO();
void cargarConfiguracion();
void liberarConfiguracion();
int indiceTarea(t_tarea*);
char* crearDestinoApartirDeRaiz(char*);
bool validarExistenciaFileSystem(char*, char*,char*);
void setearFiles(t_file*, char*);


#endif
