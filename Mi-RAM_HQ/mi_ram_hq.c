#include "mi_ram_hq.h"

#define MAX_BUFFER_SIZE  200
#define PORT 3222 // Define el nuemero de puerto que va a tener mongo
sem_t sem_conexion; // Iniciar una variable tipo semaforo
pthread_mutex_t mutex_queue; // Iniciar mutex (mutua exclusion)
int discordiador_socket; // Entero donde se guarda el el valor de accept del socket

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

	return EXIT_SUCCESS;

}

void iniciar_conexion() {
	// Crea un buffer tipo char con la capacidad del vector de MAX_BUFFER_SIZE
	int server_sock = socket(AF_INET, SOCK_STREAM, 0);// Crea socket en la variable local server_sock
	unsigned int len = sizeof(struct sockaddr); // Crea un entero sin signo que almacena la cantidad de bytes que ocupa la estructura sockaddr
	int yes = 0;

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

	while (1) {

		t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->buffer = malloc(sizeof(t_buffer));

		int recvd = recv(discordiador_socket, &(paquete->codigo_operacion), sizeof(uint8_t), MSG_WAITALL);
		if(recvd <= 0){
			if(recvd == -1){
				perror("recv");
			}
			close(discordiador_socket);
		}

		recvd = recv(discordiador_socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
		if(recvd <= 0){
			if(recvd == -1){
				perror("recv");
			}
			close(discordiador_socket);
		}

		paquete->buffer->stream = malloc(paquete->buffer->size);
		recvd = recv(discordiador_socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);
		if(recvd <= 0){
			if(recvd == -1){
				perror("recv");
			}
			close(discordiador_socket);
		}

		int offset = 0;
		void* stream = paquete->buffer->stream;
		tipoDeDato codigoDeOp;
		memcpy(&codigoDeOp, stream, sizeof(uint8_t));
		stream += sizeof(uint8_t);

		//Aca llamariamos a la funcion deserializar segun codOperacion

		char* mensajeRecibido;
		int longitud;

		memcpy(&longitud, stream, sizeof(int));
		offset += sizeof(int);
		mensajeRecibido = malloc(longitud);
		memcpy(mensajeRecibido, stream + offset, longitud);


		printf("Recibi de la conexion el siguiente mensaje: %s", mensajeRecibido);

		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);

	}

	free(serverAddress);
	free(localAddress);
	close(discordiador_socket);
}

void comunicarse() {
	/*
	sem_wait(&sem_conexion); // Espera que se conecte para poder comunicarse
	char* cadena = malloc(MAX_BUFFER_SIZE);
	while (fgets(cadena, MAX_BUFFER_SIZE, stdin) != NULL) {
		char* mensaje = malloc(sizeof(int) + strlen(cadena) + 1); //serializamos "on-the fly" un int junto con la cadena para que esta pueda ser de tamaño variable
		int len = strlen(cadena) + 1;
		int tmpSize = 0;
		memcpy(mensaje, &len, tmpSize = sizeof(int));
		memcpy(mensaje + tmpSize, cadena, strlen(cadena) + 1);

		if (send(discordiador_socket, mensaje, len + tmpSize, MSG_NOSIGNAL) <= 0)
			close(discordiador_socket);
	}

	if (cadena != NULL)
		free(cadena);
		*/
}
