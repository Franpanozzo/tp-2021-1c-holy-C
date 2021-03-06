#include "consola.h"

//INICIAR_PATOTA 3 /home/utnso/tp-2021-1c-holy-C/Discordiador/tareas.txt 3|1

void leerConsola(){
	char* leido;
	char** comandoYparametros;
	int cursor;

	while(1){
		leido = readline("Discordiador-->");
		string_trim(&leido);
		comandoYparametros = string_split(leido, " ");
		cursor = 0;

		if(strcmp(comandoYparametros[cursor], "INICIAR_PATOTA") == 0){

			printf("Se ingreso el comando iniciar patota");
			cursor ++;

			uint32_t cantidadTripulantes = procesarCantidadTripulantes(comandoYparametros, &cursor);
			char* tareas = procesarPathTareas(comandoYparametros, &cursor);
			log_info(logDiscordiador, "Las tareas son: %s",tareas);
			t_coordenadas* coordenadasTripulantes= procesarPosicionesTripulantes(comandoYparametros, cantidadTripulantes, &cursor);

			iniciarPatota(coordenadasTripulantes, tareas, cantidadTripulantes);

			free(coordenadasTripulantes);
//			free(tareas);

		}
		else if (strcmp(comandoYparametros[cursor], "INICIAR_PLANIFICACION") == 0){
			if(leerPlanificacion() == PAUSADA){
				modificarPlanificacion(CORRIENDO);
				printf("Se inicio la planificacion");
				sem_post(&semPlanificacion);
			}
			else{
				printf("La planificacion ya esta iniciada");
			}
		}
		else if (strcmp(comandoYparametros[cursor], "PAUSAR_PLANIFICACION") == 0){
			if(leerPlanificacion() == CORRIENDO){
				modificarPlanificacion(PAUSADA);
				printf("Se pauso la planificacion");
				sem_wait(&semPlanificacion);
			}
			else{
				log_info(logDiscordiador, "La planificacion ya esta pausada");
			}
		}

		else if (strcmp(comandoYparametros[cursor], "LISTAR_TRIPULANTES") == 0){
			listarTripulantes();
		}
		else if (strcmp(comandoYparametros[cursor], "ELIMINAR_TRIPULANTE") == 0){
			printf("Se ingreso el comando ELIMINAR_TRIPULANTE");
			cursor ++;
			uint32_t idTripulante = procesarCantidadTripulantes(comandoYparametros, &cursor);
			eliminarTripulante(idTripulante);

		}
		else if (strcmp(comandoYparametros[cursor], "SABOTAJE") == 0){
			printf("Se ingreso el comando SABOTAJE");
			cursor ++;
			sabotaje->coordenadas.posX = 6;
			sabotaje->coordenadas.posY = 6;
			sabotaje->haySabotaje = 1;
		}
		else if(strcmp(comandoYparametros[cursor], "OBTENER_BITACORA") == 0){

			printf("Se ingreso el comando OBTENER_BITACORA");
			cursor ++;
			uint32_t idTripulante = procesarCantidadTripulantes(comandoYparametros, &cursor);
			int socketMongo = enviarA(puertoEIPMongo, &idTripulante, OBTENER_BITACORA);
			recibirBitacora(socketMongo, idTripulante);
			close(socketMongo);
		}
		else{
			printf("\n No se reconoce el comando %s\n", comandoYparametros[cursor]);
		}
		liberarStringSplit(comandoYparametros);
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
			*cursor = *cursor + 1;
			liberarStringSplit(coordenadasString);
		}
	}

	if(parametros[*cursor] != NULL){
		log_info(logDiscordiador,"Se ingresaron coordenadas de mas, solo se tendran en cuenta "
				"las primeras %d coordenadas", cantidadTripulantes);
	}

	return coordenadasTripulantes;
}
void liberarStringSplit(char** arrayParametros) {

	char** liberadorDeStrings = arrayParametros;

	while((*liberadorDeStrings) != NULL) {

		free(*liberadorDeStrings);
		liberadorDeStrings++;
		//if((*liberadorDeStrings) == NULL) free(arrayParametros);

	}

	free(arrayParametros);
}

char* procesarPathTareas(char** parametros, int* cursor){

	if(parametros[*cursor] == NULL){
		log_error(logDiscordiador,"Faltan parametros en el comando INICIAR_PATOTA, "
				"no se ingreso el path de las tareas");
					//break se tiene q romper el leer consola
		}

	FILE* archivo = fopen(parametros[*cursor], "r");

	if(archivo == NULL){
		log_error(logDiscordiador,"No se encontro el archivo con las tareas, "
				"revise la direccion ingresada %s", parametros[*cursor]);
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
