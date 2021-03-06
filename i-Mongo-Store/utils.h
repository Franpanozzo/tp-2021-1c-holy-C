#ifndef UTILS_H_
#define UTILS_H_


//#include "variables.h"
#include "i_mongo_store.h"


void crearConfig(t_config**, char*);
char * pathLog();
char* crearDestinoApartirDeRaiz(char*);
void cargarPaths();
void cargarDatosConfig();
void iniciarMutex();
void liberarEstructuraDatosConfig();
int indiceTarea(t_tarea*);
void asignarTareas();
int leerHaySabotaje();
void modificarHaySabotaje(bool);
int leerCantEscriturasPendientes();
void modificarCantEscriturasPendientes(int);

int min(int ,int );
int max(int, int);
void detallesArchivo(int);
bool verificarSiExiste(char*);
bool validarExistenciaFileSystem(char*, char*, char*);

void crearMemoria(int);
void sincronizarMemoriaSecundaria();
char* reconstruirArchivo(t_list*);
char* obtenerMD5(t_list*);

t_list* convertirEnLista(char**);
char* convertirListaEnString(t_list*);
char* convertirBitmapEnString(t_bitarray*);

t_desplazamiento* deserializarDesplazamiento(void*);
t_avisoTarea* deserializarAvisoTarea(void*);
uint32_t deserializarID(void*);

void crearBitacora(char*);
char* pathBitacoraTripulante(char*);

void generarTarea(t_file*, uint32_t);
void consumirTarea(t_file*, uint32_t);
void escribirBitacora(char*, char*);
uint32_t fragmentacion(uint32_t);
void escribirBloque(uint32_t, char*, uint32_t);
void consumirBloque(uint32_t, uint32_t, uint32_t);
t_list* buscarBloques(uint32_t);
void ocuparBloque(uint32_t);
void liberarBloque(uint32_t);

void actualizarFile(t_file*);
void actualizarBitacora(t_bitacora_tripulante*);
void actualizarSuperBloque();

t_list* listaCoordenadasSabotaje();
void sabotaje(int);


#endif
