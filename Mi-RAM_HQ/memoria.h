/*
 * memoria.h
 *
 *  Created on: 12 jun. 2021
 *      Author: utnso
 */

#ifndef MEMORIA_H_
#define MEMORIA_H_

#define MEM_PPAL 0
#define MEM_VIRT 1

#define PAGINACION 0;
#define SEGMENTACION 1;

#define FRAME_INVALIDO 2147483646

#define TAM_TCB 21
#define TAM_PCB 8

#include "estructuras.h"
//#include "mi_ram_hq.h"


typedef struct {
	int tamanioMemoria;
	char* esquemaMemoria;
	int tamanioPagina;
	int tamanioSwap;
	char* pathSwap;
	char* algoritmoReemplazo;
	char* criterioSeeleccion;
	int puerto;

} t_configRam;

typedef enum {
	PCB,
	TCB,
	TAREAS
} tipoEstructura;

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
	int desplazamientoInicial;
	int bytesAlojados;
	tipoEstructura tipo;
	int datoAdicional;  //PARA LOS TRIPULANTES ES EL ID, PARA LAS TAREAS SI LA PAGINA ES LA ULTIMA QUE CONTIENE TAREAS, PARA PCB -1
} t_alojado;


typedef struct {
	int idPatota;
	t_list* tablaDeSegmentos;
} t_tablaSegmentosPatota;

typedef struct {
	tipoEstructura tipo;
	int deslazamientoInicial;
	int bytesAlojados;
	//int idTripu  //ESTE EN LAS DEMAS VARIABLES -1
} t_info_segmento;


t_log* logMemoria;
t_configRam configRam;

void* memoria_principal;
t_bitarray* frames_ocupados_ppal;
int cant_frames_ppal;

t_list* tablasPaginasPatotas;

pthread_mutex_t mutexMemoria;
pthread_mutex_t mutexEscribirMemoria;


void cargar_configuracion();
bool get_frame(int , int);
void set_frame(int , int);
char* asignar_bytes(int );
void iniciarMemoria();
void* leer_memoria(int,int);
int insertar_en_memoria(t_info_pagina*, void*, int, int*, tipoEstructura, int);
void agregarEstructAdminTipo(t_info_pagina*, int, int, tipoEstructura,int);
uint32_t buscar_frame_disponible(int );
void* buscar_pagina(t_info_pagina* );
t_info_pagina* crearPaginaEnTabla(t_tablaPaginasPatota* ,tipoEstructura);
int asignarPaginasEnTabla(void* , t_tablaPaginasPatota* , tipoEstructura );
t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int );
t_info_pagina* buscarUltimaPaginaDisponible(t_tablaPaginasPatota* );
void* meterEnBuffer(void* , tipoEstructura , int*, int*);
int guardarPCB(pcb*, char*);
t_tarea* guardarTCB(tcb*, int);
t_tarea* guardarTCBPag(tcb*, int);
int guardarPCBPag(pcb*, char*);
uint32_t estimarDLTareas();
int guardarPCBSeg(pcb*, char*);
t_tarea* guardarTCBSeg(tcb*, int);
uint32_t buscarInicioDLTareas(t_tablaPaginasPatota* );
t_tarea* irABuscarSiguienteTarea(t_tablaPaginasPatota* , tcb* );
bool tieneEstructuraAlojada(t_list* , tipoEstructura);
bool tieneTripulanteAlojado(t_list* , int);
t_alojado* obtenerAlojadoPagina(t_list* , int);
int actualizarTripulanteEnMem(t_tablaPaginasPatota* , tcb*);
t_tarea* armarTarea(char* );
int frameTotalmenteLibre(int );
t_list* paginasConTripu(t_list*, uint32_t );
int sobreescribirTripu(t_list* , tcb* );
void cargarDLTripulante(void* , tcb* );
int actualizarTripulante(tcb* , int);
tcb* obtenerTripulante(t_tablaPaginasPatota* ,int );
tcb* cargarEnTripulante(void* );
t_tarea* asignarProxTarea(int , int);











#endif /* MEMORIA_H_ */




