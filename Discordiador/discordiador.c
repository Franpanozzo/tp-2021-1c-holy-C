#include "discordiador.h"

int tripulantes;
int main() {
	char * path = pathLog();
	logDiscordiador = iniciarLogger(path,"Discordiador",1);
	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	iniciarMutex();
	iniciarColas();
	iniciarTareasIO();
	sabotaje = malloc(sizeof(t_sabotaje));
	sabotaje->tripulanteSabotaje = malloc(sizeof(t_tripulante));
	sabotaje->tripulanteSabotaje->idTripulante = -1;
	sabotaje->haySabotaje = 0;

	cargar_configuracion();

	idTripulante = 0;
	idPatota = 0;
	idTripulanteBlocked = NO_HAY_TRIPULANTE_BLOQUEADO;
	modificarPlanificacion(PAUSADA);
	idBuscado = -1;


	sem_init(&sabotaje->semaforoIniciarSabotaje,0,0);
	sem_init(&sabotaje->semaforoCorrerSabotaje,0,0);
	sem_init(&sabotaje->semaforoTerminoTripulante,0,0);
	sem_init(&sabotaje->semaforoTerminoSabotaje,0,0);


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


			list_iterate(listaExec->elementos, (void*)esperarTerminarTripulante);
			list_iterate(listaBlocked->elementos, (void*)esperarTerminarTripulante);
			list_iterate(listaNew->elementos, (void*)esperarTerminarTripulante);
//			list_iterate(colaReady->elements, (void*)esperarTerminarTripulante);

			if(sabotaje->tripulanteSabotaje->idTripulante != -1){
				log_info(logDiscordiador,"----- ESPERANDO AL TRIPU SABO -----");
				esperarTerminarTripulante(sabotaje->tripulanteSabotaje);
				log_info(logDiscordiador,"----- YA TERMINE DE ESPERAR AL TRIPU SABO -----");
			}

			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripulantes());
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");


			actualizarLista(listaExec, EXEC);
			actualizarLista(listaBlocked, BLOCKED);
			casoBlocked();
			actualizarLista(listaNew, NEW);
			list_iterate(listaReady->elementos, (void*)esperarTerminarTripulante);
			actualizarLista(listaReady, READY);

			if(sabotaje->haySabotaje && sabotaje->tripulanteSabotaje->idTripulante == -1){ //HAY SABOTAJE
				sem_post(&sabotaje->semaforoIniciarSabotaje);
				sem_wait(&sabotaje->semaforoCorrerSabotaje);//se traba esperando a q se bloqueen a los tripus por el sabotaje
				log_info(logDiscordiador,"----- SIGUE EL PLANI MIENTRAS HAY SABO -----");
			}

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

			list_iterate(listaExec->elementos, (void*)avisarTerminoPlanificacion);
			list_iterate(listaBlocked->elementos, (void*)avisarTerminoPlanificacion);
			list_iterate(listaNew->elementos, (void*)avisarTerminoPlanificacion);
			list_iterate(listaReady->elementos, (void*)avisarTerminoPlanificacion);

			if(sabotaje->tripulanteSabotaje->idTripulante != -1){
				avisarTerminoPlanificacion(sabotaje->tripulanteSabotaje);
			}
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
		list_iterate(listaSabotaje->elementos, (void*)ponerEnSabotaje);

		log_info(logDiscordiador,"----- LA LISTA DE READY QUEDO CON %d TRIPULANTES -----",
				list_size(listaReady->elementos));
		log_info(logDiscordiador,"----- LA LISTA DE EXEC QUEDO CON %d TRIPULANTES -----",
						list_size(listaExec->elementos));
		log_info(logDiscordiador,"----- LA LISTA DE SABOTAJE QUEDO CON %d TRIPULANTES -----",
						list_size(listaSabotaje->elementos));

		elegirTripulanteSabotaje();

		sem_post(&sabotaje->semaforoCorrerSabotaje);//avisarle al planificador que termino la preparacion

		//FINALIZACION SABOTAJE
		sem_wait(&sabotaje->semaforoTerminoTripulante);//esperar q el tripulante termine el sabotaje

		list_iterate(listaSabotaje->elementos, (void*)ponerEnReady);
		list_add_all(listaReady->elementos, listaSabotaje->elementos);
		list_iterate(listaReady->elementos, (void*)avisarTerminoPlanificacion);
		list_add(listaReady->elementos, sabotaje->tripulanteSabotaje);
		list_clean(listaSabotaje->elementos);
		sabotaje->haySabotaje = 0;
		sabotaje->tripulanteSabotaje->idTripulante = -1;

		log_info(logDiscordiador,"----- LA LISTA DE READY FINAL SABOTAJE QUEDO CON %d TRIPULANTES -----",
						list_size(listaReady->elementos));


		sem_post(&sabotaje->semaforoTerminoSabotaje);//avisarle al tripulante que ya volvio _todo a la normalidad
	}
}


