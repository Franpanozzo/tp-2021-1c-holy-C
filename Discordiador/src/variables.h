#ifndef VARIABLES_H_
#define VARIABLES_H_
t_config* config;

t_log* logDiscordiador;
pthread_t planificador;

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

char** todasLasTareasIO;

t_queue* colaExec;
t_queue* colaBlocked;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaEnd;

pthread_mutex_t mutexColaNew;
pthread_mutex_t mutexColaReady;
pthread_mutex_t mutexColaExec;
pthread_mutex_t mutexColaBlocked;
pthread_mutex_t mutexColaEnd;
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

#endif
