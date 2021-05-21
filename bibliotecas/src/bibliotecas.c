#include "bibliotecas.h"


int iniciarConexionDesdeClienteHacia(void* port){ //Este iniciarConexionCon lleva parametro porque puede elegir si conectarse con Mongo o RAM

	puertoEIP* puertoEIPAConectar = (puertoEIP*) port; // Castear parámetro que recibo por referencia

	int server_sock;

	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		//aca hay q hacer log?
		exit(1);
	}
	int yes = 0;

	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt");
		//aca hay q hacer log?
		exit(1);
	}

	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	serverAddress->sin_addr.s_addr = inet_addr(puertoEIPAConectar->IP);
	serverAddress->sin_port = htons((uint16_t)puertoEIPAConectar->puerto);
	serverAddress->sin_family = AF_INET;

	if (connect(server_sock, (struct sockaddr*) serverAddress, sizeof(struct sockaddr_in)) == -1) {
		perror("connect");
		exit(1);
		//aca hay q hacer log? y falta exit?
	}

	free(serverAddress);
	printf("Conectado\n");
	return server_sock;

}


int iniciarConexionDesdeServidor(int puerto) {

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);// Crea socket en la variable local server_sock
	int yes = 1;


	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
	}
	struct sockaddr_in* localAddress = malloc(sizeof(struct sockaddr_in));
	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	localAddress->sin_addr.s_addr = inet_addr("127.0.0.1");
	localAddress->sin_port = htons(puerto);
	localAddress->sin_family = AF_INET;

	if (bind(server_sock, (struct sockaddr*) localAddress, (socklen_t) sizeof(struct sockaddr_in)) == -1) {
		perror("bind");
	}

	if (listen(server_sock, 10) == -1) {
		perror("listen");
	}


	return server_sock;


	free(serverAddress);
	free(localAddress);

}


void liberarConexion(int socket_cliente)
{
	close(socket_cliente);
}

t_log* iniciarLogger(char* archivoLog, char* nombrePrograma, int flagConsola){

	t_log* logger = log_create(archivoLog, nombrePrograma, flagConsola, LOG_LEVEL_INFO);
	//hay q hacer un chequeo de q el programa q se paso es correcto?
	if(logger == NULL){
		printf("No se pudo iniciar el logger del archivo %s perteneciente al programa %s \n"
				, archivoLog, nombrePrograma);
		exit(1);
	}
	else
		return logger;
}

t_paquete* recibirPaquete(int server_socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(server_socket, &(paquete->codigo_operacion), sizeof(tipoDeDato), 0);

	recv(server_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);

	paquete->buffer->stream = malloc(paquete->buffer->size);

	recv(server_socket, paquete->buffer->stream, paquete->buffer->size, 0);

	return paquete;



}

int tamanioEstructura(void* estructura ,tipoDeDato cod_op){

	switch(cod_op){

		case PATOTA:
		{
			t_patota* patota = (t_patota*) estructura;
			return sizeof(uint32_t) * 2 + patota->tamanioTareas;
		}

		case TRIPULANTE:
		{
			return sizeof(uint32_t) * 4 + sizeof(t_estado);
		}


		case TAREA:
		{
			t_tarea* tarea = (t_tarea*) estructura;

			return sizeof(uint32_t) * 5 + strlen(tarea->nombreTarea) + 1;

		}

		default:
				printf("\n No pusiste el tipo de estructura para ver el tamanio negro \n");
				exit(1);
	}

}


t_tarea* deserializarTarea(void* stream){

	int offset = 0;
	t_tarea* tarea = malloc(sizeof(t_tarea));
	uint32_t tamanioNombreTarea = 0;

	memcpy(&(tarea->parametro),stream + offset ,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tarea->posX),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tarea->posY),stream + offset,sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tarea->tiempo),stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(tamanioNombreTarea),stream + offset, sizeof(uint32_t));// este uint no pertenece a la estructura original, OJO!!!!
	offset += sizeof(uint32_t);
	memcpy(tarea->nombreTarea, stream + offset, tamanioNombreTarea);

	return tarea;
}


