#include "mi_ram_hq.h"

#define MAX_BUFFER_SIZE  200
#define PORT 3222 // Define el nuemero de puerto que va a tener mongo
sem_t sem_conexion; // Iniciar una variable tipo semaforo
pthread_mutex_t mutex_queue; // Iniciar mutex (mutua exclusion)
int discordiador_socket; // Entero donde se guarda el el valor de accept del socket
t_persona* persona;
t_buffer* buffer;
/*
 * Para evitar el uso de un socket global, se podria implementar una cola de mensajes
 * entre el hilo que recibe de consola y el hilo de comunicaciones asi este es el unico que se relaciona con el socket
 * Para esto es necesario aplicarle locks al TAD de queue actual.
 */

int main(void) {

	pthread_t h1; // Crear variable tipo hilo

	sem_init(&sem_conexion, 0, 0); // Iniciar semaforo sem_conexion

	pthread_mutex_init(&mutex_queue, NULL); // Iniciar mutex declarado en las variables globales

	pthread_create(&h1, NULL, (void*) iniciar_conexion, NULL); // Crear hilo con la funcion casteada iniciar_conexion

	//comunicarse(); // Hace un chat abierto con el cliente, en este caso Discordiador

	pthread_join(h1, (void**) NULL); // Realiza un orden en los hilos, en este caso, h1

	printf("Si tuvimos exito se va a leer algo a continuacion: %d",persona->dni);

	return EXIT_SUCCESS;

}

void iniciar_conexion() {
	// Crea un buffer tipo char con la capacidad del vector de MAX_BUFFER_SIZE
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

	sem_post(&sem_conexion); // Finalizar semaforo sem_conexion

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(discordiador_socket, &(paquete->codigo_operacion), sizeof(uint8_t), 0);

	recv(discordiador_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(discordiador_socket, paquete->buffer->stream, paquete->buffer->size, 0);

	switch(paquete->codigo_operacion) {
		case PERSONA:
			persona = deserializar_persona(paquete->buffer);
								break;
		default:
				break;
	}

	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);




	free(serverAddress);
	free(localAddress);
	close(discordiador_socket);

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

