#include "i_mongo_store.h"


int main(void) {

	int puerto = 5001;

	int serverSock = iniciarConexionDesdeServidor(puerto);

	pthread_t manejo_tripulante2;
	pthread_create(&manejo_tripulante2, NULL, (void*) atenderTripulantes, (void*) &serverSock);
	pthread_join(manejo_tripulante2, (void*) NULL);


	return EXIT_SUCCESS;

}

void atenderTripulantes(int* serverSock) {

    while(1){

		int tripulanteSock = esperarTripulante(*serverSock);

		pthread_t t;

		pthread_create(&t, NULL, (void*) manejarTripulante, (void*) &tripulanteSock);

		pthread_detach(t);

    }

}


int esperarTripulante(int serverSock) {

    struct sockaddr_in serverAddress;

    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (void*) &serverAddress, &len);

    log_info(logImongo, "Se conecto un cliente!\n");

    return socket_tripulante;

}

//CUANDO CREAS UN HILO HAY QUE PASAR SI O SI UN PUNTERO

void manejarTripulante(int *tripulanteSock) {

    t_paquete* paquete = recibirPaquete(*tripulanteSock);

    deserializarSegun(paquete,*tripulanteSock);
}


void deserializarSegun(t_paquete* paquete, int tripulanteSock){

	log_info(logImongo,"Deserializando...");

	switch(paquete->codigo_operacion){

			case PATOTA:

						log_info(logImongo,"Patota recibida negro \n");
						break;

			case TRIPULANTE:

						log_info(logImongo,"Tripulante recibido negro \n");
						break;

			case TAREA:
			{

						log_info(logImongo,"Tarea recibida negro \n");

						break;
			}
			default:

					log_info(logImongo,"No se puede deserializar ese tipo de estructura negro \n");
					exit(1);

		}
	eliminarPaquete(paquete);
}
