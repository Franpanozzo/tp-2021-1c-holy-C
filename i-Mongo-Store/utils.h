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
void actualizarEstructurasFile(tarea*);
void mandarErrorAdiscordiador(int*);
int* obtenerArrayDePosiciones(int);
bool bloquesLibres(int);
void actualizarPosicionesFile(tarea*, int*,int);
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
t_list* convertirEnLista(char** arrayValores);
char* convertirEnString(t_list* listaEnteros);
t_desplazamiento* deserializarDesplazamiento(void*);
t_avisoTarea* deserializarAvisoTarea(void*);
int deserializarAvisoSabotaje(void*);
void escribirEnBitacora(char*, int, int*);
char * obtenerMD5(t_list * );
char * reconstruirArchivo(t_list *);
void guardarStringEnMemoriaSecundaria(int*, char*, int);



#endif
