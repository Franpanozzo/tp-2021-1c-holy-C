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

	return server_sock;

}


void iniciarConexionDesdeServidor(int *discordiador_socket, int puerto) {

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);// Crea socket en la variable local server_sock
	unsigned int len = sizeof(struct sockaddr); // Crea un entero sin signo que almacena la cantidad de bytes que ocupa la estructura sockaddr
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

	if ((*discordiador_socket = accept(server_sock, (struct sockaddr*) serverAddress, &len)) == -1) {
		perror("accept");
	}


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


void deserializarSegun(t_paquete* paquete){

	switch(paquete->codigo_operacion){

			case PERSONA:
						deserializarPersona(paquete->buffer);
						break;


			default:
					printf("\n No se puede deserializar ese tipo de estructura negro \n");
					exit(1);
		}
}

void deserializarPersona(t_buffer* buffer) {

	t_persona* persona = malloc(sizeof(t_persona));

	void* stream = buffer->stream;

	memcpy(&(persona->dni), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(persona->edad), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(persona->pasaporte), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(persona->nombre_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	persona->nombre = malloc(persona->nombre_length);
	memcpy(persona->nombre, stream, persona->nombre_length);

	printf("El nombre de la persona es: %s \n",persona->nombre);
	printf("El DNI de la persona es: %d \n",persona->dni);

	free(persona->nombre);
	free(persona);
}


int tamanioEstructura(void* estructura ,tipoDeDato cod_op){

	switch(cod_op){
		case PERSONA:
		{
			t_persona* persona = (void*) estructura;
			return  sizeof(uint32_t) * 3 + sizeof(uint8_t) + strlen(persona->nombre) + 1;
		}



		default:
				printf("\n No pusiste el tipo de estructura para ver el tamanio negro \n");
				exit(1);
	}

}


void* serializarPersona(void* stream, void* estructura,  int offset){

	t_persona* persona = (void*) estructura;
	memcpy(stream + offset, &(persona->dni), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(persona->edad), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &(persona->pasaporte), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(persona->nombre_length), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, persona->nombre, strlen(persona->nombre) + 1);

	return stream;
}




void* serializarEstructura(void* estructura,int tamanio,tipoDeDato cod_op){

	void* stream = malloc(tamanio);

	int offset = 0;

	switch(cod_op){
		case PERSONA:

				return serializarPersona(stream,estructura, offset);

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



