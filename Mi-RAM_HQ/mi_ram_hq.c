#include "mi_ram_hq.h"


int main(void) {

	int discordiador_socket = 0;

	int puerto = 3222;

	iniciarConexionDesdeServidor(&discordiador_socket,puerto);

	recibirPaquete(discordiador_socket);

	return EXIT_SUCCESS;
}







