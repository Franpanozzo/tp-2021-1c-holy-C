#include "i_mongo_store.h"


int main(void) {

	signal(SIGUSR1, sabotaje);

	oxigeno = malloc(sizeof(t_file2));
	oxigeno->caracterLlenado = "O";
	comida = malloc(sizeof(t_file2));
	comida->caracterLlenado = "C";
	basura = malloc(sizeof(t_file2));
	basura->caracterLlenado = "B";

	sem_init(&sabotajeResuelto, 0, 0);
	sem_init(&semTarea, 0, 0);


	superBloque = malloc(sizeof(t_superBloque));

	bitacoras = dictionary_create();

	haySabotaje = false;
	cantEscriturasPendientes = 0;

	char* path = pathLog();
	logImongo = iniciarLogger(path, "i-mongo-store",1);
	crearConfig(&configImongo,"/home/utnso/tp-2021-1c-holy-C/i-Mongo-Store/i_mongo_store.config");
	cargarDatosConfig();
	cargarPaths();
	iniciarMutex();
	asignarTareas();
	iniciarFileSystem();

	listaPosicionesSabotaje = listaCoordenadasSabotaje();

	pthread_create(&hiloSincronizador, NULL, (void*) sincronizarMemoriaSecundaria, NULL);
	pthread_detach(hiloSincronizador);

	int serverSock = iniciarConexionDesdeServidor(datosConfig->puerto);

	pthread_create(&manejoTripulante, NULL, (void*) atenderTripulantes, (void*) &serverSock);
	pthread_join(manejoTripulante, (void*) NULL);

	proximoPosSabotaje = 0;

	liberarRecursosGlobalesAlTerminar();

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

    return socket_tripulante;
}


void manejarTripulante(int *tripulanteSock) {

    t_paquete* paquete = recibirPaquete(*tripulanteSock);

    deserializarSegun(paquete,tripulanteSock);

}


