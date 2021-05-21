#include "discordiador.h"



int main() {
	idTripulante = 0;
	idPatota = 0;

	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador"
			, "Discordiador", 1);

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

	t_paquete* paquete = armarPaqueteCon((void*) patota,PATOTA);

	enviarPaquete(paquete,server_socket);

	for (int i=0; i<cantidadTripulantes; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));

		tripulante->posX = coordenadas[i].posX;

		tripulante->posY = coordenadas[i].posY;

		idTripulante++;

		tripulante->idTripulante = idTripulante;

		tripulante->idPatota = patota->ID;

		tripulante->estado = NEW;

		t_log* bitacora = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador"
					, "Discordiador", 1);

		sem_init(&tripulante->semaforo, 0, 0);

		//no hace falta el mutex aca pero bueno sha fue
		lock(mutexListaNew);
		list_add(listaDeNew,(void*)tripulante);
		unlock(mutexListaNew);

		pthread_t t;
		pthread_create(&t, NULL, (void*) hiloTripulante, (void*) tripulante);
		pthread_detach(t);

	}
}


void atenderMiRAM(int socketMiRAM,t_tripulante* tripulante) {

    	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

    	t_tripulante* tripulanteParaCheckear;

    	t_tarea* tarea = deserializarTarea(paqueteRecibido);

    	if(paqueteRecibido->codigo_operacion == TAREA){

    		bool idIgualA(t_tripulante* tripulanteAcomparar){

    			return tripulanteAcomparar->idTripulante == tripulante->idTripulante;
    		}

    		lock(mutexListaNew);
    		tripulanteParaCheckear = list_remove_by_condition(listaDeNew, (void*) idIgualA);
    		unlock(mutexListaNew);


    		if(tripulanteParaCheckear != NULL){

				tripulante->instruccionAejecutar = tarea;

				queue_push(colaDeReady,(void*) tripulanteParaCheckear);

    		}

    		else{
    			log_info(logDiscordiador, "Estas queriendo meter a Ready un NULL negro\n");
    			exit(1);

    		}
    	}
}


void hiloTripulante(t_tripulante* tripulante) {

	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM); // Duda porque no se si hay q iniciarla de vuelta ya q la anterior no se cerró

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);

	enviarPaquete(paqueteEnviado, miRAMsocket);

	atenderMiRAM(miRAMsocket,tripulante);

	eliminarPaquete(paqueteEnviado);


}
