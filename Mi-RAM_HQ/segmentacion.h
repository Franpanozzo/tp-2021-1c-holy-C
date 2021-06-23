/*
 * segmentacion.h
 *
 *  Created on: 21 jun. 2021
 *      Author: utnso
 */

#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_
#include "memoria.h"


typedef struct {
	int idPatota;
	t_list* tablaDeSegmentos;
} t_tablaSegmentosPatota;

typedef struct {
	int indice;
	tipoEstructura tipo;
	int deslazamientoInicial;
	int bytesAlojados;
	int datoAdicional;  //ID EN LOS TRIPUES, EN LAS DEMAS -1
} t_info_segmento;

typedef struct {
	int inicio;
	int bytesSobrantes;

} t_lugarLibre;


t_list* tablasSegmentosPatotas;


int guardarPCBSeg(pcb*, char*);
t_tarea* guardarTCBSeg(tcb*, int);
t_tablaSegmentosPatota* buscarTablaDeSegmentosDePatota(int);
int asignarSegmentosEnTabla(void* , t_tablaSegmentosPatota* , tipoEstructura);
void insertar_en_memoria_seg(t_info_segmento*, void*);
int buscarSegmentoSegunAjuste(int);
t_info_segmento* crearSegmentoEnTabla(t_tablaSegmentosPatota, tipoEstructura);












#endif /* SEGMENTACION_H_ */
