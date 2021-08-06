#include "discordiador.h"

int tripulantes;
int main() {
	char * path = pathLog();
	logDiscordiador = iniciarLogger(path,"Discordiador",1);
	crearConfig(); // Crear config para puerto e IP de Mongo y Ram


	iniciarMutex();
	iniciarListas();
	iniciarTareasIO();

	sabotaje = malloc(sizeof(t_sabotaje));
//	sabotaje->tripulanteSabotaje = malloc(sizeof(t_tripulante));
	sabotaje->tripulanteSabotaje = NULL;
	sabotaje->haySabotaje = 0;

	cargarConfiguracion();

	idTripulante = 0;
	idPatota = 0;
	modificarTripulanteBlocked(NO_HAY_TRIPULANTE_BLOQUEADO);
	modificarPlanificacion(PAUSADA);

	sem_init(&sabotaje->semaforoIniciarSabotaje,0,0);
	sem_init(&sabotaje->semaforoCorrerSabotaje,0,0);
	sem_init(&sabotaje->semaforoTerminoTripulante,0,0);
	sem_init(&sabotaje->semaforoTerminoSabotaje,0,0);
	sem_init(&semPlanificacion,0,0);
	sem_init(&semHayTripulantes, 0, 0);

	pthread_create(&planificador, NULL, (void*) hiloPlanificador, NULL);
	pthread_detach(planificador);

	int serverSock = iniciarConexionDesdeServidor(puertoDisc);

	pthread_create(&manejoSabotaje, NULL, (void*) atenderSabotaje, (void*) &serverSock);
	pthread_detach(manejoSabotaje);

	leerConsola();
	free(path);
	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}


void hiloPlanificador(){

	int contadorCiclos = 0;

	while(1){

		sem_wait(&semPlanificacion);
		sem_post(&semPlanificacion);

		if(totalTripulantes() > 0){

			comunicarseConTripulantes(listaExec, (void*)esperarTerminarTripulante);
			comunicarseConTripulantes(listaBlocked, (void*)esperarTerminarTripulante);
			comunicarseConTripulantes(listaNew, (void*)esperarTerminarTripulante);

			log_info(logDiscordiador, "####----CONTADOR CICLOS----####: %d", contadorCiclos);

			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripulantes());
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");

			if(!list_is_empty(listaExec->elementos))
				actualizarListaExec();
			if(!list_is_empty(listaBlocked->elementos))
				actualizarListaBlocked();
			if(!list_is_empty(listaNew->elementos))
				actualizarListaNew();
			if(!list_is_empty(listaReady->elementos)){
				list_iterate(listaReady->elementos, (void*)esperarTerminarTripulante);
				actualizarListaReady();
			}

			if(sabotaje->haySabotaje && sabotaje->tripulanteSabotaje == NULL){ //HAY SABOTAJE
				sem_post(&sabotaje->semaforoIniciarSabotaje);
				sem_wait(&sabotaje->semaforoCorrerSabotaje);//se traba esperando a q se bloqueen a los tripus por el sabotaje
			}

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

			comunicarseConTripulantes(listaExec, (void*)avisarTerminoPlanificacion);
			comunicarseConTripulantes(listaBlocked, (void*)avisarTerminoPlanificacion);
			comunicarseConTripulantes(listaNew, (void*)avisarTerminoPlanificacion);
			comunicarseConTripulantes(listaReady, (void*)avisarTerminoPlanificacion);
			comunicarseConTripulantes(listaExit, (void*)avisarTerminoPlanificacion);

			contadorCiclos++;
		}
		else{
			sem_wait(&semHayTripulantes);
		}
	}
}


void atenderSabotaje(int* serverSock) {

    while(1){

    	int* sabotajeSock = malloc(sizeof(int));
		*sabotajeSock = esperarSabotaje(*serverSock);

		procedimientoSabotaje(sabotajeSock);
    }
}


int esperarSabotaje(int serverSock) {

    struct sockaddr_in serverAddress;

    unsigned int len = sizeof(struct sockaddr);

    int socket_tripulante = accept(serverSock, (void*) &serverAddress, &len);

	log_info(logDiscordiador,"----- HAY UN SABOTAJE -----");

    return socket_tripulante;
}



