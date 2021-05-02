#include "discordiador.h"

#define MAX_BUFFER_SIZE  200
sem_t sem_conexion; // Variable de tipo semaforo
int server_socket; // Entero donde se va a almacenar el socket cliente del discordiador
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

	pthread_create(&h1, NULL, (void*) iniciar_conexion,(void*) puertoEIPRAM); // Crear hilo con la funcion iniciar_conexion
	comunicarse(); // Funcion para comunicación tipo chat con la conexión al puerto creado anteriormente
	pthread_join(h1, (void**) NULL);

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

void iniciar_conexion(void* port) {

	puertoEIP* puertoEIPAConectar = (puertoEIP*) port; // Castear parámetro que recibo por referencia
	//char buffer[MAX_BUFFER_SIZE]; // Reservar buffer de memoria del define que esta al comienzo
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

	server_socket = server_sock; // *
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

		memset(buffer, '\0', MAX_BUFFER_SIZE); // Rellena con /0 lo que queda del vector

		int recvd = recv(server_socket, buffer, sizeof(int), MSG_WAITALL);

		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(server_socket);
			break;
		}
		int lenCadena;

		memcpy(&lenCadena, buffer, sizeof(int));

		recvd = recv(server_socket, buffer, lenCadena, MSG_WAITALL);
		if (recvd <= 0) {
			if (recvd == -1) {
				perror("recv");
			}

			close(server_socket);
			break;
		}

		printf("Ha recibido del servidor el siguiente mensaje: %s \n", buffer);
		*/
	}
	free(serverAddress);

}

void comunicarse() {
	sem_wait(&sem_conexion);


	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->contenidoMensaje = strdup("Juan hace zumbita");
	mensaje->longitud = strlen(mensaje->contenidoMensaje) + 1;

	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) + mensaje->longitud;
	buffer->stream = malloc(buffer->size);

	int offset = 0;
	memcpy(buffer->stream, &(mensaje->longitud) , sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer->stream + offset, mensaje->contenidoMensaje, mensaje->longitud);

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = STRING;
	paquete->buffer = buffer;

	//Empieza la parte de meter to:do en bloque de bytes

	void* a_enviar = malloc(sizeof(uint8_t) + sizeof(paquete->buffer->size) + paquete->buffer->size);

	offset = 0;
	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);


	if(send(server_socket,a_enviar, sizeof(uint8_t) + sizeof(paquete->buffer->size) + paquete->buffer->size, 0) == -1){
		perror("send");
		exit(1);
	}

	free(mensaje->contenidoMensaje);
	free(mensaje);
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	close(server_socket);
}
