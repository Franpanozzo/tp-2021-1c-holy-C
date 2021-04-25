#include "discordiador.h"

#define MAX_BUFFER_SIZE  200
#define PORTRAM 3200
#define PORTMONGO 5001
sem_t sem_conexion;
pthread_mutex_t mutex_queue;
int client_socket;

int main() {

	pthread_t h1;
	sem_init(&sem_conexion, 0, 0);
	pthread_mutex_init(&mutex_queue, NULL);
	pthread_create(&h1, NULL, (void*) iniciar_conexion,(void*) PORTRAM);
	comunicarse();
	pthread_join(h1, (void**) NULL);

	return EXIT_SUCCESS;
}

void iniciar_conexion(int port) {

	char buffer[MAX_BUFFER_SIZE];

	int server_sock = socket(AF_INET, SOCK_STREAM, 0);
	int yes = 0;

	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	serverAddress->sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress->sin_port = htons(port);
	serverAddress->sin_family = AF_INET;

	if (connect(server_sock, (struct sockaddr*) serverAddress, sizeof(struct sockaddr_in)) == -1) {
		perror("connect");
	}

	client_socket = server_sock;
	sem_post(&sem_conexion);

	while (1) {
		memset(buffer, '\0', MAX_BUFFER_SIZE);
		int recvd = recv(client_socket, buffer, sizeof(int), MSG_WAITALL);
		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(client_socket);
			break;
		}

		int lenCadena;
		memcpy(&lenCadena, buffer, sizeof(int));
		recvd = recv(client_socket, buffer, lenCadena, MSG_WAITALL);
		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(client_socket);
			break;
		}

		printf("Ha recibido del servidor el siguiente mensaje: %s \n", buffer);
	}
	free(serverAddress);

}

void comunicarse() {
	sem_wait(&sem_conexion);
	char* cadena = malloc(MAX_BUFFER_SIZE);
	while (fgets(cadena, MAX_BUFFER_SIZE, stdin) != NULL) {
		char* mensaje = malloc(sizeof(int) + strlen(cadena) + 1);
		int len = strlen(cadena) + 1;
		int tmpSize = 0;
		memcpy(mensaje, &len, tmpSize = sizeof(int));
		memcpy(mensaje + tmpSize, cadena, strlen(cadena) + 1);

		if (send(client_socket, mensaje, len + tmpSize, MSG_NOSIGNAL) <= 0) {
			close(client_socket);
		}
	}

	if (cadena != NULL)
		free(cadena);
}
