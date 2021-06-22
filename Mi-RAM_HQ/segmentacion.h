/*
 * segmentacion.h
 *
 *  Created on: 21 jun. 2021
 *      Author: utnso
 */

#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_
#include "memoria.h"


int guardarPCBSeg(pcb*, char*);
t_tarea* guardarTCBSeg(tcb*, int);

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


#endif /* SEGMENTACION_H_ */
