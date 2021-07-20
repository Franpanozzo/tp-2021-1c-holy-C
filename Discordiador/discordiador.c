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
	idTripulanteBlocked = NO_HAY_TRIPULANTE_BLOQUEADO;
	modificarPlanificacion(PAUSADA);

	sem_init(&sabotaje->semaforoIniciarSabotaje,0,0);
	sem_init(&sabotaje->semaforoCorrerSabotaje,0,0);
	sem_init(&sabotaje->semaforoTerminoTripulante,0,0);
	sem_init(&sabotaje->semaforoTerminoSabotaje,0,0);
	sem_init(&semUltimoTripu,0,0);


	pthread_create(&planificador, NULL, (void*) hiloPlanificador, NULL);
	pthread_detach(planificador);

	pthread_create(&sabo, NULL, (void*) hiloSabotaje, NULL);
	pthread_detach(sabo);

	leerConsola();
	free(path);
	free(puertoEIPRAM->IP);
	free(puertoEIPRAM);
	free(puertoEIPMongo->IP);
	free(puertoEIPMongo);

	return EXIT_SUCCESS;
}


void hiloPlanificador(){
	while(1){
		if(leerPlanificacion() == CORRIENDO && totalTripulantes() > 0){

			comunicarseConTripulantes(listaExec, (void*)esperarTerminarTripulante);
			comunicarseConTripulantes(listaBlocked, (void*)esperarTerminarTripulante);
			comunicarseConTripulantes(listaNew, (void*)esperarTerminarTripulante);

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
		}
	}
}


void hiloSabotaje(){

	while(1){
		/*
		 * TODO la parte de recibir un saboteje, osea un recv y se
		 * deserializa para ver si llego un sabotaje en el caso de
		 * que si, se hace lo siguiente:
		 */
		while(sabotaje->haySabotaje == 0);
		log_info(logDiscordiador,"----- DETECTE Q HAY UN SABOTAJE -----");

		//PREPARACION SABOTAJE
		sabotaje->haySabotaje = 1;
		sem_wait(&sabotaje->semaforoIniciarSabotaje);//esperar a que el plani termine de actualizar las listas
		log_info(logDiscordiador,"----- EL PLANIFICADOR ME DEJO AVANZAR -----");

		pasarAlistaSabotaje(listaExec);
		pasarAlistaSabotaje(listaReady);

		log_info(logDiscordiador,"----- LA LISTA DE READY QUEDO CON %d TRIPULANTES -----",
				list_size(listaReady->elementos));
		log_info(logDiscordiador,"----- LA LISTA DE EXEC QUEDO CON %d TRIPULANTES -----",
						list_size(listaExec->elementos));
		log_info(logDiscordiador,"----- LA LISTA DE SABOTAJE QUEDO CON %d TRIPULANTES -----",
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
		int socketMongo = enviarA(puertoEIPMongo, &sabotaje->tripulanteSabotaje->idTripulante, FIN_SABOTAJE);
		close(socketMongo);
		sabotaje->tripulanteSabotaje = NULL;

		log_info(logDiscordiador,"----- LA LISTA DE READY FINAL SABOTAJE QUEDO CON %d TRIPULANTES -----",
						list_size(listaReady->elementos));


		sem_post(&sabotaje->semaforoCorrerSabotaje);//avisarle al planificador que termino la preparacion

//		sem_post(&sabotaje->semaforoTerminoSabotaje);//avisarle al tripulante que ya volvio _todo a la normalidad
	}
}


void hiloTripulante(t_tripulante* tripulante){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	int quantumPendiente = quantum;
	t_avisoTarea* avisoTarea = NULL;

	while(tripulante->estado != EXIT){
		switch(tripulante->estado){

			case NEW:
				log_info(logDiscordiador,"tripulanteId %d: estoy en new", tripulante->idTripulante);
				recibirPrimerTareaDeMiRAM(tripulante);
				//sem_post(&semUltimoTripu);
				sem_post(&tripulante->semaforoFin);
				break;

			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
        
				log_info(logDiscordiador,"el tripulante %d esta en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec +
						distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas));

				sem_post(&tripulante->semaforoFin);
				break;

			case EXEC:
				log_info(logDiscordiador,"el tripulante %d esta en exec con %d ciclos y %d quantum",
						tripulante->idTripulante, ciclosExec +
						distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas),
						quantumPendiente);

				sleep(retardoCiclosCPU);
				quantumPendiente--;

				if(distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas) > 0){
					desplazarse(tripulante, tripulante->instruccionAejecutar->coordenadas);
				}
				else{
					if(avisoTarea == NULL){
						avisoTarea = malloc(sizeof(t_avisoTarea));
						avisoTarea->idTripulante = tripulante->idTripulante;
						avisoTarea->nombreTarea = tripulante->instruccionAejecutar->nombreTarea;
						int socketMongo = enviarA(puertoEIPMongo, avisoTarea, INICIO_TAREA);
						close(socketMongo);
					}
					ciclosExec --;
				}
				if(quantumPendiente == 0){
					tripulante->estado = READY;
				}
				if(ciclosExec <= 0){

					int socketMongo = enviarA(puertoEIPMongo, avisoTarea, FIN_TAREA);
					close(socketMongo);
					free(avisoTarea);
					avisoTarea = NULL;

					if(esIO(tripulante->instruccionAejecutar->nombreTarea)){
						int socketMongo = enviarA(puertoEIPMongo, tripulante->instruccionAejecutar, TAREA);
						close(socketMongo);
						ciclosBlocked = tripulante->instruccionAejecutar->tiempo;
						tripulante->estado = BLOCKED;
					}
					else{
						recibirProximaTareaDeMiRAM(tripulante);
						ciclosExec = calculoCiclosExec(tripulante);
					}
				}
				sem_post(&tripulante->semaforoFin);
				break;

			case BLOCKED:
				if(tripulante->idTripulante == leerTripulanteBlocked()){
					sleep(retardoCiclosCPU);
					log_info(logDiscordiador,"el tripulante %d esta en block ejecutando, me quedan %d ciclos",
							tripulante->idTripulante, ciclosBlocked);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						modificarTripulanteBlocked(NO_HAY_TRIPULANTE_BLOQUEADO);
						tripulante->estado = READY;
						recibirProximaTareaDeMiRAM(tripulante);
						ciclosExec = calculoCiclosExec(tripulante);
					}
				}
				sem_post(&tripulante->semaforoFin);
				break;

			case SABOTAJE:
				ciclosExec = 0;
				if(distancia(tripulante->coordenadas, sabotaje->coordenadas) > 0){
					desplazarse(tripulante, sabotaje->coordenadas);
					sleep(retardoCiclosCPU);

					log_info(logDiscordiador,"el tripulante %d esta yendo al sabo, esta a %d de distancia",
							tripulante->idTripulante, distancia(tripulante->coordenadas, sabotaje->coordenadas));
				}
				else{
					log_info(logDiscordiador,"el tripulante %d llego a la posicion del SABO %d|%d",
							tripulante->idTripulante, tripulante->coordenadas.posX, tripulante->coordenadas.posX);

					sleep(sabotaje->tiempo);
					sem_post(&sabotaje->semaforoTerminoTripulante);
//					sem_wait(&sabotaje->semaforoTerminoSabotaje);
					sem_wait(&tripulante->semaforoInicio);
					log_info(logDiscordiador,"el tripulante %d ya termino el sabotaje y ya se "
							"restablecio todo, su estado es %s", tripulante->idTripulante,
							traducirEstado(tripulante->estado));
				}
				break;

			case EXIT:
				log_error(logDiscordiador,"el tripulante %d no deberia estar aca", tripulante->idTripulante);
				break;
		}
		if(tripulante->estado != SABOTAJE && tripulante->estado != EXIT){
			sem_wait(&tripulante->semaforoInicio);
		}
	}
	log_info(logDiscordiador,"el tripulante %d con estado %s esta por terminar el hilo", tripulante->idTripulante,
											traducirEstado(tripulante->estado));
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	if(confirmacion(miRAMsocket)){
		log_info(logDiscordiador,"creando la patota %d con %d tripulantes",
				patota->ID, cantidadTripulantes);
    
		for (int i=0; i<cantidadTripulantes; i++){
			iniciarTripulante(*(coordenadas+i), patota->ID);
		}

		/*
		for(int i=0;i<cantidadTripulantes;i++){
			sem_wait(&semUltimoTripu);
		}
		mandarTripulanteNulo();
		*/
	}
	else{
		log_info(logDiscordiador,"No hay espacio suficiente en memoria para iniciar la patota %d",patota->ID);
	}


	eliminarPatota(patota);
	close(miRAMsocket);
}


