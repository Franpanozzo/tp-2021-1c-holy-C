#ifndef VARIABLES_H_
#define VARIABLES_H_

typedef struct{
	t_tripulante* tripulanteSabotaje;
	t_coordenadas coordenadas;
	int haySabotaje;
	int tiempo;
	sem_t semaforoIniciarSabotaje;
	sem_t semaforoCorrerSabotaje;
	sem_t semaforoTerminoTripulante;
	sem_t semaforoTerminoSabotaje;
} t_sabotaje;


t_config* config;

t_log* logDiscordiador;
pthread_t planificador;

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

t_sabotaje* sabotaje;

char** todasLasTareasIO;

t_queue* colaExec;
t_queue* colaBlocked;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaSabotaje;

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