void deserializarSegun(t_paquete* paquete, int* tripulanteSock){

	switch(paquete->codigoOperacion){

		case TAREA:
		{
			t_tarea* tarea = deserializarTarea(paquete->buffer->stream);

			log_info(logImongo,"tarea recibida %s",tarea->nombreTarea);

			seleccionarTarea(tarea);

			free(tarea->nombreTarea);
			free(tarea);
			break;
		}

		case DESPLAZAMIENTO:
		{

			t_desplazamiento* desplazamiento = deserializarDesplazamiento(paquete->buffer->stream);


			log_info(logImongo,"Se recibio el desplazamiento del tripulante de ID %d", desplazamiento->idTripulante);

			char* mensaje = string_from_format("Se mueve de %d|%d a %d|%d.",
					desplazamiento->inicio.posX, desplazamiento->inicio.posY,
					desplazamiento->fin.posX, desplazamiento->fin.posY);

			char* stringIdtripulante = string_itoa(desplazamiento->idTripulante);

			lock(&mutexExisteBitacora);
			if(!dictionary_has_key(bitacoras, stringIdtripulante)){
				crearBitacora(stringIdtripulante);
			}
			unlock(&mutexExisteBitacora);

			if(leerHaySabotaje()){
				modificarCantEscriturasPendientes(leerCantEscriturasPendientes() + 1);
				sem_wait(&sabotajeResuelto);
			}

			escribirBitacora2(stringIdtripulante, mensaje);

			free(stringIdtripulante);
			free(mensaje);
			free(desplazamiento);

			break;
		}

		case INICIO_TAREA:
		{
			t_avisoTarea* avisoTarea = deserializarAvisoTarea(paquete->buffer->stream);

			log_info(logImongo,"Se recibio el inicio de tarea del tripulante de ID %d", avisoTarea->idTripulante);

			char* mensaje = string_from_format("Empieza la tarea %s.", avisoTarea->nombreTarea);

			char* stringIdtripulante = string_itoa(avisoTarea->idTripulante);

			lock(&mutexExisteBitacora);
			if(!dictionary_has_key(bitacoras, stringIdtripulante)){
				crearBitacora(stringIdtripulante);
			}
			unlock(&mutexExisteBitacora);

			if(leerHaySabotaje()){
				modificarCantEscriturasPendientes(leerCantEscriturasPendientes() + 1);
				sem_wait(&sabotajeResuelto);
			}

			escribirBitacora2(stringIdtripulante, mensaje);

			free(mensaje);
			free(stringIdtripulante);
			free(avisoTarea->nombreTarea);
			free(avisoTarea);

			/*
			if(!dictionary_has_key(semaforosTareas, stringIdtripulante)){
				sem_t* semTarea = (sem_t*) dictionary_get(semaforosTareas, stringIdtripulante);
				sem_post(semTarea);
			}
			else{
				sem_t* semTarea = malloc(sizeof(sem_t));
				sem_init(semTarea, 0, 0);
				dictionary_put(semaforosTareas, stringIdtripulante, semTarea);
			}
			*/

			break;
		}

		case FIN_TAREA:
		{


			t_avisoTarea* avisoTarea = deserializarAvisoTarea(paquete->buffer->stream);
			char* stringIdtripulante = string_itoa(avisoTarea->idTripulante);

			/*
			//FALTA MUTEX
			if(!dictionary_has_key(semaforosTareas, stringIdtripulante)){
				sem_t* semTarea = (sem_t*) dictionary_get(semaforosTareas, stringIdtripulante);
				sem_wait(semTarea);
			}
			else{
				sem_t* semTarea = malloc(sizeof(sem_t));
				sem_init(semTarea, 0, 0);
				dictionary_put(semaforosTareas, stringIdtripulante, semTarea);
				sem_wait(semTarea);
			}
			*/
			log_info(logImongo,"Se recibio el fin de tarea del tripulante de ID %d", avisoTarea->idTripulante);

			char* mensaje = string_from_format("Termino la tarea %s.", avisoTarea->nombreTarea);


			lock(&mutexExisteBitacora);
			if(!dictionary_has_key(bitacoras, stringIdtripulante)){
				crearBitacora(stringIdtripulante);
			}
			unlock(&mutexExisteBitacora);

			if(leerHaySabotaje()){
				modificarCantEscriturasPendientes(leerCantEscriturasPendientes() + 1);
				sem_wait(&sabotajeResuelto);
			}

			escribirBitacora2(stringIdtripulante, mensaje);

			free(mensaje);
			free(stringIdtripulante);
			free(avisoTarea->nombreTarea);
			free(avisoTarea);

			break;
		}

		case ID_SABOTAJE:
		{
			int idTripulante = deserializarID(paquete->buffer->stream);

			log_info(logImongo,"Se recibio que el tripulante de ID %d va a resolver el sabotaje", idTripulante);

			char* mensaje = string_from_format("Se dirige al sabotaje.");

			char* stringIdtripulante = string_itoa(idTripulante);

			lock(&mutexExisteBitacora);
			if(!dictionary_has_key(bitacoras, stringIdtripulante)){

				crearBitacora(stringIdtripulante);
			}
			unlock(&mutexExisteBitacora);

			if(leerHaySabotaje()){
				modificarCantEscriturasPendientes(leerCantEscriturasPendientes() + 1);
				sem_wait(&sabotajeResuelto);
			}

			escribirBitacora2(stringIdtripulante, mensaje);

			free(stringIdtripulante);

			free(mensaje);

			break;
		}

		case RESOLUCION_SABOTAJE:
		{
			int idTripulante = deserializarID(paquete->buffer->stream);

			log_info(logImongo,"Se recibio que el tripulante de ID %d ya resolvio el sabotaje", idTripulante);

			char* mensaje = string_from_format("Resolvio el sabotaje.");

			char* stringIdtripulante = string_itoa(idTripulante);

			lock(&mutexExisteBitacora);
			if(!dictionary_has_key(bitacoras, stringIdtripulante)){
				crearBitacora(stringIdtripulante);
			}
			unlock(&mutexExisteBitacora);

			fsck();

			escribirBitacora2(stringIdtripulante, mensaje);
			free(mensaje);
			free(stringIdtripulante);

			break;
		}

		case OBTENER_BITACORA:
		{
			int idTripulante = deserializarID(paquete->buffer->stream);

			char * stringIdtripulante = string_itoa(idTripulante);

			log_info(logImongo,"Se va a buscar la bitacora del tripulante de ID %s", stringIdtripulante);

			if(dictionary_has_key(bitacoras, stringIdtripulante)){

				t_bitacora_tripulante* bitacora = (t_bitacora_tripulante*) dictionary_get(bitacoras, stringIdtripulante);

				char* bitacoraTripulante = reconstruirArchivo(bitacora->bloques);

				t_paquete* paquete = armarPaqueteCon(bitacoraTripulante,STRING);

				enviarPaquete(paquete,*tripulanteSock);
			}

			free(stringIdtripulante);
			break;
		}

		default:
			log_info(logImongo,"i-Mongo-Store no entiende esa tarea");
	}

	 close(*tripulanteSock);
	 free(tripulanteSock);

	eliminarPaquete(paquete);

}


