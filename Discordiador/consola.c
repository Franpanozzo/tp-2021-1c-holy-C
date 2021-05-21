#include "consola.h"


void leerConsola(){
	char* leido;

	leido = readline("Discordiador-->");

	string_trim(&leido);

	char** comandoYparametros = string_split(leido, " ");
	int cursor = 0;
	///home/utnso/tp-2021-1c-holy-C/Discordiador/tareas.txt
	if(strcmp(comandoYparametros[cursor], "INICIAR_PATOTA") == 0){

		log_info(logger, "\nSe ingreso el comando iniciar patota");
		cursor ++;

		uint32_t cantidadTripulantes = procesarCantidadTripulantes(comandoYparametros, &cursor);
		char* tareas = procesarPathTareas(comandoYparametros, &cursor);
		t_coordenadas* coordenadasTripulantes= procesarPosicionesTripulantes(comandoYparametros, cantidadTripulantes, &cursor);

		//iniciarPatota(coordenadasTripulantes, tareas, uint32_t cantidadTripulantes);

		//logear todo el comando
		log_info(logger, " con parametros: \n  cantidad de tripulantes: %d"
				"\n  tareas: %s"
				"\n posiciones:", cantidadTripulantes, tareas);
		for(int c=0; c<cantidadTripulantes; c++){
			log_info(logger, " %d|%d", coordenadasTripulantes[c].posX, coordenadasTripulantes[c].posY);
		}

	}
	else{
		log_error(logger, "\n No se reconoce el comando %s\n", comandoYparametros[cursor]);
	}

	free(leido);
}


uint32_t procesarCantidadTripulantes(char** parametros, int* cursor){

	if(parametros[*cursor] == NULL){
		log_error(logger, "\n Faltan parametros en el comando iniciar patota, no se ingreso la cantidad de tripulantes \n");
		exit(1);
	}

	int cantidad = atoi(parametros[*cursor]);

	if(cantidad <= 0){
		log_error(logger,"\n Se ingreso una cantidad no valida de tripulantes: %d \n", cantidad);
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
				log_info(logger, "La coordenada numero %d esta incompleta \n", c);
			}

			coordenadasTripulantes[c].posX = atoi(coordenadasString[0]);
			coordenadasTripulantes[c].posY = atoi(coordenadasString[1]);

			*cursor = *cursor + 1;
		}
	}

	if(parametros[*cursor] != NULL){
		log_info(logger,"Se ingresaron coordenadas de mas, solo se tendran en cuenta "
				"las primeras %d coordenadas \n", cantidadTripulantes);
	}

	return coordenadasTripulantes;
}


char* procesarPathTareas(char** parametros, int* cursor){

	if(parametros[*cursor] == NULL){
		log_error(logger,"\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso el path de las tareas");
					//break se tiene q romper el leer consola
		}

	FILE* archivo = fopen(parametros[*cursor], "r");

	if(archivo == NULL){
		log_error(logger,"\n No se encontro el archivo con las tareas, revise la direccion ingresada %s", parametros[*cursor]);
	}

	char* unaTarea = malloc(sizeof(char) * 50);
	char* tareas = string_new();

	while(!feof(archivo)){
		fgets(unaTarea, 50, archivo);
		string_append(&tareas, "\n");
		string_append(&tareas, unaTarea);
	}

	free(unaTarea);
	fclose(archivo);
	*cursor = *cursor + 1;
	return tareas;
}
