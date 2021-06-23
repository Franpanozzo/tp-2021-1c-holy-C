#ifndef VARIABLES_H_
#define VARIABLES_H_

t_config* config;

t_log* logDiscordiador;
pthread_t planificador;
pthread_t sabo;

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante;
int idPatota;
int planificacion;
int totalTripus;
int haySabotaje;
int quantum;
int idTripulanteBlocked;
int gradoMultiprocesamiento;
int idBuscado;
int retardoCiclosCPU;
t_estado estadoAcomparar;

t_sabotaje* sabotaje;

char** todasLasTareasIO;

t_lista* listaExec;
t_lista* listaBlocked;
t_lista* listaNew;
t_lista* listaReady;
t_lista* listaSabotaje;

pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaExec;
pthread_mutex_t mutexColaBlocked;
pthread_mutex_t mutexColaEnd;
pthread_mutex_t mutexPlanificadorFin;
pthread_mutex_t mutexLogDiscordiador;
pthread_mutex_t mutexTotalTripus;
pthread_mutex_t mutexIdTripulanteBlocked;
pthread_mutex_t mutexPlanificador;


sem_t semPlanificacion;
sem_t semaforoPlanificadorInicio;
sem_t semaforoPlanificadorFin;


#endif