void seleccionarTarea(t_tarea* tarea){

	log_info(logImongo,"Recibi la tarea %s", tarea->nombreTarea);

	switch(indiceTarea(tarea)){

		case 0:
		{
			generarTarea2(oxigeno, tarea->parametro);
			break;
		}

		case 1:
		{

			consumirTarea2(oxigeno, tarea->parametro);
			break;
		}

		case 2:
		{
			generarTarea2(comida, tarea->parametro);
			break;
		}

		case 3:
		{
			consumirTarea2(comida,tarea->parametro);
			break;
		}

		case 4:
		{
			generarTarea2(basura, tarea->parametro);
			break;
		}

		case 5:
		{
			lock(&basura->mutex);
			uint32_t cantConsumir = basura->tamanioArchivo;
			unlock(&basura->mutex);
			consumirTarea2(basura, cantConsumir);
			break;
		}

		default:

			log_info(logImongo,"No existe ese tipo de tarea");
			exit(1);
	}
}


void eliminarBloquesDeBitacoraTripulante(t_config* config){

	char** bloquesQueOcupa = config_get_array_value(config,"BLOCKS");

	int i = 0;

	uint32_t bloque = 0;

	while((*(bloquesQueOcupa + i)) != NULL){

		bloque = atoi(*(bloquesQueOcupa + i));
		log_info(logImongo,"Los bloques que ocupa la tarea en curso son %d", bloque);
		consumirBloque(bloque,superBloque->block_size,superBloque->block_size);
		liberarBloque(bloque);
		i++;
	}

	actualizarSuperBloque();

	config_destroy(config);

	for(int j=0; j<i; j++){

		free(*(bloquesQueOcupa + j));

	}

	free(bloquesQueOcupa);

}


void eliminarBitacorasAnteriores(){

	t_config* config;

	int i = 1;

	char* pathBitacoraTripulante = strdup(pathBitacoras);

	string_append_with_format(&pathBitacoraTripulante,"/Tripulante%d.ims",i);

		while(verificarSiExiste(pathBitacoraTripulante)){

		log_info(logImongo,"El path de la metadata del tripulante %d es %s",i,pathBitacoraTripulante);

		 crearConfig(&config,pathBitacoraTripulante);

		 eliminarBloquesDeBitacoraTripulante(config);

		 remove(pathBitacoraTripulante);

		 i++;

		 free(pathBitacoraTripulante);

		 pathBitacoraTripulante = strdup(pathBitacoras);

		 string_append_with_format(&pathBitacoraTripulante,"/Tripulante%d.ims",i);

		}

		free(pathBitacoraTripulante);

}


void crearFileSystemExistente(){

	cargarSuperBloque();
	cargarBlocks();
	cargarFile(oxigeno);
	cargarFile(comida);
	cargarFile(basura);

	if(!verificarSiExiste(pathBitacoras)){

		mkdir(pathBitacoras,0777);
	}

	eliminarBitacorasAnteriores();

	log_info(logImongo,"Se levanto el file system existente");
}


