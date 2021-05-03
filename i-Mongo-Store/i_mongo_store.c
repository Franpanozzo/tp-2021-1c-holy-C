#include "i_mongo_store.h"

#define MAX_BUFFER_SIZE  200
#define PORT 5001 // Define el nuemero de puerto que va a tener mongo
int discordiador_socket; // Entero donde se guarda el el valor de accept del socket
t_persona paraEnviar;
t_persona*  paraRecibir;
t_buffer* buffer;


int main(void) {

	paraEnviar.dni = 88;
	paraEnviar.edad = 33;
	paraEnviar.nombre = "Mr. Lacteo";
	paraEnviar.nombre_length = strlen(paraEnviar.nombre) + 1;
	paraEnviar.pasaporte = 8888881;

	iniciarConexion();
	enviarPaquete(paraEnviar);

	//printf("Si tuvimos exito se va a leer algo a continuacion: %d",paraRecibir->dni);

	close(discordiador_socket);

	return EXIT_SUCCESS;

}


void iniciarConexion() {

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);// Crea socket en la variable local server_sock
	unsigned int len = sizeof(struct sockaddr); // Crea un entero sin signo que almacena la cantidad de bytes que ocupa la estructura sockaddr
	int yes = 1;


	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
	}
	struct sockaddr_in* localAddress = malloc(sizeof(struct sockaddr_in));
	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	localAddress->sin_addr.s_addr = inet_addr("127.0.0.1");
	localAddress->sin_port = htons(PORT);
	localAddress->sin_family = AF_INET;

	if (bind(server_sock, (struct sockaddr*) localAddress, (socklen_t) sizeof(struct sockaddr_in)) == -1) {
		perror("bind");
	}

	if (listen(server_sock, 10) == -1) {
		perror("listen");
	}

	if ((discordiador_socket = accept(server_sock, (struct sockaddr*) serverAddress, &len)) == -1) {
		perror("accept");
	}



	free(serverAddress);
	free(localAddress);

}


void recibirPaquete(){

t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(discordiador_socket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);

	recv(discordiador_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(discordiador_socket, paquete->buffer->stream, paquete->buffer->size, 0);

	switch(paquete->codigo_operacion) {
		case PERSONA:
			paraRecibir = deserializar_persona(paquete->buffer);
								break;
		default:
				break;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


t_persona* deserializar_persona(t_buffer* buffer) {

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


void enviarPaquete(t_persona persona) {

	int offset = 0;

	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 3 + sizeof(uint8_t) + strlen(persona.nombre) + 1; // La longitud del string nombre.

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &persona.dni, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &persona.edad, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &persona.pasaporte, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &persona.nombre_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, persona.nombre, strlen(persona.nombre) + 1);

	buffer->stream = stream;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PERSONA;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));

	offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	send(discordiador_socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t),
	0);


	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