void procedimientoSabotaje(int* sabotajeSock){

	t_paquete* paquete = recibirPaquete(*sabotajeSock);

	sabotaje->coordenadas = deserializarCoordenadas(paquete->buffer->stream);

	log_info(logDiscordiador,"EL SABOTAJE ES EN: X:%d - Y:%d", sabotaje->coordenadas.posX, sabotaje->coordenadas.posY );

	//PREPARACION SABOTAJE
	sabotaje->haySabotaje = 1;
	sem_wait(&sabotaje->semaforoIniciarSabotaje);//esperar a que el plani termine de actualizar las listas
	//log_info(logDiscordiador,"----- EL PLANIFICADOR ME DEJO AVANZAR -----");

	pasarAlistaSabotaje(listaExec);
	pasarAlistaSabotaje(listaReady);

	log_info(logDiscordiador,"%d tripulantes pasaron a la lista de sabotaje",
					list_size(listaSabotaje->elementos));

	elegirTripulanteSabotaje();

	//FINALIZACION SABOTAJE
	sem_wait(&sabotaje->semaforoTerminoTripulante);//esperar q el tripulante termine el sabotaje

	list_iterate(listaSabotaje->elementos, (void*)ponerEnReady);
	list_add_all(listaReady->elementos, listaSabotaje->elementos);
	sabotaje->tripulanteSabotaje->estado = READY;
	list_clean(listaSabotaje->elementos);
	sabotaje->haySabotaje = 0;
	list_add(listaReady->elementos, sabotaje->tripulanteSabotaje);
	sabotaje->tripulanteSabotaje = NULL;

	sem_post(&sabotaje->semaforoCorrerSabotaje);//avisarle al planificador que termino la preparacion

	close(*sabotajeSock);
	free(sabotajeSock);
}


