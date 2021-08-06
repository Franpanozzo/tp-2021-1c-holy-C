#ifndef SABOTAJE_H_
#define SABOTAJE_H_

#include "i_mongo_store.h"


void fsck();
bool haySabotajeCantBloquesEnSuperBloque();
void arreglarSabotajeCantBloquesEnSuperBloque();
bool haySabotajeBitmapEnSuperBloque();
void arreglarSabotajeBitmapEnSuperBloque();
bool haySabotajeSizeEnFile(t_file*);
void arreglarSabotajeSizeEnFile(t_file*);
bool haySabotajeCantBloquesEnFile(t_file*);
void arreglarSabotajeCantBloquesEnFile(t_file*);
bool haySabotajeBloquesEnFile(t_file*);
void arreglarSabotajeBloquesEnFile(t_file*);
bool haySabotajeBlocksBloqueExtra(t_file*);
bool haySabotajeBlocksBloqueExtra2(t_file*);
void arreglarSabotajeBlocksBloqueExtra(t_file*);

uint32_t cantBloquesEnBlocks();
uint32_t sizeSegunBlocks(t_file*);
uint32_t tamanioUltimoBloque(t_file*);
uint32_t cantBloquesSegunLista(t_file*);
bool existeBitacoraTripulante(int);
void cargarBitmap(char*,  t_bitarray*);
void agregarBloquesOcupados(t_list*, t_config*);
void sabotajesFile(t_file*);



#endif
