#include "discordiador.h"


t_config* config;
t_log* logger;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante = 0;
t_list* listaDeNew;


int main() {

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	listaDeNew = list_create();

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	char* tarea = strdup("GENERAR_OXIGENO 3;2;2;3  \nDESCARTAR_BASURA 2;4;5;5");

	t_paquete* paquete = armarPaqueteCon((void*) tarea,TAREA_PATOTA);

	enviarPaquete(paquete,server_socket);

	pthread_t consola;
	pthread_create(&consola, NULL, (void*) leerConsola, NULL);
	pthread_join(consola);

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	free(tarea);


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

	for (int i=0; i<cantidadTripulantes; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));

		tripulante->posX = coordenadas[i].posX;

		tripulante->posY = coordenadas[i].posY;

		idTripulante++;

		tripulante->ID = idTripulante;

		tripulante->estado = NEW;

		sem_init(&tripulante->semaforo, 0, 0);

		list_add(listaDeNew,(void*)tripulante);


		pthread_t t;
		pthread_create(&t, NULL, (void*) hilo_tripulante, (void*) tripulante);
		pthread_detach(t);

	}
}

void hilo_tripulante(t_tripulante* triuplante) {

	/*Prepararse (comunicarse con ramInformar al módulo Mi-RAM HQ que desea iniciar, indicando a qué patota pertenece
	Solicitar la primera tarea a realizar.*/


	//Se conecta con discordiador y le dice che toy ready, meteme en lista de ready y actualiza estado

	//While 1
	//sem_wait(semEstructura);
	//Solicitar la primera tarea a realizar

	//Hablar con imongo (haceme esta tarea,modifica este archivo)
	//Funciones varias del discordiador
}