void hiloTripulante(t_tripulante* tripulante){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	int quantumPendiente = quantum;
	while(tripulante->estaVivo){

		if(tripulante->estado != NEW){
			actualizarEstadoEnRAM(tripulante);
		}
		switch(tripulante->estado){
			case NEW:
				log_info(logDiscordiador,"tripulanteId %d: estoy en new", tripulante->idTripulante);
				if(sabotaje->haySabotaje == 1)
					tripulante->estado = SABOTAJE;
				else
					tripulante->estado = READY;
				recibirPrimerTareaDeMiRAM(tripulante);
				break;
			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
				log_info(logDiscordiador,"el tripulante %d esta en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec);
				tripulante->estado = EXEC;
				break;
			case EXEC:
				log_info(logDiscordiador,"el tripulante %d esta en exec con %d ciclos y %d quantum",
						tripulante->idTripulante, ciclosExec, quantumPendiente);

				if(distancia(tripulante->coordenadas, tripulante->instruccionAejecutar->coordenadas) > 0){
					desplazarse(tripulante, tripulante->instruccionAejecutar->coordenadas);
				}
				ciclosExec --;
				quantumPendiente--;
				sleep(retardoCiclosCPU);
				if(quantumPendiente == 0){
					tripulante->estado = READY;
				}
				if(ciclosExec <= 0){
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
				break;
			case BLOCKED:
				if(tripulante->idTripulante == leerTripulanteBlocked()){
					sleep(retardoCiclosCPU);
					lock(mutexLogDiscordiador);
					log_info(logDiscordiador,"el tripulante %d esta en block ejecutando, me quedan %d ciclos",
							tripulante->idTripulante, ciclosBlocked);
					unlock(mutexLogDiscordiador);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						modificarTripulanteBlocked(NO_HAY_TRIPULANTE_BLOQUEADO);
						tripulante->estado = READY;
						recibirProximaTareaDeMiRAM(tripulante);
						ciclosExec = calculoCiclosExec(tripulante);
					}
				}
				break;
			case SABOTAJE:
				ciclosExec = 0;

				log_info(logDiscordiador,"el tripulante %d entro al case SABO",
						tripulante->idTripulante);

				if(distancia(tripulante->coordenadas, sabotaje->coordenadas) > 0){
					desplazarse(tripulante, sabotaje->coordenadas);
					sleep(retardoCiclosCPU);

					log_info(logDiscordiador,"el tripulante %d esta yendo al sabo, "
							"esta a %d de distancia", tripulante->idTripulante,
							distancia(tripulante->coordenadas, sabotaje->coordenadas));
				}
				else{

					log_info(logDiscordiador,"el tripulante %d llego a la posicion "
							"del SABO %d|%d", tripulante->idTripulante,
							tripulante->coordenadas.posX, tripulante->coordenadas.posX);

					sleep(sabotaje->tiempo);
					sem_post(&sabotaje->semaforoTerminoTripulante);
					sem_wait(&sabotaje->semaforoTerminoSabotaje);
				}
				break;
		}
		sem_post(&tripulante->semaforoFin);
		sem_wait(&tripulante->semaforoInicio);
	}
	liberarTripulante(tripulante);
}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	char* confirmacion = esperarConfirmacionDePatotaEnRAM(miRAMsocket);
	if(strcmp(confirmacion,"OK") == 0){
		log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

		for (int i=0; i<cantidadTripulantes; i++){
			totalTripus ++;
			log_info(logDiscordiador,"---------posx:%d;posy:%d---------",coordenadas[i].posX,coordenadas[i].posY);
			iniciarTripulante(*(coordenadas+i), patota->ID);
		}
	}
	else{
		log_info(logDiscordiador,"No hay espacio suficiente en memoria para iniciar la patota %d",patota->ID);
	}
	free(patota);
	free(confirmacion);
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

	if(sabotaje->haySabotaje == 1)
		meterEnLista(tripulante, listaSabotaje);
	else
		meterEnLista(tripulante, listaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d",tripulante->idTripulante);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, (void*) tripulante);
	pthread_detach(_hiloTripulante);
}


void actualizarLista(t_lista* lista, t_estado estado){

	estadoAcomparar = estado;

	lock(lista->mutex);

	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----",
				traducirEstado(estado), list_size(lista->elementos));

	t_list* listaAux = list_filter(lista->elementos, tieneDistintoEstado);
	if(estado == READY){
		int cantidadApasar = gradoMultiprocesamiento - list_size(listaExec->elementos);

		for(int i=cantidadApasar; i<list_size(listaAux); i++){
			t_tripulante* tripulante = (t_tripulante*) list_get(listaAux, i);
			tripulante->estado = READY;
		}
		listaAux = list_take(listaAux, cantidadApasar);
	}

	lista->elementos = list_filter(lista->elementos, tieneIgualEstado);
	list_iterate(listaAux, (void*)pasarDeLista);

	log_info(logDiscordiador,"------FIN Planficando cola de %s con %d tripulantes-----",
					traducirEstado(estado), list_size(lista->elementos));

	unlock(lista->mutex);
}



