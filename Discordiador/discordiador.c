#include "discordiador.h"


t_config* config;
t_log* logger;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo
int server_socket; // Entero donde se va a almacenar el socket cliente del discordiador


int main() {

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram
	logger = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/prueba.log", "Discordiador", 1);

	log_info(logger, "hola 2");

	puertoEIP* puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIP* puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)



	log_info(logger, "hola");

	t_paquete* paquete;

	t_persona persona;

	persona.nombre = strdup("Lechoso");
	persona.dni = 30256897;
	persona.edad = 21;
	persona.nombre_length = strlen(persona.nombre) + 1;
	persona.pasaporte = 30256897;

	t_persona* punteroApersona = malloc(sizeof(persona));
	*punteroApersona = persona;
	void* cosa = (void*) punteroApersona;

	server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	paquete = armarPaqueteCon(cosa,PERSONA);
	enviarPaquete(paquete,server_socket);

	free(punteroApersona->nombre);
	free(punteroApersona);
	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	log_destroy(logger);

	return EXIT_SUCCESS;

}


void crearConfig(){
	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");
	if(config == NULL){
		log_error(logger, "\n La ruta es incorrecta \n");
		exit(1);
		}
}




