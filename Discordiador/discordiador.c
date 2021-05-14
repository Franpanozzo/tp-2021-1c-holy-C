#include "discordiador.h"


t_config* config;
t_log* logger;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;


int main() {

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)


	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);



	return EXIT_SUCCESS;

}


void crearConfig(){
	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");
	if(config == NULL){
		log_error(logger, "\n La ruta es incorrecta \n");
		exit(1);
		}
}


void iniciarPatota(t_coordenadas coordenadas[], char* string, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	void* estructura = (void*) string;

	t_paquete* paquete = armarPaqueteCon(estructura,TAREA_PATOTA);

	enviarPaquete(paquete,server_socket);
}



