#include "discordiador.h"


int main() {

	idTripulante = 0;
	idPatota = 0;


	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/logDisocridador.log","Discordiador",1);
	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	puertoEIPRAM = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Ram
	puertoEIPMongo = malloc(sizeof(puertoEIP)); // Reservar memoria para struct Mongo

	puertoEIPRAM->puerto = config_get_int_value(config,"PUERTO_MI_RAM_HQ"); // Asignar puerto tomado desde el config (RAM)
	puertoEIPRAM->IP = strdup(config_get_string_value(config,"IP_MI_RAM_HQ")); // Asignar IP tomada desde el config (RAM)

	puertoEIPMongo->IP = strdup(config_get_string_value(config,"IP_I_MONGO_STORE")); // Asignar IP tomada desde el config (Mongo)
	puertoEIPMongo->puerto = config_get_int_value(config,"PUERTO_I_MONGO_STORE"); // Asignar puerto tomado desde el config (Mongo)

	listaDeNew = list_create();
	colaDeReady = queue_create();

	pthread_mutex_init(&mutexListaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);

	char* tarea = strdup("GENERAR_OXIGENO 4;5;6;7\nGENERAR_YOGUR 4;5;6;7\n");

		t_coordenadas coordenadas[1];

		for(int i = 0; i<1 ;i++) {

			coordenadas[i].posX = i;
			coordenadas[i].posY = i+ 1;
		}


	iniciarPatota(coordenadas, tarea, 1);

	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);



	return EXIT_SUCCESS;

}


void crearConfig(){
	config  = config_create("/home/utnso/tp-2021-1c-holy-C/Discordiador/discordiador.config");
	if(config == NULL){
		log_error(logDiscordiador, "\n La ruta es incorrecta \n");
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

		sem_init(&tripulante->semaforo, 0, 0);

		list_add(listaDeNew,(void*)tripulante);

		//esto va en detach
		pthread_t t;
		pthread_create(&t, NULL, (void*) hiloTripulante, (void*) tripulante);
		pthread_join(t, (void**) NULL);

	}

	close(server_socket);
}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(sizeof(paquete->buffer->size));

	memcpy(string,&(paquete->buffer->stream),sizeof(paquete->buffer->size));

	return string;
}



void atenderMiRAM(int socketMiRAM,t_tripulante* tripulante) {

    	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

    	t_tripulante* tripulanteParaCheckear;

    	t_tarea* tarea = deserializarTarea(paqueteRecibido->buffer->stream);

    	printf("Soy el tripulante %d y recibi la tarea de: %s \n",tripulante->idTripulante,tarea->nombreTarea);

    	if(paqueteRecibido->codigo_operacion == TAREA){

    		bool idIgualA(t_tripulante* tripulanteAcomparar){

    			return tripulanteAcomparar->idTripulante == tripulante->idTripulante;
    		}

    		lock(mutexListaNew);
    		if((tripulanteParaCheckear = list_remove_by_condition(listaDeNew, (void*) idIgualA)) != NULL){
    		unlock(mutexListaNew);

    		tripulante->instruccionAejecutar = tarea;

    		queue_push(colaDeReady,(void*) tripulanteParaCheckear);

    		}

    		else{

    			printf("Estas queriendo meter a Ready un NULL negro\n");
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
