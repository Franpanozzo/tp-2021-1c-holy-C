#include "mi_ram_hq.h"


int main(void) {

	int puerto = 3222;

	int serverSock = iniciarConexionDesdeServidor(puerto);

	atender_tripulantes(serverSock);


	return EXIT_SUCCESS;
}


void atender_tripulantes(int serverSock) {

	while(1){

	int tripulanteSock = esperar_tripulante(serverSock);

	manejar_tripulante(tripulanteSock);


	}

}


int esperar_tripulante(int serverSock) {

	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));
	unsigned int len = sizeof(struct sockaddr);

	int socket_tripulante = accept(serverSock, (struct sockaddr*) serverAddress, &len);

	//log_info(logger, "Se conecto un cliente!");

	return socket_tripulante;

}


void manejar_tripulante(int tripulanteSock) {

	t_paquete* paquete = recibirPaquete(tripulanteSock);


	switch(paquete->codigo_operacion){

				case PERSONA:
							deserializarPersona(paquete->buffer);
							break;


				default:
						printf("\n No se puede deserializar ese tipo de estructura negro \n");
						exit(1);
			}

	eliminarPaquete(paquete);
}