void hiloTripulante(t_tripulante* tripulante){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	int quantumPendiente = quantum;
	t_avisoTarea* avisoTarea = NULL;

	while(leerEstado(tripulante) != EXIT){
		switch(leerEstado(tripulante)){

			case NEW:
				log_info(logDiscordiador,"el tripulante %d esta en new", tripulante->idTripulante);
				sem_post(&tripulante->semaforoFin);
				sem_wait(&tripulante->semaforoInicio);
				break;

			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
        
				log_info(logDiscordiador,"el tripulante %d esta en ready con %d ciclos",
						tripulante->idTripulante, ciclosExec +
						distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas));

				sem_post(&tripulante->semaforoFin);
				sem_wait(&tripulante->semaforoInicio);
				break;

			case EXEC:

				if(quantumPendiente < 0){
					log_info(logDiscordiador,"el tripulante %d esta en exec con %d ciclos",
							tripulante->idTripulante, ciclosExec +
							distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas));
				}
				else{
					log_info(logDiscordiador,"el tripulante %d esta en exec con %d ciclos y %d quantum",
							tripulante->idTripulante, ciclosExec +
							distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas),
							quantumPendiente);
				}

				usleep(retardoCiclosCPU * 1000);
				quantumPendiente--;

				if(distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas) > 0){
					desplazarse(tripulante, tripulante->instruccionAejecutar->coordenadas);
				}
				else{
					if(avisoTarea == NULL){
						avisoTarea = malloc(sizeof(t_avisoTarea));
						avisoTarea->idTripulante = tripulante->idTripulante;
						avisoTarea->nombreTarea = tripulante->instruccionAejecutar->nombreTarea;
						log_info(logDiscordiador,"El tripulante %d esta enviando el INICIO de la tarea: %s",tripulante->idTripulante, avisoTarea->nombreTarea);
						int socketMongo = enviarA(puertoEIPMongo, avisoTarea, INICIO_TAREA);
						close(socketMongo);
					}
					ciclosExec --;
				}
				if(quantumPendiente == 0 && leerEstado(tripulante) != EXIT){
					modificarEstado(tripulante, READY);
				}
				if(ciclosExec <= 0){

					int socketMongo = enviarA(puertoEIPMongo, avisoTarea, FIN_TAREA);
					log_info(logDiscordiador,"El tripulante %d esta enviando el FIN de la tarea: %s",tripulante->idTripulante, avisoTarea->nombreTarea);
					close(socketMongo);
					free(avisoTarea);
					avisoTarea = NULL;

					if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
						log_info(logDiscordiador,"El tripulante %d esta enviando la TAREA: %s",tripulante->idTripulante, tripulante->instruccionAejecutar->nombreTarea);
						int socketMongo = enviarA(puertoEIPMongo, tripulante->instruccionAejecutar, TAREA);
						close(socketMongo);
						ciclosBlocked = tripulante->instruccionAejecutar->tiempo;
						if(leerEstado(tripulante) != EXIT){
							modificarEstado(tripulante, BLOCKED);
						}
					}
					else{
						recibirProximaTareaDeMiRAM(tripulante);
						ciclosExec = calculoCiclosExec(tripulante);
					}
				}
				sem_post(&tripulante->semaforoFin);
				sem_wait(&tripulante->semaforoInicio);
				break;

			case BLOCKED:
				if(tripulante->idTripulante == leerTripulanteBlocked()){
					usleep(retardoCiclosCPU * 1000);
					log_info(logDiscordiador,"el tripulante %d esta en block con %d ciclos",
							tripulante->idTripulante, ciclosBlocked);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						modificarTripulanteBlocked(NO_HAY_TRIPULANTE_BLOQUEADO);
						if(leerEstado(tripulante) != EXIT){
							modificarEstado(tripulante, READY);
						}
						recibirProximaTareaDeMiRAM(tripulante);
						ciclosExec = calculoCiclosExec(tripulante);
					}
				}
				sem_post(&tripulante->semaforoFin);
				sem_wait(&tripulante->semaforoInicio);
				break;

			case SABOTAJE:
				sem_wait(&semPlanificacion);
				sem_post(&semPlanificacion);
				ciclosExec = 0;
				if(distancia(tripulante->coordenadas, sabotaje->coordenadas) > 0){
					desplazarse(tripulante, sabotaje->coordenadas);
					sleep(retardoCiclosCPU);

					log_info(logDiscordiador,"el tripulante %d esta yendo al sabo, esta a %d de distancia",
							tripulante->idTripulante, distancia(tripulante->coordenadas, sabotaje->coordenadas));
				}
				else{
					log_info(logDiscordiador,"el tripulante %d llego a la posicion del sabotaje %d|%d",
							tripulante->idTripulante, tripulante->coordenadas.posX, tripulante->coordenadas.posX);

					int socketMongo = enviarA(puertoEIPMongo, &tripulante->idTripulante, RESOLUCION_SABOTAJE);
					close(socketMongo);
					sleep(sabotaje->tiempo);

					sem_post(&sabotaje->semaforoTerminoTripulante);
//					sem_wait(&sabotaje->semaforoTerminoSabotaje);
					sem_wait(&tripulante->semaforoInicio);
					log_info(logDiscordiador,"el tripulante %d resolvio el sabotjae",
							tripulante->idTripulante);
				}
				break;

			case EXIT:
				//log_error(logDiscordiador,"el tripulante %d no deberia estar aca", tripulante->idTripulante);
				break;
		}
	}

	lock(&mutexEliminarPatota);
	if(patotaSinTripulantes(tripulante->idPatota, tripulante->idTripulante)){
		log_info(logDiscordiador,"Llego el tripulante %d y ya no quedan tripulantes de la patota %d",
				tripulante->idTripulante, tripulante->idPatota);
		eliminiarPatota(tripulante->idPatota);
	}
	unlock(&mutexEliminarPatota);
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	void empezarHiloTripulante(t_tripulante* tripulante){

		pthread_t _hiloTripulante;

		log_info(logDiscordiador,"Se creo el tripulante numero %d con posicion %d|%d",
					tripulante->idTripulante, tripulante->coordenadas.posX, tripulante->coordenadas.posY);

		pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, tripulante);
		pthread_detach(_hiloTripulante);
	}

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	t_list* listaTripulantes = list_create();

	if(confirmacion(miRAMsocket)){

		log_info(logDiscordiador,"Se creo la patota %d con %d tripulantes",
				patota->ID, cantidadTripulantes);
    
		for (int i=0; i<cantidadTripulantes; i++){
			iniciarTripulante(*(coordenadas+i), patota->ID, listaTripulantes);
		}

		if(totalTripulantes() == list_size(listaTripulantes)){
			sem_post(&semHayTripulantes);
		}

		list_iterate(listaTripulantes, (void*) empezarHiloTripulante);
	}
	else{
		log_info(logDiscordiador,"No hay espacio suficiente en memoria para iniciar la patota %d",patota->ID);
	}


	list_destroy(listaTripulantes);
	eliminarPatota(patota);
	close(miRAMsocket);
}