void cargarSuperBloque(){

	t_config* configSB = config_create(superBloque->path);

	superBloque->block_size = config_get_int_value(configSB,"BLOCK_SIZE");
	superBloque->blocks = config_get_int_value(configSB,"BLOCKS");

	log_info(logImongo,"El valor del block size es: %d", superBloque->block_size);
	log_info(logImongo,"La cantidad de bloques es: %d", superBloque->blocks);

	int sizeBitArrayEnBytes = (int) ceil (((float)superBloque->blocks / (float) 8));
	bitArray = malloc( sizeBitArrayEnBytes);
	superBloque->bitmap = bitarray_create_with_mode(bitArray,sizeBitArrayEnBytes ,MSB_FIRST);

	char* bitmap = config_get_string_value(configSB,"BITMAP");

	int cantidadPosicionesBitArray = bitarray_get_max_bit(superBloque->bitmap);
	for(int i=0; i<cantidadPosicionesBitArray;i++){

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

	config_destroy(configSB);
}


void cargarBlocks(){

	int fd = open(pathBloque,O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
	memoriaSecundaria = mmap(NULL,superBloque->block_size * superBloque->blocks, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);

	lock(&mutexMemoriaSecundaria);
	copiaMemoriaSecundaria= malloc(superBloque->block_size * superBloque->blocks);
	memcpy(copiaMemoriaSecundaria,memoriaSecundaria, superBloque->block_size * superBloque->blocks);
	unlock(&mutexMemoriaSecundaria);

	detallesArchivo(fd);
}



void cargarFile(t_file2* archivo){

	t_config* config = config_create(archivo->path);

	archivo->tamanioArchivo = config_get_long_value(config,"SIZE");
	char** b = config_get_array_value(config,"BLOCKS");
	archivo->bloques = convertirEnLista(b);
	archivo->md5_archivo = config_get_string_value(config,"MD5_ARCHIVO");

	config_destroy(config);

	liberarDoblesPunterosAChar(b);
}


void crearFileSystemDesdeCero(){

	crearSuperBloque();

	if(mkdir(pathFiles,0777) != 0){
		log_info(logImongo, "Hubo un error al crear el directorio %s", pathFiles);
	}

	crearFile(oxigeno);
	crearFile(comida);
	crearFile(basura);

	if(!verificarSiExiste(pathBitacoras)){

		mkdir(pathBitacoras,0777);
	}

	int fd = open(pathBloque,O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);

	crearMemoria(fd);
}


void crearSuperBloque(){

	superBloque->block_size = (uint32_t) config_get_long_value(configImongo,"BLOCK_SIZE");
	superBloque->blocks = (uint32_t)config_get_long_value(configImongo,"BLOCKS");

	int sizeBitArrayEnBytes = (int) ceil (((float)superBloque->blocks / (float) 8));

	bitArray = malloc(sizeBitArrayEnBytes);

	superBloque->bitmap = bitarray_create_with_mode(bitArray,sizeBitArrayEnBytes ,MSB_FIRST);

	int cantidadPosicionesBitArray = bitarray_get_max_bit(superBloque->bitmap);

	log_info(logImongo,"Se inicializo un bitmap con %d posiciones", superBloque->blocks );

	for(int i=0; i<superBloque->blocks;i++){

		bitarray_clean_bit(superBloque->bitmap, i);
	}

	for(int i=superBloque->blocks; i<cantidadPosicionesBitArray ; i++){

		bitarray_set_bit(superBloque->bitmap, i);
	}

	FILE* SB = fopen(superBloque->path,"wb");
	fclose(SB);

	t_config* configSB;

	crearConfig(&configSB,superBloque->path);

	char* superBloqueBlockSize = string_itoa(superBloque->block_size);
	char* superBloqueBlocks = string_itoa(superBloque->blocks);

	config_set_value(configSB,"BLOCK_SIZE",superBloqueBlockSize);
	config_set_value(configSB,"BLOCKS",superBloqueBlocks);

	config_save(configSB);
	config_destroy(configSB);

	free(superBloqueBlockSize);
	free(superBloqueBlocks);

	actualizarSuperBloque();
}


void crearFile(t_file2* archivo){

	FILE* fd = fopen(archivo->path,"wb");
	fclose(fd);

	archivo->tamanioArchivo = 0;
	archivo->bloques = list_create();
	archivo->md5_archivo = string_new();

	actualizarFile(archivo);

	log_info(logImongo,"Se creo el file %s", archivo->path);
}


void iniciarFileSystem(){

	int flag = access(datosConfig->puntoMontaje, F_OK );

	if(flag == -1){
		log_info(logImongo, "No existe el punto de montaje en el directorio %s, "
				"se creara uno", datosConfig->puntoMontaje);
		mkdir(datosConfig->puntoMontaje,0777);
	}
	else if(flag == 0){
		log_info(logImongo, "Existe el punto de montaje en el directorio %s , se procedera "
				"a validar la existencia de SuperBloque.ims y Blocks.ims", datosConfig->puntoMontaje);
	}
	else{
		log_info(logImongo, "La verificacion de si existe la carpeta punto de montaje esta tirando cualquier valor");
		exit(1);
	}
	if(validarExistenciaFileSystem(superBloque->path,pathBloque,datosConfig->puntoMontaje)){

		log_info(logImongo,"Existe un file system actualmente");
		crearFileSystemExistente();
	}
	else{
		log_info(logImongo,"Existe el punto de montaje ahora, pero no existe "
				"SuperBloque.ims y Blocks.ims, creando archivos...");
		crearFileSystemDesdeCero();
	}
}


void liberarSuperBloque(){

	free(superBloque->path);
	bitarray_destroy(superBloque->bitmap);
	free(superBloque);

}


void liberarEstructuraFile(t_file2* file){

	free(file->caracterLlenado);
	free(file->md5_archivo);
	free(file->path);
	list_destroy(file->bloques);
	free(file);

}


void liberarRecursosGlobalesAlTerminar(){

	liberarEstructuraDatosConfig();
	log_destroy(logImongo);
	config_destroy(configImongo);
	liberarEstructuraFile(oxigeno);
	liberarEstructuraFile(basura);
	liberarEstructuraFile(comida);
	dictionary_destroy(bitacoras);
	free(pathFiles);
	free(pathBitacoras);
	free(pathBloque);
	free(copiaMemoriaSecundaria);
	free(memoriaSecundaria);
	list_destroy(listaPosicionesSabotaje);
	free(bitArray);
	for(int i=0;i<6;i++){
		free(tareas[i]);
	}
	free(tareas);
	liberarSuperBloque();

}





