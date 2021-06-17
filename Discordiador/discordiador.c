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
	cargar_configuracion();

	idTripulante = 0;
	idPatota = 0;
	idTripulanteBlocked = NO_HAY_TRIPULANTE_BLOQUEADO;
	modificarPlanificacion(PAUSADA);
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

			log_info(logDiscordiador,"----- TOTAL TRIPUS: %d ----", totalTripus);
			log_info(logDiscordiador,"----- COMIENZA LA PLANI ----");

			actualizarCola(EXEC, colaExec, mutexColaExec);
			actualizarCola(BLOCKED, colaBlocked, mutexColaBlocked);
			casoBlocked();
			actualizarCola(NEW, colaNew, mutexColaNew);
			actualizarCola(READY, colaReady, mutexColaReady);

/*
			if(haySabotaje){ // HAY SABOTAJE
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

*/

//			planificadorFin = 0;

			log_info(logDiscordiador,"----- TERMINA LA PLANI -----");

			list_iterate(colaExec->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaBlocked->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaNew->elements, (void*)avisarTerminoPlanificacion);
			list_iterate(colaReady->elements, (void*)avisarTerminoPlanificacion);

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

		haySabotaje = 1;
//		sem_wait(&semaforoSabotajeResuelto);
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
				tripulante->estado = READY;
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
				desplazarse(tripulante);
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

			case SABOTAJE: // para algunos es como un bloqueo donde no se hace nada
/*				if(tripu->idTripulante == tripulanteDesabotaje->idTripulante){
					sleep(1);
					ciclosSabo --;
					if(ciclosSabo > 0){
						desplazarse(tripu);
					}
					else{

					 *    FIN DE SABO
					 * hay q pasar a todos en su respectivo orden a sus estados anteriores
					 * y hacer q el tripu q se movio hasta para solucionar el sabo re calcule
					 * sus cilos de ejecucion teniendo en cuenta q se tiene q volver a
					 * desplazar y q puede ser q ya haya hecho parte de la tarea

					sem_post(semaforoSabo); //este semaforo le permite recuperar su "imagen"
					//el wait se hace dentro del hiloSabotaje antes de q devolverle la imagen
					//y el semaforo se inicializa en 0
					sem_wait(semaforoImagen); //lo hago para asegurarme q le devuelva la imagen
					}
				}
				sem_post(semaforoPlanificadorInicio);
*/
				break;
		}
		sem_post(&tripulante->semaforoFin);
		sem_wait(&tripulante->semaforoInicio);
		}

}


void iniciarPatota(t_coordenadas* coordenadas, char* tareasString, uint32_t cantidadTripulantes){

	t_patota* patota = asignarDatosAPatota(tareasString);
	int miRAMsocket = enviarA(puertoEIPRAM, patota, PATOTA);
	esperarConfirmacionDeRAM(miRAMsocket);
	log_info(logDiscordiador,"creando patota, con %d tripulantes", cantidadTripulantes);

	for (int i=0; i<cantidadTripulantes; i++){
		totalTripus ++;
		log_info(logDiscordiador,"---------posx:%d;posy:%d---------",coordenadas[i].posX,coordenadas[i].posY);
		iniciarTripulante(*(coordenadas+i), patota->ID);
	}
	free(patota);
	close(miRAMsocket);
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
	char * _estado = traducirEstado(estado);
	log_info(logDiscordiador,"------Planficando cola de %s con %d tripulantes-----", _estado, tamanioInicialCola);

	for(int i=0; i<tamanioInicialCola; i++){
		lock(colaMutex);
		tripulante = (t_tripulante*) queue_pop(cola);
		unlock(colaMutex);

		if(tripulante->estado != estado /*|| tripulante->estado == END*/){
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
			tripulante->estado = estado;
			lock(colaMutex);
			queue_push(cola, tripulante);
			unlock(colaMutex);
		}

		//sem_post(&tripulante->semaforoInicio);
	}
	free(_estado);
}




