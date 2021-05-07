#include "discordiador.h"
//#include <bibliotecas.h>


#define MAX_BUFFER_SIZE  200
t_config* config; // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo
t_persona*  paraRecibir;
t_persona paraEnviar;


int main() {
	int server_socket; // Entero donde se va a almacenar el socket cliente del discordiador

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

	server_socket = iniciarConexionCon(puertoEIPMongo);

	t_paquete* paquete = armarPaqueteCon(cosaQsePuedeMandar,PERSONA);
	enviarPaquete(paquete, server_socket);



	//printf("Si tuvimos exito se va a leer algo a continuacion: %d",paraRecibir->dni);
	//estos
	free(cosa);


	close(server_socket);
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


int iniciarConexionCon(void* port){ //Este iniciarConexionCon lleva parametro porque puede elegir si conectarse con Mongo o RAM

	puertoEIP* puertoEIPAConectar = (puertoEIP*) port; // Castear parámetro que recibo por referencia

	int server_sock;

	if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	int yes = 0;

	if(setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		perror("setsockopt");
		exit(1);
	}

	struct sockaddr_in* serverAddress = malloc(sizeof(struct sockaddr_in));

	serverAddress->sin_addr.s_addr = inet_addr(puertoEIPAConectar->IP);
	serverAddress->sin_port = htons((uint16_t)puertoEIPAConectar->puerto);
	serverAddress->sin_family = AF_INET;

	if (connect(server_sock, (struct sockaddr*) serverAddress, sizeof(struct sockaddr_in)) == -1) {
		perror("connect");
	}

	free(serverAddress);

	return server_sock;

}




