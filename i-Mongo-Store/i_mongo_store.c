#include "i_mongo_store.h"


int main(void) {

	char* path = pathLog();

	logImongo = iniciarLogger(path, "i-mongo-store",1);

	crearConfig();

	cargarConfiguracion();

	crearTareasIO();



	//int serverSock = iniciarConexionDesdeServidor(datosConfig->puerto);

	//pthread_t manejoTripulante;
	//pthread_create(&manejoTripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
	//pthread_join(manejoTripulante, (void*) NULL);

	iniciarFileSystem();

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

char* crearDestinoApartirDeRaiz(char* destino){

	char* raiz = string_new();

	string_append(&raiz, datosConfig->puntoMontaje);
	string_append(&raiz, "/");
	string_append(&raiz, destino);

	return raiz;

}


bool validarExistenciaFileSystem(char* superBloque, char* blocks, char* raiz){


	return (access(superBloque, F_OK ) != -1) && (access(blocks, F_OK ) != -1) && (access(raiz, F_OK ) != -1);

}


void crearFileSystemDesdeCero(char* destinoRaiz, char* destinoSuperBloque, char* destinoBlocks){

	FILE* superBloque = fopen(destinoSuperBloque,"wb");
	FILE* blocks = fopen(destinoBlocks,"wb");

	char* pathFiles = crearDestinoApartirDeRaiz("Files");
	char* pathBitacora = crearDestinoApartirDeRaiz("Files/Bitacora");

	if(mkdir(pathFiles,0777) != 0){

		log_info(logImongo, "Hubo un error al crear el directorio %s", pathFiles);
	}

	if(mkdir(pathBitacora,0777) != 0){

			log_info(logImongo, "Hubo un error al crear el directorio %s", pathFiles);
		}

	char* destinoOxigeno = crearDestinoApartirDeRaiz("Files/Oxigeno.ims");
	char* destinoComida = crearDestinoApartirDeRaiz("Files/Comida.ims");
	char* destinoBasura = crearDestinoApartirDeRaiz("Files/Basura.ims");

	FILE* oxigeno = fopen(destinoOxigeno,"wb");
	FILE* comida = fopen(destinoComida,"wb");
	FILE* basura = fopen(destinoBasura,"wb");

}

void iniciarFileSystem(){

	char* destinoRaiz = crearDestinoApartirDeRaiz("");

	int flag = access(destinoRaiz, F_OK );

	if(flag == -1){

		log_info(logImongo, "No existe el punto de montaje en el directorio %s , se creara uno", destinoRaiz);

		mkdir(destinoRaiz,0777);

		}

	else if(flag == 0){

			log_info(logImongo, "Existe el punto de montaje en el directorio %s , se procedera a validar la existencia de SuperBloque.ims y Blocks.ims", destinoRaiz);

			}

	else{

		log_info(logImongo, "La verificacion de si existe la carpeta punto de montaje esta tirando cualquier valor");
		exit(1);
	}


	char* destinoSuperBloque = crearDestinoApartirDeRaiz("SuperBloque.ims");
	char* destinoBlocks = crearDestinoApartirDeRaiz("Blocks.ims");

	if(validarExistenciaFileSystem(destinoSuperBloque,destinoBlocks,destinoRaiz)){

		log_info(logImongo,"Existe un file system actualmente");


	}

	else{

		log_info(logImongo,"Existe el punto de montaje ahora, pero no existe SuperBloque.ims y Blocks.ims, creando archivos...");

		crearFileSystemDesdeCero(destinoRaiz, destinoSuperBloque, destinoBlocks);


	}

}