void iniciarTripulante(t_coordenadas coordenada, uint32_t idPatota, t_list* listaTripulantes){

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));

	idTripulante++;

	tripulante->coordenadas.posX = coordenada.posX;
	tripulante->coordenadas.posY = coordenada.posY;
	tripulante->idTripulante = idTripulante;
	tripulante->idPatota = idPatota;
	pthread_mutex_init(&tripulante->mutexEstado, NULL);
	modificarEstado(tripulante, NEW);
	tripulante->instruccionAejecutar = malloc(sizeof(t_tarea*));
	tripulante->instruccionAejecutar->nombreTarea = malloc(sizeof(char*));
	sem_init(&tripulante->semaforoInicio, 0, 0);
	sem_init(&tripulante->semaforoFin, 0, 0);

	mandarTCBaMiRAM(tripulante);

	if(tripulante->estado != EXIT){

		if(sabotaje->haySabotaje){
			meterEnLista(tripulante, listaSabotaje);
		}
		else{
			list_add(listaNew->elementos, tripulante);
			//meterEnLista(tripulante, listaNew);
		}
		list_add(listaTripulantes, tripulante);
	}
	else{
		liberarTripulante(tripulante);
	}
}


void actualizarListaNew(){

	void actualizarYPedirTarea(t_tripulante* tripulante) {
			actualizarEstadoEnRAM(tripulante);
			recibirProximaTareaDeMiRAM(tripulante);
	}

	lock(&listaNew->mutex);

	log_info(logDiscordiador,"------Iniciando planficacion cola de new con %d tripulantes-----",
				list_size(listaNew->elementos));

	list_iterate(listaNew->elementos, (void*)ponerEnReady);
	list_iterate(listaNew->elementos, (void*)actualizarYPedirTarea);
	list_iterate(listaNew->elementos, (void*)pasarDeLista);
	list_clean(listaNew->elementos);

	log_info(logDiscordiador,"------Finalizando planficacion cola de new con %d tripulantes-----",
					list_size(listaNew->elementos));

	unlock(&listaNew->mutex);
}


void actualizarListaReady(){
	void ponerEnExec(t_tripulante* tripulante){
		cambiarDeEstado(tripulante, EXEC);
	}

	lock(&listaReady->mutex);

	log_info(logDiscordiador,"------Iniciando planficacion cola de ready con %d tripulantes-----",
				list_size(listaReady->elementos));

	int cantidadApasar = gradoMultiprocesamiento - list_size(listaExec->elementos);
	t_list* listaAux = list_take_and_remove(listaReady->elementos, cantidadApasar);
	list_iterate(listaAux, (void*)ponerEnExec);
	list_iterate(listaAux, (void*)actualizarEstadoEnRAM);
	list_iterate(listaAux, (void*)pasarDeLista);

	log_info(logDiscordiador,"------Finalizando planficacion cola de ready con %d tripulantes-----",
					list_size(listaReady->elementos));

	unlock(&listaReady->mutex);

	list_destroy(listaAux);
}


void actualizarListaExec(){
	actualizarListaEyB(listaExec, EXEC);
}


void actualizarListaBlocked(){
	actualizarListaEyB(listaBlocked, BLOCKED);
	if(leerTripulanteBlocked() == NO_HAY_TRIPULANTE_BLOQUEADO && list_size(listaBlocked->elementos) > 0)
		elegirTripulanteAbloquear();
}


void actualizarListaEyB(t_lista* lista, t_estado estado){

	bool tieneDistintoEstado(t_tripulante* tripulante){
		return estado != leerEstado(tripulante);
	}

	lock(&lista->mutex);

	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----",
				traducirEstado(estado), list_size(lista->elementos));

	while(list_any_satisfy(lista->elementos, (void*)tieneDistintoEstado)){
		t_tripulante* tripulante = (t_tripulante*)list_remove_by_condition
				(lista->elementos, (void*)tieneDistintoEstado);
		actualizarEstadoEnRAM(tripulante);
		pasarDeLista(tripulante);
	}

	log_info(logDiscordiador,"------FIN Planficando cola de %s con %d tripulantes-----",
					traducirEstado(estado), list_size(lista->elementos));

	unlock(&lista->mutex);
}
