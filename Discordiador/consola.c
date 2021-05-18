#include "consola.h"


void leerConsola(){
	//t_log* logger = iniciarLogger("consola.log", "Discordiador", int flagConsola)
	char* leido;

	leido = readline(">");

	//string_trim(&leido);
	//esto elimina todos los espacio de la izq y de la der?

	char** comandoYparametros = string_split(leido, " ");
	int cursor = 0;

	if(strcmp(*(comandoYparametros + cursor), "INICIAR_PATOTA") == 0){

		printf("\nSe ingreso el comando %s", "INICIAR_PATOTA");
		//log_info(logger, "\nSe ingreso el comando %s", "INICIAR_PATOTA");
		cursor ++;

		//int cantidadTripulantes = procesarCantidadTripulantes(comandoYparametros, &cursor);
		//char* listaTares = procesarPathTareas(comandoYparametros, &cursor);
		//t_coordenadas* coordenadasTripulantes= procesarPosicionesTripulantes(comandoYparametros, cantidadTripulantes, cursor);

	}
	else{
		printf("No se reconoce el comando");
	}

	free(leido);
}


int procesarCantidadTripulantes(char** parametros, int* cursor){

	if(*(parametros + *cursor) == NULL){
		printf("\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso la cantidad de tripulantes");
		//log_error(logger,"\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso la cantidad de tripulantes");
				//break se tiene q romper el leer consola
	}

	int cantidad = atoi((*parametros + *cursor));

	if(cantidad <= 0){
		printf("\n Se ingreso una cantidad no valida de tripulantes: %d", cantidad);
		//log_error(logger,"\n Se ingreso una cantidad no valida de tripulantes: %d", cantidad);
		//break se tiene q romper el leer consola
	}

	cursor ++;
	return cantidad;
}

t_coordenadas* procesarPosicionesTripulantes(char** parametros, int cantidadTripulantes, int* cursor){

	t_coordenadas coordenadasTripulantes[cantidadTripulantes];
	//t_coordenadas* coordenadasTripulantes = malloc(sizeof(t_coordenadas) * cantidadTripulantes);

	for(int c = 0; c < cantidadTripulantes; c++){

		if(*(parametros + *cursor) == NULL){
			//memcpy(coordenadasTripulantes->posX + c, &cero, sizeof(uint32_t));
			coordenadasTripulantes[c].posX = 0;
			coordenadasTripulantes[c].posY = 0;
		}
		else{
			char** coordenadasString = string_split(*(parametros + *cursor), "|");

			coordenadasTripulantes[c].posX = atoi(*coordenadasString);
			coordenadasTripulantes[c].posY = atoi(*(coordenadasString + 1));

			cursor++;
		}
	}

	return coordenadasTripulantes;
}


char* procesarPathTareas(char** parametros, int* cursor){

	if(*(parametros + *cursor) == NULL){
		printf("\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso el path de las tareas");
		//log_error(logger,"\n Faltan parametros en el comando INICIAR_PATOTA, no se ingreso el path de las tareas");
					//break se tiene q romper el leer consola
		}

	FILE* archivo = fopen(*(parametros + *cursor), "r");

	if(archivo == NULL){
		printf("\n No se encontro el archivo con las tareas, revise la direccion ingresada %s", *(parametros + *cursor));
		//log_error(logger,"\n No se encontro el archivo con las tareas, revise la direccion ingresada %s", *(parametros + cursor));
	}

	char* unaTarea;
	char* tareas;

	while(!feof(archivo)){
		fgets(unaTarea, 200, archivo);
		string_append(&tareas, "\n");
		string_append(&tareas, unaTarea);
	}

	fclose(archivo);
	cursor++;
	return tareas;
}


