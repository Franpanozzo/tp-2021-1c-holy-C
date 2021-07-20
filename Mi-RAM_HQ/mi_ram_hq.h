#ifndef MI_RAM_HQH
#define MI_RAM_HQH

#include "estructuras.h"
#include "paginacion.h"

t_log* logMiRAM;


void cargar_configuracion();
char* asignar_bytes(int );
void iniciarMemoria();
char* delimitarTareas(char*);
void mandarConfirmacionDisc(char* , int);
void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarTareas(void*,t_list*,uint32_t);
int deserializarInfoPCB(t_paquete*);
t_tarea* deserializarSolicitudTarea(t_paquete*);
void setearSgteTarea(tcb*);
void eliminarTarea(t_tarea*);
void eliminarPCB(pcb*);
void eliminarListaPCB(t_list*);
void eliminarListaTCB(t_list*);
void eliminarTCB(tcb* tcb);
tcb* buscarTripulante(int tcbAActualizar,pcb* patotaDeTripu);
void deserializarSegun(t_paquete*, int);
t_tarea* deserializarTripulante(t_paquete*);
pcb* buscarPatota(uint32_t);
void asignarSiguienteTarea(tcb*);
void mandarTarea(t_tarea* , int);
t_tarea* tarea_error();
char asignarEstadoTripu(t_estado);
int recibirActualizarTripulante(t_paquete* );
void deserializarExpulsionTripulante(t_paquete*);
void cargarDLTripulante(void* , tcb* );
tcb* cargarEnTripulante(void* );
t_tarea* armarTarea(char* );
void* meterEnBuffer(void* , tipoEstructura , int*, int*);
int guardarPCB(pcb*, char*);
t_tarea* guardarTCB(tcb*, int);
void hacerDump(int);
void expulsarTripulante(int,int);
t_tarea* asignarProxTarea(int, int);
int actualizarTripulante(tcb* , int);
char* pathLogRam();
char* pathLogMemoria();
t_tarea* tarea_nula();


#endif
