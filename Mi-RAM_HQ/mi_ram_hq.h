#ifndef MI_RAM_HQH
#define MI_RAM_HQH

#include "estructuras.h"
#include "memoria.h"

t_log* logMiRAM;


char* delimitarTareas(char*);
void mandarConfirmacionDisc(char* , int);
void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarTareas(void*,t_list*,uint32_t);
int deserializarInfoPCB(t_paquete*);
void deserializarSolicitudTarea(t_paquete*,int);
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


#endif
