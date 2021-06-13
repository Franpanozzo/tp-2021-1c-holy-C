#include "discordiador.h"

int tripulantes;
int main() {
	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/logDiscordiador.log","Discordiador",1);
	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	iniciarSemaforos();
	iniciarMutex();
	iniciarColas();
	iniciarTareasIO();
	cargar_configuracion();

	idTripulante = 0;
	idPatota = 0;


	//leerConsola();

	tripulantes = 2;
	t_coordenadas coordenadas[tripulantes ];
	for(int i=0; i<tripulantes;i++){
		coordenadas[i].posX = i+ 2;
		coordenadas[i].posY = i + 2;
	}

	//CONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2
	iniciarPatota(coordenadas, "COMER_YOGUR;2;3;7\nGENERAR_OXIGENO 4;2;3;7", tripulantes);
	pthread_create(&planificador, NULL, (void*) hiloPlani, NULL);
	pthread_join(planificador, (void**) NULL);
	//pthread_detach(planificador);


	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}




