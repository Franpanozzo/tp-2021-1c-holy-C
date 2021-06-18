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
void iterarCola(t_queue*, t_estado);
void pasarDeCola(t_tripulante*);

void siguienteTarea(t_tripulante* tripulante, int* ciclosExec);
char* deserializarString (t_paquete*);
void mandarTareaAejecutar(t_tripulante*,int);
void actualizarEstadoEnRAM(t_tripulante*);
int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion);
bool tripulanteDeMenorId(void*, void*);
t_tripulante* elTripuMasCerca(t_coordenadas lugarSabotaje);

void recibirPrimerTareaDeMiRAM(t_tripulante*);
void recibirProximaTareaDeMiRAM(t_tripulante*);
void recibirTareaDeMiRAM(int ,t_tripulante*);
void recibirConfirmacionDeMongo(int, t_tarea*);
void esperarConfirmacionDeRAM(int);
char* esperarConfirmacionDePatotaEnRAM(int);

int esIO(char*);
uint32_t calculoCiclosExec(t_tripulante*);
void desplazarse(t_tripulante*, t_coordenadas);
uint32_t diferencia(uint32_t, uint32_t);

void listarTripulantes();
char* traducirEstado(t_estado);

void eliminarTripulante(uint32_t);
void eliminarPatota(t_patota*);
void liberarTripulante(t_tripulante*);
void liberarColaEnd();

#endif
