/*
 * memoria.h
 *
 *  Created on: 12 jun. 2021
 *      Author: utnso
 */

#ifndef PAGINACION_H_
#define PAGINACION_H_

#define MEM_PPAL 0
#define MEM_VIRT 1

#define PAGINACION 0;
#define SEGMENTACION 1;

#define FRAME_INVALIDO -1

#define TAM_TCB 21
#define TAM_PCB 8

#include "estructuras.h"
#include "segmentacion.h"
#include "mi_ram_hq.h"


typedef struct {
	int tamanioMemoria;
	char* esquemaMemoria;
	int tamanioPagina;
	int tamanioSwap;
	char* pathSwap;
	char* algoritmoReemplazo;
	char* criterioSeleccion;
	int puerto;

} t_configRam;

typedef struct {
	int idPatota;
	t_list* tablaDePaginas; //Aca adentro van a estar los t_info_pagina
} t_tablaPaginasPatota;

typedef struct {
    int indice;
    //uint32_t frame_m_virtual;
    int frame_m_ppal;
    int bytesDisponibles; // COMO IDENTIFICAMOS SI ES UN PCB, TCB O TAREAS ??
    t_list* estructurasAlojadas;
    //double tiempo_uso;
    //otros datos..
} t_info_pagina;

typedef struct {
	int indice;
	int desplazamientoInicial;
	int bytesAlojados;
	tipoEstructura tipo;
	int datoAdicional;  //PARA LOS TRIPULANTES ES EL ID, PARA LAS TAREAS SI LA PAGINA ES LA ULTIMA QUE CONTIENE TAREAS, PARA PCB -1
} t_alojado;


t_log* logMemoria;
t_configRam configRam;

void* memoria_principal;
t_bitarray* frames_ocupados_ppal;
int cant_frames_ppal;

t_list* tablasPaginasPatotas;

pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexEscribirMemoria;
pthread_mutex_t mutexBuscarLugarLibre;
pthread_mutex_t mutexTablasSegmentos;
pthread_mutex_t mutexTablasPaginas;
pthread_mutex_t mutexExpulsionTripulante;
pthread_mutex_t mutexTablaSegmentosPatota;
pthread_mutex_t mutexTablaPaginasPatota;
pthread_mutex_t mutexBitarray;
pthread_mutex_t mutexAlojados;




bool get_frame(int , int);
void set_frame(int , int);
void* leer_memoria_pag(int,int);
int insertar_en_memoria_pag(t_info_pagina*, void*, int, int*, tipoEstructura, int, int*);
void agregarEstructAdminTipo(t_info_pagina*, int, int, tipoEstructura,int);
uint32_t buscar_frame_disponible(int );
void* buscar_pagina(t_info_pagina* );
t_info_pagina* crearPaginaEnTabla(t_tablaPaginasPatota* ,tipoEstructura);
int asignarPaginasEnTabla(void* , t_tablaPaginasPatota* , tipoEstructura );
t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int );
t_info_pagina* buscarUltimaPaginaDisponible(t_tablaPaginasPatota* );
t_tarea* guardarTCBPag(tcb*, int);
int guardarPCBPag(pcb*, char*);
uint32_t estimarDLTareasPag();
uint32_t buscarInicioDLTareas(t_tablaPaginasPatota* );
t_tarea* irABuscarSiguienteTareaPag(t_tablaPaginasPatota* , tcb* );
bool tieneEstructuraAlojada(t_list* , tipoEstructura);
bool tieneTripulanteAlojado(t_list* , int);
t_alojado* obtenerAlojadoPagina(t_list* , int);
int actualizarTripulanteEnMemPag(t_tablaPaginasPatota* , tcb*);
int frameTotalmenteLibre(int );
t_list* paginasConTripu(t_list*, uint32_t );
int sobreescribirTripu(t_list* , tcb* );
int actualizarTripulantePag(tcb* , int);
tcb* obtenerTripulante(t_tablaPaginasPatota* ,int );
t_tarea* asignarProxTareaPag(int , int);
t_list_iterator* iterarHastaIndice(t_list*, int);
void existenciaDeTablaParaPatota(t_tablaPaginasPatota*);
void chequearUltimoTripulante(t_tablaPaginasPatota*);
bool tieneTripulantesPag(t_tablaPaginasPatota*);
t_tablaPaginasPatota* patotaConFrame(int);
t_info_pagina* paginaConFrame(int ,t_tablaPaginasPatota*);
void expulsarTripulantePag(int ,int);
void dumpPag();















#endif /* PAGINACION_H_ */




