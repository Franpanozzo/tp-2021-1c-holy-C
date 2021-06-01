#include "discordiador.h"

char** todasLasTareasIO;

int main() {

	todasLasTareasIO = malloc(sizeof(char*) * 6);

	todasLasTareasIO[0] = strdup("GENERAR_OXIGENO");
	todasLasTareasIO[1] = strdup("CONSUMIR_OXIGENO");
	todasLasTareasIO[2] = strdup("GENERAR_BASURA");
	todasLasTareasIO[3] = strdup("DESCARTAR_BASURA");
	todasLasTareasIO[4] = strdup("GENERAR_COMIDA");
	todasLasTareasIO[5] = strdup("CONSUMIR_COMIDA");

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
	listaExec = list_create();
	colaES = queue_create();

	pthread_mutex_init(&mutexListaNew, NULL);
	pthread_mutex_init(&mutexColaReady, NULL);
	pthread_mutex_init(&mutexListaExec, NULL);

	//leerConsola();

	t_coordenadas coordenadas[4];
	for(int i=0; i<4;i++){
		coordenadas[i].posX = i;
		coordenadas[i].posY = i + 1;
	}

	planificador("INICIAR_PLANIFICACION",coordenadas);


	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}








//------ PLANIFICADOR SANTI INICIO -----
/*
 * Cuando se inicia un tripu se lo agrega a una lista de tripus
 * Va haber siempre un hilo (hilitoSabo)que se va a fijar si mongo
 * le mando un sabotaje. Si le mando lo que hace es:
 * fijarse cual es el que mas cerca esta comparandolo con los demas,
 * osea agarro dos, los comparo, el q esta mas cerca me lo quedo para
 * compararlo con el q sigue y al otro lo bloqueo por sabotaje (BLOCKED_SABO).
 * Al que me queda (el mas cerca), le guarda su "imagen" (osea los ciclos a
 * ejecutar, ciclos a bloquear, su quantum restante y su estado) y le
 * cambio su tarea a la tarea que me mando el mongo y lo pongo en EXEC
 * para que la realice. Cuando finaliza le tiene que avisar al hilitoSabo
 * que ya termino (se sincroniza con un semaforo) y ahi el hilitoSabo los
 * vuelve a todos a su estado inicial (mediante un semaforo q esta en el switch
 * de BLOCKED_SABO).
 *
 * if(noHaySabotaje + tripu.arreglandoElSabotaje)
 * noHaySabotaje es una variable global que la maneja el hilitoSabo. Se inicializa
 * en 1 cosa de que todos los tripus puedan pasar el if mientras no haya un sabotaje.
 * tripu.arreglandoElSabotaje es una variable q tiene cada tripu. La idea es que
 * despues de recibir el sabotaje lo que haga el hilitoSabo es revisar uno por uno los
 * tripus y elegir a uno para que arregle el sabotaje. A ese tripu se le pone la
 * variable arreglandoElSabotaje en 1, cosa de que entre al if para poder arreglarlo.
 * Dentro del switch grande hay un case q seria el SABOTAJE: donde el tripu hace la
 * tarea, pero a diferencia del EXEC q se pide la tarea siguiente y te fijas si es
 * I/O, cuando termina lo q hace es cambiar la varibe noHaySabotaje y la pone en 0 para
 * q los otros tripus se liberen y puedan seguir con sus tareas. Tambien antes de
 * liberar a los otros tripus, quien resolvio el sabotaje tiene q recuperar su "imagen",
 * es decir, su estado, tarea, ciclos pendientes y otros datos q tenia antes de q
 * aparezca el sabotaje
 *
 * el semaforoEjecutor(un contador q se inicializa segun el grado de
 * multiprocesamiento) simula la cola de READY, por eso en READY se hace un
 * wait y el post se hace en el EXEC.
 *
 * el semaforoBlocked (un mutex) simula la cola de blocked, en la cual
 * solo se restan los ciclos de un tripu a la vez
 *
 *
 * Nosotros tenemos en el hiloPlani una lista para cada esta estado (es global)
 * entonces una vez q todos los tripus terminen su ciclo, lo q se hace es
 * revisar la lista de los exec, aquellos q ahora su estado no es EXEC
 * los paso a sus respectivas lista (se ponen al final de la lista), lo
 * libero para q haga sus cosas (un post al semaforo q lo paraba). Lo
 * mismo hago con BLOCK, NEW, READY (tambien tengo q tener en cuenta
 * el grado de multiprocesamiento), END(en ese orden).
 */
