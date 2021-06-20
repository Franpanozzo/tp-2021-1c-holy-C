#include "discordiador.h"

int tripulantes;
int main() {
	char * path = pathLog();
	logDiscordiador = iniciarLogger(path,"Discordiador",1);
	crearConfig(); // Crear config para puerto e IP de Mongo y Ram

	iniciarSemaforos();
	iniciarMutex();
	iniciarColas();
	iniciarTareasIO();
	cargarConfiguracion();

	idTripulante = 0;
	idPatota = 0;
	idTripulanteBlocked = NO_HAY_TRIPULANTE_BLOQUEADO;
	modificarPlanificacion(PAUSADA);

	sabotaje = malloc(sizeof(t_sabotaje));
	sabotaje->tripulanteSabotaje = NULL;
	sabotaje->haySabotaje = 0;

	sem_init(&sabotaje->semaforoIniciarSabotaje,0,0);
	sem_init(&sabotaje->semaforoCorrerSabotaje,0,0);
	sem_init(&sabotaje->semaforoTerminoTripulante,0,0);
	sem_init(&sabotaje->semaforoTerminoSabotaje,0,0);


	pthread_create(&planificador, NULL, (void*) hiloPlanificador, NULL);
	pthread_detach(planificador);
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
		if(leerPlanificacion() == CORRIENDO && leerTotalTripus() > 0){

			list_iterate(colaExec->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaBlocked->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaNew->elements, (void*)esperarTerminarTripulante);
			list_iterate(colaReady->elements, (void*)esperarTerminarTripulante);

			if(sabotaje->tripulanteSabotaje != NULL){
				esperarTerminarTripulante(sabotaje->tripulanteSabotaje);
			}

			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripus);
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");

			actualizarCola(EXEC, colaExec, mutexColaExec);
			actualizarCola(BLOCKED, colaBlocked, mutexColaBlocked);
			casoBlocked();
			actualizarCola(NEW, colaNew, mutexColaNew);
			actualizarCola(READY, colaReady, mutexColaReady);

			if(sabotaje->haySabotaje){ //HAY SABOTAJE
				sem_post(&sabotaje->semaforoIniciarSabotaje);
				sem_wait(&sabotaje->semaforoCorrerSabotaje);//se traba esperando a q se bloqueen a los tripus por el sabotaje
			}

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

			list_iterate(colaExec->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaBlocked->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaNew->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaReady->elements, (void*)avisarTerminoPlanificacion);

			if(sabotaje->tripulanteSabotaje != NULL){
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

		sabotaje->haySabotaje = 1;
		sem_wait(&sabotaje->semaforoIniciarSabotaje);//esperar a que el plani termine de actualizar las colas

		sabotaje->tripulanteSabotaje = elTripuMasCerca(sabotaje->coordenadas);
		t_estado estadoAnterior = sabotaje->tripulanteSabotaje->estado;
		sabotaje->tripulanteSabotaje->estado = SABOTAJE;

		list_sort(colaExec->elements, tripulanteDeMenorId);
		list_sort(colaReady->elements, tripulanteDeMenorId);
		list_add_all(colaSabotaje->elements, colaExec->elements);
		list_add_all(colaSabotaje->elements, colaReady->elements);
		list_clean(colaExec->elements);
		list_clean(colaReady->elements);

		sem_post(&sabotaje->semaforoCorrerSabotaje);//destrabar el plani

		sem_wait(&sabotaje->semaforoTerminoTripulante);//esperar q el tripu termine el sabotaje

		//reestablecer todo_ como estaba antes
		sabotaje->haySabotaje = 0;

		sem_post(&sabotaje->semaforoTerminoSabotaje);//avisarle al tripu
	}
}