void* serializarTarea(void* stream, void* estructura, int offset){

	t_tarea* tarea = (t_tarea*) estructura;
	uint32_t tamanioNombreTarea = strlen(tarea->nombreTarea) + 1;
	memcpy(stream + offset, &(tarea->parametro),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tarea->posX),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tarea->posY),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tarea->tiempo),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tamanioNombreTarea),sizeof(uint32_t));// este uint no pertenece a la estructura original, OJO!!!!
	offset += sizeof(uint32_t);
	memcpy(stream + offset, tarea->nombreTarea, tamanioNombreTarea);


    t_tarea* tarea = (t_tarea*) estructura;
    uint32_t tamanioNombreTarea = strlen(tarea->nombreTarea) + 1;
    memcpy(stream + offset, &(tarea->parametro),sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(tarea->posX),sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(tarea->posY),sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(tarea->tiempo),sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(tamanioNombreTarea),sizeof(uint32_t));// este uint no pertenece a la estructura original, OJO!!!!
    offset += sizeof(uint32_t);
    memcpy(stream + offset, tarea->nombreTarea, tamanioNombreTarea);

    return stream;
}


void* serializarPatota(void* stream, void* estructura, int offset){

	t_patota* patota = (t_patota*) estructura;
	memcpy(stream + offset, &(patota->ID),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(patota->tamanioTareas),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, patota->tareas, patota->tamanioTareas);

	return stream;
}

void* serializarTripulante(void* stream, void* estructura, int offset){

	t_tripulante* tripulante = (t_tripulante*) estructura;
	memcpy(stream + offset, &(tripulante->idPatota),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante->idTripulante),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante->estado),sizeof(t_estado));
	offset += sizeof(t_estado);
	memcpy(stream + offset, &(tripulante->posX),sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tripulante->posY),sizeof(uint32_t));
	offset += sizeof(uint32_t);

	return stream;
}


void* serializarEstructura(void* estructura,int tamanio,tipoDeDato cod_op){

	void* stream = malloc(tamanio);

	int offset = 0;

	switch(cod_op){

		case PATOTA:

				return serializarPatota(stream,estructura,offset);

		case TRIPULANTE:

				return serializarTripulante(stream,estructura,offset);

		case TAREA:

			return serializarTarea(stream,estructura,offset);

		default:
				printf("\n No pusiste el tipo de estructura para poder serializar negro \n");
				exit(1);
	}

}


t_paquete* armarPaqueteCon(void* estructura,tipoDeDato cod_op){

	t_paquete* paquete = crearPaquete(cod_op);
	paquete->buffer->size = tamanioEstructura(estructura,paquete->codigo_operacion);
	paquete->buffer->stream = serializarEstructura(estructura,paquete->buffer->size,paquete->codigo_operacion);

	return  paquete;

}


void enviarPaquete(t_paquete* paquete, int socket) {

	int tamanioTotal = paquete->buffer->size + sizeof(tipoDeDato) + sizeof(uint32_t);

	void* a_enviar = serializarPaquete(paquete,tamanioTotal);
	send(socket, a_enviar, tamanioTotal,0);

	free(a_enviar);
	//hay que tener cuidado, definir donde hacer free's y que seamos consitente en el tp
	eliminarPaquete(paquete);
}



void* serializarPaquete(t_paquete* paquete, int bytes){

	void * magic = malloc(bytes);

	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(tipoDeDato));
	desplazamiento+= sizeof(tipoDeDato);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento+= sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}


void lock(pthread_mutex_t mutex){

    pthread_mutex_lock(&mutex);
}


void unlock(pthread_mutex_t mutex){

    pthread_mutex_unlock(&mutex);
}



void crearBuffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


t_paquete* crearPaquete(tipoDeDato cod_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crearBuffer(paquete);
	return paquete;
}


void eliminarPaquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}



