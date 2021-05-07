#include "mi_ram_hq.h"


#define PORT 3222 // Define el nuemero de puerto que va a tener mongo
int discordiador_socket; // Entero donde se guarda el el valor de accept del socket


int main(void) {

	/*
	t_paquete* paquete = recibirPaquete(discordiador_socket);


	switch(paquete->codigo_operacion) {
		case PERSONA:
			paraRecibir = deserializarPersona(paquete->buffer);
								break;
		default:
				break;
	}

	printf("Si tuvimos exito se va a leer algo a continuacion: ----%s---- \n",paraRecibir->nombre);
	 */


	return EXIT_SUCCESS;

}


void iniciarConexion(int *discordiador_socket) {

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);// Crea socket en la variable local server_sock
	unsigned int len = sizeof(struct sockaddr); // Crea un entero sin signo que almacena la cantidad de bytes que ocupa la estructura sockaddr
	int yes = 1;


	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
	}
	struct sockaddr_in* localAddress = malloc(sizeof(struct sockaddr_in));
	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	localAddress->sin_addr.s_addr = inet_addr("127.0.0.1");
	localAddress->sin_port = htons(PORT);
	localAddress->sin_family = AF_INET;

	if (bind(server_sock, (struct sockaddr*) localAddress, (socklen_t) sizeof(struct sockaddr_in)) == -1) {
		perror("bind");
	}

	if (listen(server_sock, 10) == -1) {
		perror("listen");
	}

	if ((*discordiador_socket = accept(server_sock, (struct sockaddr*) serverAddress, &len)) == -1) {
		perror("accept");
	}



	free(serverAddress);
	free(localAddress);

}




