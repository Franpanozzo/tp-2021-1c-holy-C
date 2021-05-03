#include "discordiador.h"

#define MAX_BUFFER_SIZE  200
sem_t sem_conexion; // Variable de tipo semaforo
int server_socket; // Entero donde se va a almacenar el socket cliente del discordiador
t_config* config; // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo
t_persona persona;


int main() {

	persona.dni = 33238307;
	persona.edad = 33;
	persona.nombre = "Mr. Lacteo";
	persona.nombre_length = strlen(persona.nombre) + 1;
	persona.pasaporte = 8888881;

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
	comunicarse(persona); // Funcion para comunicación tipo chat con la conexión al puerto creado anteriormente
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
	sem_post(&sem_conexion); // Fin semáforo sem_conexion

	free(serverAddress);

}

void comunicarse(t_persona persona) {
	sem_wait(&sem_conexion);

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

	send(server_socket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t),
	0);


	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);

	close(server_socket);
}
