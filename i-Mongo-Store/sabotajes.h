#ifndef SABOTAJE_H_
#define SABOTAJE_H_

#include "i_mongo_store.h"


void fsck();
bool haySabotajeCantBloquesEnSuperBloque();
void arreglarSabotajeCantBloquesEnSuperBloque();
bool haySabotajeBitmapEnSuperBloque();
void arreglarSabotajeBitmapEnSuperBloque();
bool haySabotajeSizeEnFile(t_file2*);
void arreglarSabotajeSizeEnFile(t_file2*);
bool haySabotajeCantBloquesEnFile(t_file2*);
void arreglarSabotajeCantBloquesEnFile(t_file2*);
bool haySabotajeBloquesEnFile(t_file2*);
void arreglarSabotajeBloquesEnFile(t_file2*);

uint32_t cantBloquesEnBlocks();
uint32_t sizeSegunBlocks(t_file2*);
uint32_t tamanioUltimoBloque(t_file2*);
uint32_t cantBloquesSegunLista(t_file2*);
bool existeBitacoraTripulante(int);
t_bitarray* crearBitmap();
void agregarBloquesOcupados(t_list*, t_config*);
void sabotajesFile(t_file2*);



#endif
