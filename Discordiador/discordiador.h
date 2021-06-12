#ifndef DISCORDIADOR_H_
#define DISCORDIADOR_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <bibliotecas.h>
#include <unistd.h>
#include <sys/select.h>
#include <memory.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <stdbool.h>
#include "consola.h"
//#include "utils.h"
#include <math.h>


t_config* config;

t_log* logDiscordiador;

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante;
int idPatota;
int planificacionPlay;
int totalTripus;
int haySabotaje;
int quantum;
int idTripulanteBlocked;
int gradoMultiprocesamiento;

t_queue* colaExec;
t_queue* colaBlocked;
t_queue* colaNew;
t_queue* colaReady;

pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaExec;
pthread_mutex_t mutexColaBlocked;
pthread_mutex_t mutexPlanificadorFin;
pthread_mutex_t mutexLogDiscordiador;
pthread_mutex_t mutexTotalTripus;

sem_t semPlanificacion;
sem_t semaforoPlanificadorInicio;
sem_t semaforoPlanificadorFin;

t_tripulante* tripulanteDesabotaje;

typedef struct{
	int socket;
	t_tripulante* tripulante;

}t_bloqueado;

typedef struct{

	int cantidad;
	t_tripulante* tripulante;

}t_eliminado;

int leerTotalTripus();
void loginfo( char*);
int enviarA(puertoEIP* puerto, void* informacion, tipoDeDato codigoOperacion);
void crearConfig();
void eliminarPatota(t_patota*);
void iniciarTripulante(t_coordenadas, uint32_t);
void iniciarPatota(t_coordenadas*, char*, uint32_t);
void pasarDeCola(t_tripulante*);
void hiloTripulante(t_tripulante* );
t_patota* asignarDatosAPatota(char*);
char* deserializarString (t_paquete*);
void mandarTareaAejecutar(t_tripulante*,int);
void recibirPrimerTareaDeMiRAM(t_tripulante*);
void recibirProximaTareaDeMiRAM(t_tripulante*);
void recibirTareaDeMiRAM(int ,t_tripulante*);
int esIO(char*);
void hiloPlani();
void hilitoSabo();
void actualizarCola(t_estado, t_queue*, pthread_mutex_t);
//t_tripulante* elTripuMasCerca(t_coordenadas);
uint32_t calculoCiclosExec(t_tripulante*);
uint32_t diferencia(uint32_t, uint32_t);
void desplazarse(t_tripulante*);
void actualizarEstadoEnRAM(t_tripulante*);
int calcularCiclosExec(t_tripulante*);
int calculoMovimiento(t_tripulante*);
void listarTripulante();
char* traducirEstado(t_estado);
void iterarCola(t_queue*);
void pausarPlanificacion();
t_eliminado* deleteTripulante(uint32_t, t_queue*);
void eliminarTripulante(uint32_t);
void esperarConfirmacionDeRAM(int);


#endif
