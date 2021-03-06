#ifndef VARIABLES_H_
#define VARIABLES_H_

	#define NO_HAY_TRIPULANTE_BLOQUEADO -1
	#define SIN_QUANTUM -1
	#define CORRIENDO 1
	#define PAUSADA 0
	#define SIN_EMPEZAR -1


	t_config* config;
	t_log* logDiscordiador;

	pthread_t planificador;
	pthread_t manejoSabotaje;

	sem_t semHayTripulantes;

	puertoEIP* puertoEIPRAM;
	puertoEIP* puertoEIPMongo;

	int idTripulante;
	int idPatota;
	int planificacion;
	int quantum;
	int gradoMultiprocesamiento;
	int retardoCiclosCPU;
	int idTripulanteBlocked;
	int puertoDisc;

	t_estado estadoAcomparar;
	t_sabotaje* sabotaje;

	sem_t semPlanificacion;

	char** todasLasTareasIO;

	t_lista* listaExec;
	t_lista* listaBlocked;
	t_lista* listaNew;
	t_lista* listaReady;
	t_lista* listaSabotaje;
	t_lista* listaExit;

	pthread_mutex_t mutexTotalTripus;
	pthread_mutex_t mutexIdTripulanteBlocked;
	pthread_mutex_t mutexPlanificador;
	pthread_mutex_t mutexEliminarPatota;



#endif
