#include "i_mongo_store.h"


int main(void) {

	char* path = pathLog();

	logImongo = iniciarLogger(path, "i-mongo-store",1);

	cargarTodosLosConfig();

	asignarTareas();

	cargarConfiguracion();

	iniciarFileSystem();

	//int serverSock = iniciarConexionDesdeServidor(datosConfig->puerto);

	//pthread_t manejoTripulante;
	//pthread_create(&manejoTripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
	//pthread_join(manejoTripulante, (void*) NULL);

	liberarConfiguracion();

	liberarTareas();

	liberarTodosLosConfig();

	log_destroy(logImongo);

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

    deserializarSegun(paquete,tripulanteSock);
}


void deserializarSegun(t_paquete* paquete, int *tripulanteSock){

	log_info(logImongo,"Deserializando...");

	switch(paquete->codigoOperacion){

		case TAREA:
					{

					t_tarea* tarea = deserializarTarea(paquete->buffer->stream);

					log_info(logImongo,"tareaRecibida %s \n",tarea->nombreTarea);

					seleccionarTarea(tarea,tripulanteSock);

					break;

					}

		case DESPLAZAMIENTO:
					{
					break;
					}

		case INICIO_TAREA:
					{
					break;
					}

		case FIN_TAREA:
					{
					break;
					}

		case ID_SABOTAJE:
					{
					break;
					}
		case FIN_SABOTAJE:
					{
					break;
					}

		default:
				log_info(logImongo,"i-Mongo-Store no entiende esa tarea");

	}

	eliminarPaquete(paquete);

}





void seleccionarTarea(t_tarea* tarea, int* tripulanteSock){


				switch(indiceTarea(tarea)){

						case 0:

						{
									log_info(logImongo,"Recibi una tarea de GENERAR_OXIGENO \n");

									generarOxigeno(tarea,tripulanteSock);

									break;
						}

						case 1:

						{
									log_info(logImongo,"Recibi una tarea de CONSUMIR_OXIGENO \n");

									consumirOxigeno(tarea,tripulanteSock);

									break;
						}
						case 2:

						{
									log_info(logImongo,"Recibi una tarea de GENERAR_COMIDA \n");

									generarComida(tarea,tripulanteSock);

									break;

						}
						case 3:

						{
									log_info(logImongo,"Recibi una tarea de CONSUMIR_COMIDA \n");

									consumirComida(tarea,tripulanteSock);

									break;

						}

						case 4:

						{
									log_info(logImongo,"Recibi una tarea de GENERAR_BASURA \n");

									generarBasura(tarea,tripulanteSock);

									break;

						}

						case 5:

						{
									log_info(logImongo,"Recibi una tarea de DESCARTAR_BASURA \n");

									descartarBasura(tarea,tripulanteSock);

									break;

						}

						default:

								log_info(logImongo,"No existe ese tipo de tarea negro \n");
								exit(1);

					}

}


void crearFileSystemDesdeCero(char* destinoSuperBloque, char* destinoBlocks, char* destinoRaiz){

	superBloque = malloc(sizeof(t_superBloque));

	superBloque->block_size = (uint32_t) config_get_int_value(configImongo,"BLOCK_SIZE");
	superBloque->blocks = (uint32_t)config_get_int_value(configImongo,"BLOCKS");

	int sizeBitArray = superBloque->block_size * superBloque->blocks / 8;
	bitArray = malloc(sizeBitArray);
	superBloque->bitmap = bitarray_create_with_mode(bitArray,sizeBitArray ,MSB_FIRST);

	for(int i=0; i<sizeBitArray;i++){

		bitarray_clean_bit(superBloque->bitmap, i);

	}

    log_info(logImongo,"Se inicializo un bitmap con %d posiciones", bitarray_get_max_bit(superBloque->bitmap));

    char* bitmap = malloc(sizeBitArray + 1);

    for(int i=0; i<sizeBitArray;i++){

      	int valor =  bitarray_test_bit(superBloque->bitmap, i);

      	bitmap[i] = valor + '0';

      	printf("%c ", bitmap[i]);

     }

    config_set_value(configSuperBloque,"BLOCK_SIZE",string_itoa(superBloque->block_size));
    config_set_value(configSuperBloque,"BLOCKS",string_itoa(superBloque->blocks));
    config_set_value(configSuperBloque,"BITMAP",bitmap);

    config_save(configSuperBloque);

	int fd = open(destinoBlocks,O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);

	if(mkdir(pathFiles,0777) != 0){

		log_info(logImongo, "Hubo un error al crear el directorio %s", pathFiles);
	}

	if(mkdir(pathBitacora,0777) != 0){

			log_info(logImongo, "Hubo un error al crear el directorio %s", pathFiles);
		}

	crearMemoria(fd);

}

void iniciarFileSystem(){

	int flag = access(datosConfig->puntoMontaje, F_OK );

	if(flag == -1){

		log_info(logImongo, "No existe el punto de montaje en el directorio %s , se creara uno", datosConfig->puntoMontaje);

		mkdir(datosConfig->puntoMontaje,0777);

		}

	else if(flag == 0){

			log_info(logImongo, "Existe el punto de montaje en el directorio %s , se procedera a validar la existencia de SuperBloque.ims y Blocks.ims", datosConfig->puntoMontaje);

			}

	else{

		log_info(logImongo, "La verificacion de si existe la carpeta punto de montaje esta tirando cualquier valor");
		exit(1);
	}

	if(validarExistenciaFileSystem(pathSuperBloque,pathBloque,datosConfig->puntoMontaje)){

		log_info(logImongo,"Existe un file system actualmente");

		superBloque->block_size = config_get_int_value(configSuperBloque,"BLOCK_SIZE");
		superBloque->blocks = config_get_int_value(configSuperBloque,"BLOCKS");

		int sizeBitArray = superBloque->block_size * superBloque->blocks / 8;
		bitArray = malloc(sizeBitArray);
		superBloque->bitmap = bitarray_create_with_mode(bitArray,sizeBitArray ,MSB_FIRST);

		char* bitmap = config_get_string_value(configSuperBloque,"BITMAP");

		for(int i=0; i<sizeBitArray;i++){

			if(bitmap[i] == '1'){

				bitarray_set_bit(superBloque->bitmap,i);

			}
			else if (bitmap[i] == '0'){

				bitarray_clean_bit(superBloque->bitmap,i);

			}
			else{

				log_info(logImongo,"El archivo del bitarray tira valores no validos");
				exit(1);

			}

		}

	}

	else{

		log_info(logImongo,"Existe el punto de montaje ahora, pero no existe SuperBloque.ims y Blocks.ims, creando archivos...");

		crearFileSystemDesdeCero(pathSuperBloque,pathBloque,datosConfig->puntoMontaje);


	}

}




