#include "discordiador.h"


int main() {

	idTripulante = 0;
	idPatota = 0;

	logDiscordiador = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador/logDiscordiador.log","Discordiador",1);

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
/*
	int serverImongo = iniciarConexionDesdeClienteHacia(puertoEIPMongo);

	t_patota* patotaPrueba = asignarDatosAPatota("GENERAR_OXIGENO 4;5;6;7\nGENERAR_COMIDA;5;6;7\n");

	t_paquete*paquetePatota = armarPaqueteCon((void*) patotaPrueba,PATOTA);

	enviarPaquete(paquetePatota,serverImongo);
*/

/*	char* tarea = strdup("GENERAR_OXIGENO 4;5;6;7\nGENERAR_COMIDA;5;6;7\n");

	t_coordenadas coordenadas[4];

	for(int i = 0; i<4 ;i++) {

		coordenadas[i].posX = i;
		coordenadas[i].posY = i+ 1;
		}

	iniciarPatota(coordenadas, tarea, 4);
*/

	leerConsola();


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


void eliminarPatota(t_patota* patota){
	free(patota->tareas);
	free(patota);
}


t_patota* asignarDatosAPatota(char* tareasString){

	t_patota* patota = malloc(sizeof(t_patota));

	patota->tamanioTareas = strlen(tareasString) + 1;

	idPatota++;

	log_info(logDiscordiador,"Se creo la patota numero %d\n",idPatota);

	patota->ID = idPatota;

	//aca no seria mejor hacer un strdup y hacer un free de tareasString??
	// asi despues no perdemos referencia de tareasString para hacer el free
	// o es que despues se puede hacer free(patota->tareas)??
	//no estaria haciendo un free a una posicion que ya esta apuntada por otro puntero?
	patota->tareas = tareasString;

	return patota;
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	t_patota* patota = asignarDatosAPatota(tareasString);

	t_paquete* paquete = armarPaqueteCon((void*) patota,PATOTA);

	enviarPaquete(paquete,server_socket);

	//lo que esta dentro del for no se podria abstraer en una funcion que sea iniciarTripulante?
	//separa la logica de iniciar la patota y la de inciar el tripunlante.
	//Ademas hace mas expresivo el codigo y la funcion no seria tan larga

	for (int i=0; i<cantidadTripulantes; i++){

		t_tripulante* tripulante = malloc(sizeof(t_tripulante));

		tripulante->posX = coordenadas[i].posX;

		tripulante->posY = coordenadas[i].posY;

		idTripulante++;

		log_info(logDiscordiador,"Se creo el tripulante numero %d\n",idTripulante);

		tripulante->idTripulante = idTripulante;

		tripulante->idPatota = patota->ID;

		tripulante->estado = NEW;

		//t_log* bitacora = iniciarLogger("/home/utnso/tp-2021-1c-holy-C/Discordiador", "Discordiador", 1);

		//aca no hay que empezar en 1 por lo menos?
		sem_init(&tripulante->semaforo, 0, 0);

		//no hace falta el mutex aca pero bueno sha fue
		lock(mutexListaNew);

		list_add(listaDeNew,(void*)tripulante);

		unlock(mutexListaNew);


		//esto va en detach
		pthread_t t;

		pthread_create(&t, NULL, (void*) hiloTripulante, (void*) tripulante);

		pthread_join(t, (void**) NULL);

	}

	eliminarPatota(patota);
	close(server_socket);

}


void atenderMiRAM(int socketMiRAM,t_tripulante* tripulante) {

    	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

    	t_tripulante* tripulanteParaCheckear;

    	if(paqueteRecibido->codigo_operacion == TAREA){

    		t_tarea* tarea = deserializarTarea(paqueteRecibido->buffer->stream);

    		log_info(logDiscordiador,"Soy el tripulante %d y recibi la tarea de: %s \n",tripulante->idTripulante,tarea->nombreTarea);

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

    			log_info(logDiscordiador,"Estas queriendo meter a Ready un NULL negro\n");

    			exit(1);

    		}

    	}


    	eliminarPaquete(paqueteRecibido);
}


void hiloTripulante(t_tripulante* tripulante) {

	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	// Duda porque no se si hay q iniciarla de vuelta ya q la anterior no se cerró

	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);

	enviarPaquete(paqueteEnviado, miRAMsocket);

	atenderMiRAM(miRAMsocket,tripulante);

	close(miRAMsocket);

	//dameTareaMiRam()
	//atenderMiRam
}
