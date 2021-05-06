#include "discordiador.h"
#include <bibliotecas_nuestras.h>


#define MAX_BUFFER_SIZE  200
int server_socket; // Entero donde se va a almacenar el socket cliente del discordiador
t_config* config; // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo
t_persona*  paraRecibir;
t_persona paraEnviar;


int main() {

	paraEnviar.dni = 33238307;
	paraEnviar.edad = 33;
	paraEnviar.nombre = "Mr. Lacteo";
	paraEnviar.nombre_length = strlen(paraEnviar.nombre) + 1;
	paraEnviar.pasaporte = 8888881;

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIP* puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIP* puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	t_persona* cosa = malloc(sizeof(t_persona));
	*cosa = paraEnviar;
	void* cosaQsePuedeMandar = (void*)cosa;

	iniciarConexionCon(puertoEIPMongo);

	t_paquete* paquete = armarPaqueteCon(cosaQsePuedeMandar,PERSONA);
	enviarPaquete(paquete);

	//printf("Si tuvimos exito se va a leer algo a continuacion: %d",paraRecibir->dni);
	free(cosa);
	close(server_socket);
	//eliminarPaquete(paquete);
	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}


void crearConfig(){
	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");
	if(config == NULL){
		printf("\nEsta mal la ruta del config negro\n");
		exit(1);
		}
	}


void iniciarConexionCon(void* port){ //Este iniciarConexionCon lleva parametro porque puede elegir si conectarse con Mongo o RAM

	puertoEIP* puertoEIPAConectar = (puertoEIP*) port; // Castear parámetro que recibo por referencia

	int server_sock;

	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);// *
	}
	int yes = 0;

	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt");
		exit(1); // *
	}

	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	serverAddress->sin_addr.s_addr = inet_addr(puertoEIPAConectar->IP);
	serverAddress->sin_port = htons((uint16_t)puertoEIPAConectar->puerto);
	serverAddress->sin_family = AF_INET;

	if (connect(server_sock, (struct sockaddr*) serverAddress, sizeof(struct sockaddr_in)) == -1) {
		perror("connect");
	}

	server_socket = server_sock; // *


	free(serverAddress);

}


int tamanioEstructura(void* estructura ,tipoDeDato cod_op){

	t_persona* persona;


	switch(cod_op){
		case PERSONA:
					persona = (void*) estructura;
					return  sizeof(uint32_t) * 3 + sizeof(uint8_t) + strlen(persona->nombre) + 1;


		default:
				printf("\n No pusiste el tipo de estructura para ver el tamanio negro \n");
				exit(1);
	}

}


void* serializarEstructura(void* estructura,int tamanio,tipoDeDato cod_op){

	t_persona* persona;

	void* stream = malloc(tamanio);

	int offset = 0;

	switch(cod_op){
		case PERSONA:
				persona = (void*) estructura;
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


void enviarPaquete(t_paquete* paquete) {
	int tamanioTotal = paquete->buffer->size + sizeof(tipoDeDato) + sizeof(uint32_t);
	void* a_enviar = serializarPaquete(paquete,tamanioTotal);
	send(server_socket, a_enviar, tamanioTotal,
	0);

	free(a_enviar);
	eliminarPaquete(paquete);
}


void recibirPaquete(){
t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(server_socket, &(paquete->codigo_operacion), sizeof(tipoDeDato), 0);

	recv(server_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(server_socket, paquete->buffer->stream, paquete->buffer->size, 0);

	switch(paquete->codigo_operacion) {
		case PERSONA:
			paraRecibir = deserializarPersona(paquete->buffer);
								break;
		default:
				break;
	}

	eliminarPaquete(paquete);
}


t_persona* deserializarPersona(t_buffer* buffer) {

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

	return persona;
}


void* serializarPaquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
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


void liberarConexion(int socket_cliente)
{
	close(socket_cliente);
}

