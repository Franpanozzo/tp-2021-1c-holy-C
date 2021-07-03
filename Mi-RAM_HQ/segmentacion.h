/*
 * segmentacion.h
 *
 *  Created on: 21 jun. 2021
 *      Author: utnso
 */

#ifndef SEGMENTACION_H_
#define SEGMENTACION_H_
#include "paginacion.h"


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
	int bytesAlojados;
} t_lugarLibre;


t_list* lugaresLibres;

t_list* tablasSegmentosPatotas;


int guardarPCBSeg(pcb*, char*);
t_tarea* guardarTCBSeg(tcb*, int);
t_tablaSegmentosPatota* buscarTablaDeSegmentosDePatota(int);
int asignarSegmentosEnTabla(void* , t_tablaSegmentosPatota* , tipoEstructura);
void insertar_en_memoria_seg(t_info_segmento*, void*);
int buscarSegmentoSegunAjuste(int);
t_info_segmento* crearSegmentoEnTabla(t_tablaSegmentosPatota*, tipoEstructura);
void expulsarTripulanteSeg(int ,int);
void borrarLugarLibre(t_lugarLibre*);
int actualizarTripulanteSeg(tcb* ,int);
void existenciaDeTablaSegParaPatota(t_tablaSegmentosPatota*);
t_info_segmento* buscarSegmentoTripulante(int, t_tablaSegmentosPatota*);
void* leer_memoria_seg(t_info_segmento*);
void chequearUltimoTripulanteSeg(t_tablaSegmentosPatota*);
int asignarSegmentosEnTabla(void*, t_tablaSegmentosPatota*, tipoEstructura);
bool tieneTripulantesSeg(t_tablaSegmentosPatota*);
void actualizarTripulanteEnMemSeg(tcb*, t_info_segmento*);
t_tarea* irABuscarSiguienteTareaSeg(t_tablaSegmentosPatota* ,tcb*);
t_tarea* asignarProxTareaSeg(int, int);
void dumpSeg();
void imprimirDatosSegmento(t_tablaSegmentosPatota*, FILE*);
void compactarMemoria();
t_info_segmento* encontrarSegmentoQueArrancaEn(int);
t_info_segmento* tieneInicioEnTabla(t_list*, int);
t_lugarLibre* primerLugarLibre();
t_lugarLibre* buscarLugarLibre(int);
void eliminarTablaPatota(t_tablaSegmentosPatota*);



























#endif /* SEGMENTACION_H_ */
