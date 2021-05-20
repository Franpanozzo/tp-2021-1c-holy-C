#include "discordiador.h"


t_config* config;
t_log* logger;
 // Puntero a config donde se va a almacenar el puerto y la IP de Ram y Mongo

puertoEIP* puertoEIPRAM;
puertoEIP* puertoEIPMongo;

int idTripulante = 0;
int idPatota = 0;
t_list* listaDeNew;
t_queue* colaDeReady;


int main() {

	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	listaDeNew = list_create();
	colaDeReady = queue_create();



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

t_patota* asignarDatosAPatota(char* string){

	t_patota* patota = malloc(sizeof(t_patota));

		patota->tamanioTareas = strlen(string) + 1;

		idPatota++;

		patota->ID = idPatota;

		patota->tareas = string;

		return patota;
}


void iniciarPatota(t_coordenadas coordenadas[], char* string, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	t_patota* patota = asignarDatosAPatota(string);

	void* estructura = (void*) patota;

	t_paquete* paquete = armarPaqueteCon(estructura,PATOTA);

	enviarPaquete(paquete,server_socket);

	for (int i=0; i<cantidadTripulantes; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));

		tripulante->posX = coordenadas[i].posX;

		tripulante->posY = coordenadas[i].posY;

		idTripulante++;

		tripulante->idTripulante = idTripulante;

		tripulante->idPatota = patota->ID;

		tripulante->estado = NEW;

		sem_init(&tripulante->semaforo, 0, 0);

		list_add(listaDeNew,(void*)tripulante);

		pthread_t t;
		pthread_create(&t, NULL, (void*) hiloTripulante, (void*) tripulante);
		pthread_detach(t);

	}
}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(sizeof(paquete->buffer->size));

	memcpy(string,&(paquete->buffer->stream),sizeof(paquete->buffer->size));


	return string;
}

void atenderMiRAM(int socketMiRAM,t_tripulante* tripulante) {

    while(1){

    	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

    	char* string = deserializarString(paqueteRecibido);
    	//CUANDO ESTA BLOQUEADO, ES AK?
    	if(paqueteRecibido->codigo_operacion == STRING && string_equals_ignore_case(string, "OK")){

    		queue_push(colaDeReady,(void*) tripulante);




    	}

    	eliminarPaquete(paqueteRecibido);
    	}


}


void hiloTripulante(t_tripulante* tripulante) {

	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM); // Duda porque no se si hay q iniciarla de vuelta ya q la anterior no se cerró

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);

	enviarPaquete(paqueteEnviado, miRAMsocket);

	atenderMiRAM(miRAMsocket,tripulante);

	eliminarPaquete(paqueteEnviado);


}






/*Prepararse (comunicarse con ramInformar al módulo Mi-RAM HQ que desea iniciar, indicando a qué patota pertenece
	Solicitar la primera tarea a realizar.*/


	//Se conecta con discordiador y le dice che toy ready, meteme en lista de ready y actualiza estado

	//While 1
	//sem_wait(semEstructura);
	//Solicitar la primera tarea a realizar

	//Hablar con imongo (haceme esta tarea,modifica este archivo)
	//Funciones varias del discordiador