/*
 * DUDAS:
 * 1. Si a un tripu le queda un ciclo de quantum y hay un sabotaje. Cuando
 * vuelve se reinicia el quantum?
 * 2. Si hay un sabotaje, cuando los tripus vuelven, hay que volverlos a
 * su estado que estaban antes del sabotaje o a ready?
 * 3. Puede llegar un sabotaje mientras la plani esta pausada? Si se puede
 * q debo hacer?
 * 4. Cuando hay un sabotaje, hay q ir hasta la posicion donde esta? En ese
 * caso si el tripu que lo resolvio estaba en exec, tiene q volver al lugar
 * donde estaba haciendo su tarea y recalcular su tiempo de exec? Con los
 * bloqueos pasa lo mismo?
 */

// TODO eliminar el semaforo del tripu

void hiloPlani(){
	while(1){
		if(planificacion_play){

			for(int i=0; i<totalTripus; i++){
				sem_wait(&semaforoPlanificadorInicio);
			}

				actualizar(EXEC, colaExec);
				actualizar(BLOCKED, colaBlocked);
				actualizar(NEW, colaNew);
				actualizar(READY, colaReady);

			if(noHaySabotaje == 0){ // HAY SABOTAJE
				tripulanteDesabotaje = elTripuMasCerca(sabotaje.coordenadas);
				// el sabotaje es una variable global de tipo t_tarea y tripulanteDesabotaje tambien es global
				t_estado estadoAnterior = imagenTripu->estado;
				t_estado tareaAnterior = imagenTripu->instruccionAejecutar;
				tripulanteDesabotaje->estado = SABOTAJE;
				tripulanteDesabotaje->instruccionAejecutar = sabotaje;
				sem_wait(&semaforoSabo);
				tripulanteDesabotaje->estado = estadoAnterior;
				tripulanteDesabotaje->instruccionAejecutar = tareaAnterior;
//				tripulanteDesabotaje tiene q ponerse como en un null, un tripu null
				noHaySabotaje = 1;
				sem_post(semaforoImagen); // se inicializa en 0
				sem_post(&semaforoSabotajeResuelto); //se usa para indicar al hiloSabotaje
				// q ya se termino. Mas q nada por si nos mandan dos sabos juntos
			}


			for(int i=0; i<totalTripus; i++){
				sem_post(&semaforoPlanificadorFin);
			}
		}
	}
}



void hilitoSabo(){

	while(1){
		/*
		 * TODO la parte de recibir un saboteje, osea un recv y se
		 * deserializa para ver si llego un sabotaje en el caso de
		 * que si, se hace lo siguiente:
		 */

		noHaySabotaje = 0;
		sem_wait(&semaforoSabotajeResuelto);
	}
}


void hilitoTripu(t_tripulante* tripu){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	int ciclosSabo = 0;
	int quantumPendiente = quantum;
	/*
	 *	si es FIFO el quantum sera -1, si es RR sera lo q dice el config,
	 *  esta opcion la posibilidad de cambiar el modo de ejecucion y
	 *  dejar q unos hagan FIFO y otros RR, pero si el qpendiente se
	 *  lo asigna al final de inicarTripu, se puede.
	 *
	 */
	int tripuVivo = 1;
	while(tripuVivo){
		if(tripu->idTripulante != tripuDesaboteador->idTripulante)
			sem_wait(semaforoPlanificadorFin);
		if(noHaySabotaje || tripu->idTripulante == tripuDesaboteador->idTripulante){
			switch(tripu->estado){
				case NEW:
					//TODO pedir la tarea y hacer lo mas q se pueda aca
					tripu->estado = READY;
					break;
				case READY:
					if(ciclosExec == 0) // este if se pone porq si me desalojan por quantum no tengo que volver a calcular mis ciclos
						ciclosExec = calcularCiclosExec(tripu->instruccionAejecutar);
					quantumPendiente = quantum;
					tripu->estado = EXEC;
					break;
				case EXEC:
					sleep(1);//correr un ciclo;
					ciclosExec --;
					quantumPendiente--;
					if(ciclosExec > 0 && quantumPendiente != 0){
						desplazarse(tripu); //mueve al tripu hasta el lugar y si esta en el lugar no se mueve
					}
					else{
						if(quantumPendiente == 0){
							tripu->estado = READY;
						}
						else{
							if(esIO(tripu->instruccionAejecutar)){
								pasarleAMongoTarea(tripu->instruccionAejecutar);
								ciclosBlocked = tripu->instruccionAejecutar->tiempo;
								tripu->estado = BLOCKED;
							}
							tripu->instruccionAejecutar = pedirTareaRAM(tripu);
							if(tripu->instruccionAejecutar == "TAREA_NULA"){
								tripu->estado = END;
							}
							ciclosExec = calcularCiclosExec(tripu->instruccionAejecutar);
						}
					}
					break;
				case BLOCKED:
					if(tripu->idTripulante == idTripulanteBlocked){
						sleep(1);//correr un ciclo;
						ciclosBlocked --;
						if(ciclosBlocked == 0){
							idTripulanteBlocked = -1;
							tripu->estado = READY;
						}
					}
					break;
				case SABOTAJE:
					sleep(1);
					ciclosSabo --;
					if(ciclosSabo > 0){
						desplazarse(tripu);
					}
					else{
					sem_post(semaforoSabo); //este semaforo le permite recuperar su "imagen"
					//el wait se hace dentro del hiloSabotaje antes de q devolverle la imagen
					//y el semaforo se inicializa en 0
					sem_wait(semaforoImagen); //lo hago para asegurarme q le devuelva la imagen
					}
					break;
				case END:
					totalTripus--;
					tripuVivo = 0;
					break;
			}
			actualizarEstado(tripu); //esto tambien se usa para indicar q el tripu termino
			if(tripu->idTripulante != tripuDesaboteador->idTripulante)
				sem_post(semaforoPlanificadorInicio);
		}
	}
}


