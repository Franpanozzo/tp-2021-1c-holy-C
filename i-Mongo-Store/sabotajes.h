#ifndef SABOTAJE_H_
#define SABOTAJE_H_

//#include "variables.h"
//#include "utils.h"
#include "i_mongo_store.h"


void buscarSabotaje();
bool haySabotajeCantBloquesEnSuperBloque();
void arreglarSabotajeCantBloquesEnSuperBloque();
bool haySabotajeBitmapEnSuperBloque();
void arreglarSabotajeBitmapEnSuperBloque();
bool haySabotajeSizeEnFile(tarea*);
void arreglarSabotajeSizeEnFile(tarea*);
bool haySabotajeCantBloquesEnFile(tarea*);
void arreglarSabotajeCantBloquesEnFile(tarea*);
bool haySabotajeBloquesEnFile(tarea*);
void arreglarSabotajeBloquesEnFile(tarea*);

//uint32_t max(long int, long int);
uint32_t cantBloquesEnBlocks();
uint32_t sizeSegunBlocks(tarea*);
uint32_t tamanioUltimoBloque(tarea*);
uint32_t cantBloquesSegunLista(tarea*);
t_list* obtenerBloquesOcupadosSegunFiles();
t_bitarray* crearBitmap();
char* convertirBitmapEnString(t_bitarray*);
void agregarBloquesOcupados(t_list*, t_config*);


#endif
