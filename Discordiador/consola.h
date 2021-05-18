/*logInfo(logger, "\nSe ingreso el comando INICIAR_PATOTA con parametros:\n "
				"cantidad de tripulantes = " %s "\n"
				"tareas de la patota = " %s "\n"
				"posiciones de los tripulantes = " %s "\n"
				);*/
#ifndef CONSOLA_H_
#define CONSOLA_H_

	#include <bibliotecas.h>
	#include <commons/collections/queue.h>
	#include <commons/string.h>
	#include <readline/readline.h>
	#include <readline/history.h>

	t_coordenadas* procesarPosicionesTripulantes(char**, int, int*);
	char* procesarPathTareas(char**, int*);
	int procesarCantidadTripulantes(char**, int*);
	void leerConsola();


#endif