void actualizar(t_estado estado, t_queue* cola){
	t_queue* aux = queue_create();
	t_tripulante* tripulante;
	while(cola != NULL && queue_size(colaExec)=<gradoMultiprocesamiento){
		tripulante = (t_tripulante*) queue_pop(cola);
		if(tripulante->estado != estado){
			pasarDeEstado(tripulante);
		}
		else{
			queue_push(aux, &tripulante);
		}
	}
	cola = aux;
	aux = NULL;
	free(aux); //esto no se si lo tengo q hacer, capaz no es necesario liberar a aux
}

t_tripulante* elTripuMasCerca(t_coordenadas lugarSabotaje){
	/*
	 * TODO devuelve el tripu mas cerca a una cierta posicion.
	 * Hay q recorrer todas las colas e ir comparando las posiciones
	 * de los tripus hasta q quede un solo tripu. De las colas no
	 * hay q sacar nada, solo estamos leyendo. Es decir q las
	 * colas deben quedar como estaban.
	 */
}
//------ PLANIFICADOR SANTI FIN ------







// ---- NO LA USAMOS ----
void planificador(char* string,t_coordenadas* coordenadas){

	char* algoritmo;
	int grado_multiprocesamiento;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	grado_multiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	log_info(logDiscordiador,"Empezando planificacion usando el algoritmo %s",algoritmo);

	if(strcmp(string,"INICIAR_PLANIFICACION") == 0){
		iniciarPatota(coordenadas, "GENERAR_OXIGENO 4;5;6;7\nCONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2", 4);

	}

	while(list_size(listaExec) <= grado_multiprocesamiento && queue_is_empty(colaDeReady) != 1){

		t_tripulante* tripulante = queue_peek(colaDeReady);

		log_info(logDiscordiador,"Moviendo tripulante %d a EXEC", tripulante->idTripulante);

		queue_pop(colaDeReady);

		pasarDeEstado(tripulante, EXEC);
	}

	t_list_iterator* iterador = list_iterator_create(listaExec);

	while(list_iterator_has_next(iterador)){

		t_tripulante* tripulanteAsacar = (t_tripulante*) list_iterator_next(iterador);

		if(strcmp(algoritmo,"FIFO") == 0){
			planificacionFIFO(tripulanteAsacar);
		}
		else if(strcmp(algoritmo,"RR") == 0){
			planificacionRR(tripulanteAsacar);
		}
		else{
			log_info(logDiscordiador,"No puedo planificar en ese algoritmo negro\n");
			exit(1);
		}
	}
}

int calculoCiclosARealizar(t_tripulante* tripulante){

	int movimientosEnX = fabs(tripulante->instruccionAejecutar->posX - tripulante->posX);
	int movimientosEnY = fabs(tripulante->instruccionAejecutar->posY - tripulante->posY);

	return movimientosEnX + movimientosEnY;
}

