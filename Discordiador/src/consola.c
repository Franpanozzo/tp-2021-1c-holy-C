#include "consola.h"

//INICIAR_PATOTA 3 /home/utnso/tp-2021-1c-holy-C/Discordiador/tareas.txt 3|1

void leerConsola(){
	char* leido;
	char** comandoYparametros;
	int cursor;
	int primeraVez = 1;

	while(1){
		leido = readline("Discordiador-->");
		string_trim(&leido);
		comandoYparametros = string_split(leido, " ");
		cursor = 0;

		if(strcmp(comandoYparametros[cursor], "INICIAR_PATOTA") == 0){

			log_info(logDiscordiador, "\nSe ingreso el comando iniciar patota");
			cursor ++;

			uint32_t cantidadTripulantes = procesarCantidadTripulantes(comandoYparametros, &cursor);
			log_info(logDiscordiador, "La cantidad de tripulantes es: %d\n",cantidadTripulantes);
			char* tareas = procesarPathTareas(comandoYparametros, &cursor);
			log_info(logDiscordiador, "Las tareas son: %s\n",tareas);
			t_coordenadas* coordenadasTripulantes= procesarPosicionesTripulantes(comandoYparametros, cantidadTripulantes, &cursor);
			for(int i=0; i<cantidadTripulantes; i++){
				log_info(logDiscordiador, "Las coordenadas son %d , %d \n",coordenadasTripulantes[i].posX,coordenadasTripulantes[i].posY);
			}


			t_coordenadas coordenadas[cantidadTripulantes];
			for(int i=0; i<cantidadTripulantes;i++){
				coordenadas[i].posX = i+ 2;
				coordenadas[i].posY = i + 2;
			}

			//CONSUMIR_COMIDA;3;8;9\nGENERAR_BASURA;6;7;1\nGENERAR_COMIDA 8;5;1;2
			//iniciarPatota(coordenadas, "COMER_YOGUR;2;3;7\nGENERAR_OXIGENO 4;2;3;7", tripulantes);
			iniciarPatota(coordenadasTripulantes, tareas, cantidadTripulantes);
			//LAS tareas procesadas andan, las coordenadas procesadas no
		}
		else if (strcmp(comandoYparametros[cursor], "INICIAR_PLANIFICACION") == 0){
			//sem_post(&semPlanificacion);//le permites arrancar a planificar las listas/colas
			planificacionPlay = 1;
			if(primeraVez){

				pthread_create(&planificador, NULL, (void*) hiloPlani, NULL);
				//pthread_join(planificador, (void**) NULL);
				pthread_detach(planificador);
				primeraVez =0;
			}


			log_info(logDiscordiador, "\nSe ingreso el comando iniciar planificacion");
			sleep(1);
					cursor ++;
		}
		else if (strcmp(comandoYparametros[cursor], "PAUSAR_PLANIFICACION") == 0){
			planificacionPlay = 0;
			log_info(logDiscordiador, "\nSe ingreso el comando pausar planificacion");
			sleep(1);
			cursor ++;
		}
		else{
			log_error(logDiscordiador, "\n No se reconoce el comando %s\n", comandoYparametros[cursor]);
		}

		free(leido);
	}

}


uint32_t procesarCantidadTripulantes(char** parametros, int* cursor){

	if(parametros[*cursor] == NULL){
		log_error(logDiscordiador, "\n Faltan parametros en el comando iniciar patota, no se ingreso la cantidad de tripulantes \n");
		exit(1);
	}

	int cantidad = atoi(parametros[*cursor]);

	if(cantidad <= 0){
		log_error(logDiscordiador,"\n Se ingreso una cantidad no valida de tripulantes: %d \n", cantidad);
	}

	*cursor = *cursor+1;
	return cantidad;
}


t_coordenadas* procesarPosicionesTripulantes(char** parametros, int cantidadTripulantes, int* cursor){

	t_coordenadas* coordenadasTripulantes = malloc(sizeof(t_coordenadas) * cantidadTripulantes);
	char** coordenadasString;

	for(int c = 0; c < cantidadTripulantes; c++){

		if(parametros[*cursor] == NULL){
			coordenadasTripulantes[c].posX = 0;
			coordenadasTripulantes[c].posY = 0;
		}
		else{
			coordenadasString = string_split(parametros[*cursor], "|");

			if(strcmp(coordenadasString[1], "") == 0){
				log_info(logDiscordiador, "La coordenada numero %d esta incompleta \n", c);
			}

			coordenadasTripulantes[c].posX = (uint32_t) strtoul(coordenadasString[0], NULL,10);
			coordenadasTripulantes[c].posY = (uint32_t) strtoul(coordenadasString[1], NULL,10);
			log_info(logDiscordiador,"---------------posx:%d;posy:%d---------------",coordenadasTripulantes[c].posX,coordenadasTripulantes[c].posY);
			*cursor = *cursor + 1;
		}
	}

	if(parametros[*cursor] != NULL){
		log_info(logDiscordiador,"Se ingresaron coordenadas de mas, solo se tendran en cuenta "
				"las primeras %d coordenadas \n", cantidadTripulantes);
	}

	return coordenadasTripulantes;
}


char* procesarPathTareas(char** parametros, int* cursor){

	if(parametros[*cursor] == NULL){
		log_error(logDiscordiador,"\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso el path de las tareas");
					//break se tiene q romper el leer consola
		}

	FILE* archivo = fopen(parametros[*cursor], "r");

	if(archivo == NULL){
		log_error(logDiscordiador,"\n No se encontro el archivo con las tareas, revise la direccion ingresada %s", parametros[*cursor]);
	}

	char* unaTarea = malloc(sizeof(char) * 50);
	char* tareas = string_new();

	rewind(archivo);

	while(!feof(archivo)){
		fgets(unaTarea, 50, archivo);
		//string_append(&tareas, "\n");
		string_append(&tareas, unaTarea);
	}

	free(unaTarea);
	fclose(archivo);
	*cursor = *cursor + 1;
	return tareas;
}
