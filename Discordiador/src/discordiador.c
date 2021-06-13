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


	leerConsola();




	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}




