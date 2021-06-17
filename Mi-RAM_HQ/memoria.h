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

#define FRAME_INVALIDO 2147483646

#define TAM_TCB 21
#define TAM_PCB 8

#include "estructuras.h"


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
} t_alojado;


t_log* logMemoria;
t_configRam configRam;

void* memoria_principal;
t_bitarray* frames_ocupados_ppal;
int cant_frames_ppal;

t_list* tablasPaginasPatotas;

pcb cabron;


void cargar_configuracion();
bool get_frame(int , int);
void set_frame(int , int);
char* asignar_bytes(int );
void iniciarMemoria();
void* leer_memoria(int,int);
int insertar_en_memoria(t_info_pagina*, void*, int, int*, tipoEstructura);
void agregarEstructAdminTipo(t_info_pagina*, int, int, tipoEstructura);
uint32_t buscar_frame_disponible(int );
void* buscar_pagina(t_info_pagina* );
t_info_pagina* crearPaginaEnTabla(t_tablaPaginasPatota* ,tipoEstructura);
int asignarPaginasEnTabla(void* , t_tablaPaginasPatota* , tipoEstructura );
t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int );
t_info_pagina* buscarUltimaPaginaDisponible(t_tablaPaginasPatota* );
void* meterEnBuffer(void* , tipoEstructura , int* );
int guardarTCB(tcb* , int);
int guardarPCB(pcb*  ,char*);
uint32_t estimarDLTareas();


#endif /* MEMORIA_H_ */




