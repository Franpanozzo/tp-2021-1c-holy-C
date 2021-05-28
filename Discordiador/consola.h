#ifndef CONSOLA_H_
#define CONSOLA_H_

	#include <bibliotecas.h>
	#include <commons/collections/queue.h>
	#include <commons/string.h>
	#include <readline/readline.h>
	#include <readline/history.h>
	#include "discordiador.h"

	t_coordenadas* procesarPosicionesTripulantes(char**, int, int*);
	char* procesarPathTareas(char**, int*);
	uint32_t procesarCantidadTripulantes(char**, int*);
	void leerConsola();


#endif
