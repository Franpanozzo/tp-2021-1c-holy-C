#include "discordiador.h"

#define MAX_BUFFER_SIZE  200
sem_t sem_conexion; // Variable de tipo semaforo
int client_socket; // Entero donde se va a almacenar el socket cliente del discordiador
t_config* config; // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

int main() {

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIP* puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIP* puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	pthread_t h1; // Iniciliazar hilo nombrado h1
	sem_init(&sem_conexion, 0, 0); // Iniciar semaforo

	pthread_create(&h1, NULL, (void*) iniciar_conexion,(void*) puertoEIPMongo); // Crear hilo con la funcion iniciar_conexion
	comunicarse(); // Funcion para comunicación tipo chat con la conexión al puerto creado anteriormente

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}

t_config* crearConfig(){
	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");
	if(config == NULL){
		printf("\nEsta mal la ruta del config negro\n");
		exit(1);
		}
	return config;
	}

void iniciar_conexion(void* port) {

	puertoEIP* puertoEIPAConectar = (puertoEIP*) port; // Castear parámetro que recibo por referencia
	char buffer[MAX_BUFFER_SIZE]; // Reservar buffer de memoria del define que esta al comienzo
	int server_sock; // Declarar 2 veces??? *

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

	client_socket = server_sock; // *
	sem_post(&sem_conexion); // Fin semáforo sem_conexion


	while (1) {
		memset(buffer, '\0', MAX_BUFFER_SIZE); // Rellena con /0 lo que queda del vector
		int recvd = recv(client_socket, buffer, sizeof(int), MSG_WAITALL);
		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(client_socket);
			break;
		}
		// No entiendo bien porque hace 2 veces esto, o sea, copia a 2 arrays distintos usando memcpy
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
	sem_wait(&sem_conexion);// Espera que termine de conectarse para iniciar la función
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
		free(mensaje);
	}

	if (cadena != NULL)
		free(cadena);
}
