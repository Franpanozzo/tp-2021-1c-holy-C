#include "i_mongo_store.h"


int main(void) {

	char* path = pathLog();

	logImongo = iniciarLogger(path, "i-mongo-store",1);

	cargarConfiguracion();

	crearTareasIO();

	int serverSock = iniciarConexionDesdeServidor(datosConfig->puerto);

	pthread_t manejoTripulante;
	pthread_create(&manejoTripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
	pthread_join(manejoTripulante, (void*) NULL);

	liberarConfiguracion();
	free(path);

	return EXIT_SUCCESS;

}

void atenderTripulantes(int* serverSock) {

    while(1){

		int* tripulanteSock = malloc(sizeof(int));

		*tripulanteSock = esperarTripulante(*serverSock);

		pthread_t t;

		pthread_create(&t, NULL, (void*) manejarTripulante, (void*) tripulanteSock);

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

	switch(paquete->codigoOperacion){

		case TAREA:
		{
				t_tarea* tarea = deserializarTarea(paquete->buffer->stream);

				log_info(logImongo,"tareaRecibida %s \n",tarea->nombreTarea);

				switch(indiceTarea(tarea)){

						case 0:
						{
									log_info(logImongo,"Recibi una tarea de GENERAR_OXIGENO \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"GENERAR_OXIGENO",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;
						}
						case 1:
						{
									log_info(logImongo,"Recibi una tarea de CONSUMIR_OXIGENO \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"CONSUMIR_OXIGENO",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;

						}
						case 2:
						{
									log_info(logImongo,"Recibi una tarea de GENERAR_BASURA \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"GENERAR_BASURA",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;

						}
						case 3:
						{
									log_info(logImongo,"Recibi una tarea de DESCARTAR_BASURA \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"DESCARTAR_BASURA",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;
						}
						case 4:
						{
									log_info(logImongo,"Recibi una tarea de GENERAR_COMIDA \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"GENERAR_COMIDA",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;
						}
						case 5:
						{
									log_info(logImongo,"Recibi una tarea de CONSUMIR_COMIDA \n");
									t_paquete* respuesta = armarPaqueteCon((void*)"CONSUMIR_COMIDA",STRING);
									enviarPaquete(respuesta,tripulanteSock);
									break;
						}

						default:

								log_info(logImongo,"No existe ese tipo de tarea negro \n");
								exit(1);

					}
				break;

		}

		default:
				log_info(logImongo,"Estos casos todavia no estan contemplados");

	}

	eliminarPaquete(paquete);

}