void tareasIO(t_tripulante* tripulante){

	int ciclos = calculoCiclosARealizar(tripulante) + 1;

	while(ciclos > 0){

		//int socketMiRAM = iniciarConexionDesdeCliente(puertoEIPRAM);

		if(tripulante->posX > tripulante->instruccionAejecutar->posX){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posX--;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

		}

		if(tripulante->posX < tripulante->instruccionAejecutar->posX){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posX++;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}

		if(tripulante->posY > tripulante->instruccionAejecutar->posY){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posY--;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}

		if(tripulante->posY < tripulante->instruccionAejecutar->posY){
			log_info(logDiscordiador,"Moviendose de la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			tripulante->posY++;
			log_info(logDiscordiador,"A la posicion en X|Y ==> %d|%d  ",tripulante->posX, tripulante->posY );
			ciclos--;
			sleep(1);
			//t_paquete* paquete = armarPaqueteCon()

			}
		if(tripulante->posX == tripulante->instruccionAejecutar->posX && tripulante->posY == tripulante->instruccionAejecutar->posY){
			ciclos--;
			sleep(1);
			int socketMongo = iniciarConexionDesdeClienteHacia(puertoEIPMongo);
			t_paquete* tareaArealizar = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);
			enviarPaquete(tareaArealizar,socketMongo);
			t_paquete* prueba = recibirPaquete(socketMongo);
			char* respuesta = deserializarString(prueba);
			log_info(logDiscordiador,"Recibi %s \n", respuesta);
			//bool esIgualA(t_tripulante* tripulanteAcomparar){
				//return tripulanteAcomparar == tripulante;
			//}
			//list_remove_by_condition(listaExec,(void*) esIgualA);
			t_bloqueado* tripulanteBloqueado = malloc(sizeof(t_bloqueado));
			tripulanteBloqueado->socket = socketMongo;
			tripulanteBloqueado->tripulante = tripulante;
			queue_push(colaES, tripulanteBloqueado);

		}

	}


}


void tareasNoIO(t_tripulante* tripulante){

	//int cantidadAciclar = calculoCiclosARealizar(tripulante) + tripulante->instruccionAejecutar->tiempo;
}

// ----- NO LA USAMOS ----
void planificacionFIFO(t_tripulante* tripulante){

	switch(esIO(tripulante->instruccionAejecutar->nombreTarea)){

			case 1:
			{
					log_info(logDiscordiador,"Recibi una accion IO del tripulante %d",tripulante->idTripulante);
					tareasIO(tripulante);
					break;
			}
			case 0:
			{		log_info(logDiscordiador,"Recibi una accion que no es IO del tripulante %d",tripulante->idTripulante);
					tareasNoIO(tripulante);
					break;
			}

			default:
			{
					log_info(logDiscordiador,"Recibi cualquier cosa negro\n");
					exit(1);
			}
			}
}


/*-------------------------------------------------------------------------------
//LO LLAMA SANTI DESDE LA CONSOLA
void cpuPlanificacion() {
	char* algoritmo;
	int grado_multiprocesamiento;
	algoritmo = config_get_string_value(config,"ALGORITMO");
	grado_multiprocesamiento = config_get_int_value(config,"GRADO_MULTITAREA");
	log_info(logDiscordiador,"Empezando planificacion usando el algoritmo %s",algoritmo);

	planificacion_pausada= 1;
	//Iniciar_planificaion -> pausado=1;
	//Pausar_planificacion -> pausado=0;

	while(1){

	while(planificacion_pausada);

	if(planificacion_pausada){
		sem_wait(&semPlanificacion);//lo hace consola
	}

	if(list_size(listaDeNew)>0){//independiente del algoritmo de planificacion
		lock(mutexListaNew);
		t_tripulante* tripulanteARedy =  list_remove(listaDeNew,0);
		unlock(mutexListaNew);
		pasarDeEstado(tripulanteARedy,READY);

	}

	if(list_size(listaExec) < grado_multiprocesamiento){//Cuando la planificacion esta activa

		if(strcmp(algoritmo,"FIFO") == 0) {
			lock(mutexColaReady);
			t_tripulante* tripulanteAExec =  queue_pop(colaDeReady);
			unlock(mutexColaReady);
			pasarDeEstado(tripulanteAExec,EXEC);

		}

		if(strcmp(algoritmo,"RR") == 0) {
			//Todo
		}

		else {
			log_info(logDiscordiador,"No existe ese algoritmo de planificacion negro");
		}
	}
 }

}

------------------------------------------------------------------------------------------------
*/


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
	patota->ID = idPatota;
	patota->tareas = tareasString;

	log_info(logDiscordiador,"Se creo la patota numero %d\n",idPatota);
	return patota;
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	int server_socket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);

	t_patota* patota = asignarDatosAPatota(tareasString);
	t_paquete* paquete = armarPaqueteCon((void*) patota,PATOTA);
	enviarPaquete(paquete,server_socket);

	for (int i=0; i<cantidadTripulantes; i++){
		iniciarTripulante(coordenadas[i], patota->ID);
	}

	//eliminarPatota(patota);
	close(server_socket);
}