void iniciarTripulante(t_coordenadas coordenada, uint32_t idPatota){

	t_tripulante* tripulante = malloc(sizeof(t_tripulante));
	pthread_t _hiloTripulante;

	idTripulante++;

	tripulante->coordenadas.posX = coordenada.posX;
	tripulante->coordenadas.posY = coordenada.posY;
	tripulante->idTripulante = idTripulante;
	tripulante->idPatota = idPatota;
	tripulante->estado = NEW;
	//TODO es necesario el malloc?
	tripulante->instruccionAejecutar = malloc(sizeof(t_tarea*));
	tripulante->instruccionAejecutar->nombreTarea = malloc(sizeof(char*));
	sem_init(&tripulante->semaforoInicio, 0, 0);
	sem_init(&tripulante->semaforoFin, 0, 0);
	tripulante->estaVivo = 1;

	if(sabotaje->haySabotaje)
		meterEnLista(tripulante, listaSabotaje);
	else
		meterEnLista(tripulante, listaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d con posicion %d|%d",
			tripulante->idTripulante, tripulante->coordenadas.posX, tripulante->coordenadas.posY);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, tripulante);
	pthread_detach(_hiloTripulante);
}


void actualizarListaNew(){

	log_info(logDiscordiador,"------Iniciando planficacion cola de new con %d tripulantes-----",
				list_size(listaNew->elementos));

	list_iterate(listaNew->elementos, (void*)ponerEnReady);
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
	if(idTripulanteBlocked == NO_HAY_TRIPULANTE_BLOQUEADO && list_size(listaBlocked->elementos) > 0)
		elegirTripulanteAbloquear();
}


void actualizarListaEyB(t_lista* lista, t_estado estado){

	bool tieneDistintoEstado(t_tripulante* tripulante){
		return estado != tripulante->estado;
	}

	lock(&lista->mutex);

	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----",
				traducirEstado(estado), list_size(lista->elementos));
/*
	t_list* listaAux = list_filter(lista->elementos, (void*)tieneDistintoEstado);
	list_iterate(listaAux, (void*)pasarDeLista);
	list_destroy(listaAux);
*/
	while(list_any_satisfy(lista->elementos, (void*)tieneDistintoEstado)){
		t_tripulante* tripulante = (t_tripulante*)list_remove_by_condition
				(lista->elementos, (void*)tieneDistintoEstado);
		pasarDeLista(tripulante);
	}


	log_info(logDiscordiador,"------FIN Planficando cola de %s con %d tripulantes-----",
					traducirEstado(estado), list_size(lista->elementos));

	unlock(&lista->mutex);
}
