#ifndef UTILS_H_
#define UTILS_H_


//#include "variables.h"
#include "i_mongo_store.h"


void crearConfig(t_config**, char*);
char * pathLog();
char* crearDestinoApartirDeRaiz(char*);
void crearMemoria(int);
int min(int ,int );
int max(int, int);
void cargarPaths();
void detallesArchivo(int);
bool verificarSiExiste(char*);
void sincronizarMemoriaSecundaria();
void iniciarMutex();
void actualizarMD5(tarea* );
t_list* listaCoordenadasSabotaje();
void sabotaje(int);
char* pathBitacoraTripulante(int);
bool existeBitacoraTripulante(int);



#endif