void iniciarTripulante(t_coordenadas coordenada, uint32_t idPatota){

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));

	pthread_t _hiloTripulante;

	idTripulante++;

	tripulante->posX = coordenada.posX;
	tripulante->posY = coordenada.posY;
	tripulante->idTripulante = idTripulante;
	tripulante->idPatota = idPatota;
	tripulante->estado = NEW;

	sem_init(&tripulante->semaforo, 0, 0);
//	el tripulante no necesita un semaforo

	lock(mutexListaNew);
	list_add(listaDeNew,(void*)tripulante);
	unlock(mutexListaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d\n",tripulante->idTripulante);



	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, (void*) tripulante);
	pthread_join(_hiloTripulante, (void**) NULL);
//	arriba no deberia ir un detach? no quiero q el programa se bloquee ahi
}


int esIO(char* tarea){

	int i = 0;

	while(todasLasTareasIO[i] != NULL){

		if(strcmp(todasLasTareasIO[i],tarea) == 0){
			return 1;
		}


		i++;
	}

	return 0;

}


void pasarDeEstado(t_tripulante* tripulante){


	switch(tripulante->estado){
		case READY:
			queue_push(colaDeReady, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			break;

		case EXEC:

			queue_push(colaDeReady, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			break;

		case BLOCKED:
			if(idTripulanteBlocked == -1)
				idTripulanteBlocked = tripulante->idTripulante;
			queue_push(colaDeReady, &tripulante);
			log_info(logDiscordiador,"El tripulante %d paso a READY \n", tripulante->idTripulante);
			break;
		case END:
			break;

		default:
				printf("\n No se reconoce el siguiente estado \n");
				exit(1);
	}

}


void recibirTareaDeMiRAM(int socketMiRAM, t_tripulante* tripulante){

	t_paquete* paqueteRecibido = recibirPaquete(socketMiRAM);

	if(paqueteRecibido->codigo_operacion == TAREA){

	    tripulante->instruccionAejecutar = deserializarTarea(paqueteRecibido->buffer->stream);


	}
	else{
	    log_error(logDiscordiador,"El paquete recibido no es una tarea\n");
	    exit(1);
	}

	eliminarPaquete(paqueteRecibido);
}


void recibirPrimerTareaDeMiRAM(t_tripulante* tripulante){

	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete* paqueteEnviado = armarPaqueteCon((void*) tripulante,TRIPULANTE);
	enviarPaquete(paqueteEnviado, miRAMsocket);

	recibirTareaDeMiRAM(miRAMsocket, tripulante);

	close(miRAMsocket);
}


void recibirProximaTareaDeMiRAM(t_tripulante* tripulante){
	int miRAMsocket = iniciarConexionDesdeClienteHacia(puertoEIPRAM);
	t_paquete * paquete = armarPaqueteCon((void*) tripulante,SIGUIENTETAREA);
	enviarPaquete(paquete, miRAMsocket);

	recibirTareaDeMiRAM(miRAMsocket,tripulante);
	close(miRAMsocket);
}



void mandarTareaAejecutar(t_tripulante* tripulante, int socketMongo){

	t_paquete* paqueteConLaTarea = armarPaqueteCon((void*) tripulante->instruccionAejecutar,TAREA);

	enviarPaquete(paqueteConLaTarea,socketMongo);

//	recibirConfirmacionDeMongo(socketMongo,tripulante->instruccionAejecutar);
//	no es necesaria la confirmacion
}


char* deserializarString (t_paquete* paquete){

	char* string = malloc(paquete->buffer->size);

	memcpy(string,(paquete->buffer->stream),paquete->buffer->size);

	return string;
}


void recibirConfirmacionDeMongo(int socketMongo, t_tarea* tarea){

	t_paquete* paqueteRecibido = recibirPaquete(socketMongo);

	char* mensajeRecibido = deserializarString(paqueteRecibido);
	log_info(logDiscordiador,"Confirmacion %s\n",mensajeRecibido);

	if(strcmp(mensajeRecibido,"TAREA REALIZADA") == 0){

		log_info(logDiscordiador,"Se elimino la tarea %s\n",tarea->nombreTarea);

		free(tarea->nombreTarea);
		free(tarea);
		free(mensajeRecibido);
	}

	else{

		log_info(logDiscordiador, "No sabemos q hacer todavia =(\n");

		free(tarea->nombreTarea);
		free(tarea);
		free(mensajeRecibido);

	}

	eliminarPaquete(paqueteRecibido);

}


