#include "discordiador.h"

#define MAX_BUFFER_SIZE  200
sem_t sem_conexion; // Variable de tipo semaforo
int discordiador_socket; // Entero donde se va a almacenar el socket cliente del discordiador
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

	discordiador_socket = server_sock; // *
	sem_post(&sem_conexion); // Fin semáforo sem_conexion


	while (1) {
		/*
			 * Para no tener perdida de datos se usa el flag MSG_WAITALL que lo que hace es esperar a que el recv llene el buffer
			 * Con size bytes. Al poner este flag no se puede hacer que espere MAX_SIZE_BUFFER pq sino hasta que manden
			 * esa cantidad el recv quedara colgado.
			 *
			 * Es por eso que se hace UN UNICO ENVIO con un int y una cadena, consumiendo primero el int que representa
			 * la cantidad de bytes que se deben recibir en la cadena.
			 *
			 * Es importante recalcar que esto no es fraccionar un paquete, ya que se hace un unico envio y este se consume en 2 llamadas.
			 * Si hicieramos 2 send (uno con el int y otro con la cadena) estaria mal desde el punto de vista de redes ya que habria
			 * fraccionamiento de paquetes (cuando un paquete es una unidad indivisible que se transmite por la red)
			 */
		memset(buffer, '\0', MAX_BUFFER_SIZE); // Rellena con /0 lo que queda del vector

		int recvd = recv(discordiador_socket, buffer, sizeof(int), MSG_WAITALL);

		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(discordiador_socket);
			break;
		}
		int lenCadena;

		memcpy(&lenCadena, buffer, sizeof(int));

		recvd = recv(discordiador_socket, buffer, lenCadena, MSG_WAITALL);
		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(discordiador_socket);
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

		if (send(discordiador_socket, mensaje, len + tmpSize, MSG_NOSIGNAL) <= 0) {
			close(discordiador_socket);
		}
		free(mensaje);
	}

	if (cadena != NULL)
		free(cadena);
}
