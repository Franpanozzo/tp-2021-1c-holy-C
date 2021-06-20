#include "i_mongo_store.h"


int main(void) {

	logImongo = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/logs/i-mongo-store.log", "i-mongo-store",1);

	crearTareasIO();

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


int indiceTarea(t_tarea* tarea){

	int i = 0;

	while(todasLasTareasIO[i] != NULL){

		if(strcmp(todasLasTareasIO[i],tarea->nombreTarea) == 0){
			return i;
		}

		i++;
	}

	return -1;
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
