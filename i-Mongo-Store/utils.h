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
int ultimoBloqueDeLa(tarea*);
void generarTarea(tarea*, t_tarea*, int*);
void actualizarEstructurasFile(t_file*, t_config*);
void descartarBasura(t_tarea*,int*);
void mandarErrorAdiscordiador(int*);
int* obtenerArrayDePosiciones(int);
bool bloquesLibres(int);
void actualizarPosicionesFile(t_file*, int*, t_config*,int);
int min(int ,int );
int max(int, int);
void actualizarStringBitMap();
void cargarPaths();
void detallesArchivo(int);
bool verificarSiExiste(char*);
void mallocTareas();
void liberarStructTareas(t_file*);
void liberarTodosLosStructTareas();
void sincronizarMemoriaSecundaria();
void iniciarMutex();
void actualizarMD5(tarea* );
char* datosBloque(int);
int fragmentacionDe(int);
int ultimoBloqueDeLa(tarea*);
t_info*  nuevosBloquesAocupar(tarea*,t_tarea*);
void guardarEnMemoriaSecundaria(int*,char* , int, int);
bool alcanzanCaracteresParaConsumir(int,int);
void consumirTarea(tarea*, t_tarea*, int*);
void limpiarBitArray(int*, int);
void crearConfigTarea(tarea*);
void inicializarTarea(tarea*, int, int);
int saberUltimoBloqueTarea(tarea*);
char* suprimirCaracteres(int, char);

#endif