void hiloTripulante(t_tripulante* tripulante){
	int ciclosExec = 0;
	int ciclosBlocked = 0;
	//int ciclosSabo = 0;
	int quantumPendiente = quantum;
	while(tripulante->estado != END){

		if(tripulante->estado != NEW){
			actualizarEstadoEnRAM(tripulante);
		}
			//LO PUSE ACA POR EL PROBLEMA DE Q SI LO PONGO ABAJO LE VA A
		//MANDAR UN ESTDO INCORRECTO EN EL CADO DE QUE NO PUEDA ENTRAR A EXEC
		//EL OTRO PROBLEMA ES QUE LA PRIMERA VEZ LE VA ENVIAR UN TIPU SIN TAREA A RAM
		switch(tripulante->estado){
			case NEW:
				log_info(logDiscordiador,"tripulanteId %d: estoy en new", tripulante->idTripulante);
				recibirPrimerTareaDeMiRAM(tripulante);
				if(strcmp(tripulante->instruccionAejecutar->nombreTarea, "TAREA_ERROR") == 0){
					tripulante->estado = END;
					log_info(logDiscordiador,"El tripulante ID: %d de la patota ID: %d no ha sido alocado en memoria por estar sin espacio",tripulante->idTripulante,tripulante->idPatota);
				}
				else{
					tripulante->estado = READY;
				}

				break;
			case READY:
				if(ciclosExec == 0){
					ciclosExec = calculoCiclosExec(tripulante);
				}
				quantumPendiente = quantum;
				log_info(logDiscordiador,"tripulanteId %d: estoy en ready con %d ciclos exec",
						tripulante->idTripulante, ciclosExec);
				tripulante->estado = EXEC;
				break;
			case EXEC:
				log_info(logDiscordiador,"tripulanteId %d: estoy en exec con %d ciclos y %d quantum",
						tripulante->idTripulante, ciclosExec, quantumPendiente);
				desplazarse(tripulante, tripulante->instruccionAejecutar->coordenadas);
				ciclosExec --;
				quantumPendiente--;
				sleep(3);
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
						siguienteTarea(tripulante, &ciclosExec);
					}
				}
				break;
			case BLOCKED:
				if(tripulante->idTripulante == leerTripulanteBlocked()){
					sleep(3);
					lock(mutexLogDiscordiador);
					log_info(logDiscordiador,"tripulanteId %d: estoy en block ejecutando, me quedan %d ciclos",
							tripulante->idTripulante, ciclosBlocked);
					unlock(mutexLogDiscordiador);
					ciclosBlocked --;
					if(ciclosBlocked == 0){
						modificarTripulanteBlocked(NO_HAY_TRIPULANTE_BLOQUEADO);
						tripulante->estado = READY;
						siguienteTarea(tripulante, &ciclosExec);
					}
				}
				break;
			case END:
				//No va nada acá porque sobreloguea mucho
				break;

			case SABOTAJE:
				desplazarse(tripulante, sabotaje->coordenadas);
				sabotaje->tiempo --;
				if(sabotaje->tiempo <= 0){
					sem_post(&sabotaje->semaforoTerminoTripulante);
					sem_wait(&sabotaje->semaforoTerminoSabotaje);
				}

				break;
		}
		sem_post(&tripulante->semaforoFin);
		sem_wait(&tripulante->semaforoInicio);
	}

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
		log_info(logDiscordiador,"No hay espacio suficiente en memoria para iniciar la patota ID: %d",patota->ID);
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
	tripulante->instruccionAejecutar = malloc(sizeof(t_tarea*));
	tripulante->instruccionAejecutar->nombreTarea = malloc(sizeof(char*));
	sem_init(&tripulante->semaforoInicio, 0, 0);
	sem_init(&tripulante->semaforoFin, 0, 0);


	lock(mutexColaNew);
	queue_push(colaNew,(void*)tripulante);
	unlock(mutexColaNew);

	log_info(logDiscordiador,"Se creo el tripulante numero %d",tripulante->idTripulante);

	pthread_create(&_hiloTripulante, NULL, (void*) hiloTripulante, (void*) tripulante);
	pthread_detach(_hiloTripulante);
}


void actualizarCola(t_estado estado, t_queue* cola, pthread_mutex_t colaMutex){

	t_tripulante* tripulante;
	int tamanioInicialCola = queue_size(cola);
	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----", traducirEstado(estado), tamanioInicialCola);

	for(int i=0; i<tamanioInicialCola; i++){
		tripulante = (t_tripulante*) queue_pop(cola);

		if(tripulante->estado != estado){
			if(estado == READY){
				if(queue_size(colaExec) < gradoMultiprocesamiento || tripulante->estado == END){
					pasarDeCola(tripulante);
				}
				else{
					tripulante->estado = estado;
					lock(colaMutex);
					queue_push(cola, tripulante);
					unlock(colaMutex);
				}
			}
			else{
				pasarDeCola(tripulante);
			}
		}
		else{
			lock(colaMutex);
			queue_push(cola, tripulante);
			unlock(colaMutex);
		}
	}
}




