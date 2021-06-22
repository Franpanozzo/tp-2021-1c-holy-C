#ifndef UTILS_H_
#define UTILS_H_

#include "discordiador.h"
#include "variables.h"

#define NO_HAY_TRIPULANTE_BLOQUEADO -1
#define SIN_QUANTUM -1
#define CORRIENDO 1
#define PAUSADA 0
#define SIN_EMPEZAR -1

void iniciarTareasIO();
void iniciarColas();
void iniciarSemaforos();
void iniciarMutex();
void cargar_configuracion();
char * pathLog();

int leerTotalTripus();
void modificarTripulanteBlocked(uint32_t);
int leerPlanificacion();
void modificarPlanificacion(int);
uint32_t leerTripulanteBlocked();
void crearConfig();


t_patota* asignarDatosAPatota(char*);

t_tripulante* elTripulanteMasCerca(t_coordenadas);
void esperarTerminarTripulante(t_tripulante*);
void avisarTerminoPlanificacion(t_tripulante*);

void casoBlocked();
void iterarCola(t_lista*, t_estado);
void pasarDeLista(t_tripulante*);
void meterEnLista(void* , t_lista*);
void* sacarDeLista(t_lista*);

char* deserializarString (t_paquete*);
void mandarTareaAejecutar(t_tripulante*,int);
void actualizarEstadoEnRAM(t_tripulante*);
void pasarAcolaSabotaje(t_lista*);
int enviarA(puertoEIP*, void*, tipoDeDato);
bool tripulanteDeMenorId(void*, void*);
t_tripulante* elTripuMasCerca(t_coordenadas);

void recibirPrimerTareaDeMiRAM(t_tripulante*);
void recibirProximaTareaDeMiRAM(t_tripulante*);
void recibirTareaDeMiRAM(int, t_tripulante*);
void recibirConfirmacionDeMongo(int, t_tarea*);
void esperarConfirmacionDeRAM(int);
char* esperarConfirmacionDePatotaEnRAM(int);

int esIO(char*);
uint32_t calculoCiclosExec(t_tripulante*);
void desplazarse(t_tripulante*, t_coordenadas);
uint32_t diferencia(uint32_t, uint32_t);

void listarTripulantes();
char* traducirEstado(t_estado);

bool esElBuscado(void*);
bool tieneDistintoEstado(void*);
bool tieneIgualEstado(void*);
int totalTripulantes();

void eliminarTripulante();
void eliminarPatota(t_patota*);
void liberarTripulante(t_tripulante*);

#endif
