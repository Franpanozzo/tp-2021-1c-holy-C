#ifndef UTILS_H_
#define UTILS_H_

#include "discordiador.h"
#include "variables.h"

void iniciarTareasIO();
void iniciarColas();
void iniciarSemaforos();
void iniciarMutex();
void cargar_configuracion();
char * pathLog();

int leerTotalTripus();
void crearConfig();

void iniciarTripulante(t_coordenadas, uint32_t);
void iniciarPatota(t_coordenadas*, char*, uint32_t);
t_patota* asignarDatosAPatota(char*);

void hiloPlani();
void hilitoSabo();
void hiloTripu(t_tripulante*);
void actualizarCola(t_estado, t_queue*, pthread_mutex_t);
void iterarCola(t_queue*);
void pasarDeCola(t_tripulante*);

void siguienteTarea(t_tripulante* tripulante, int* ciclosExec, int* tripuVivo);
char* deserializarString (t_paquete*);
void mandarTareaAejecutar(t_tripulante*,int);
void actualizarEstadoEnRAM(t_tripulante*);
int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion);

void recibirPrimerTareaDeMiRAM(t_tripulante*);
void recibirProximaTareaDeMiRAM(t_tripulante*);
void recibirTareaDeMiRAM(int ,t_tripulante*);
void recibirConfirmacionDeMongo(int, t_tarea*);
void esperarConfirmacionDeRAM(int);

int esIO(char*);
uint32_t calculoCiclosExec(t_tripulante*);
void desplazarse(t_tripulante*);
uint32_t diferencia(uint32_t, uint32_t);

void listarTripulante();
char* traducirEstado(t_estado);

int deleteTripulante(uint32_t, t_queue*);
void eliminarTripulante(uint32_t);
void eliminarPatota(t_patota*);
void liberarTripulante(t_tripulante*);
void liberarColaEnd();

#endif
