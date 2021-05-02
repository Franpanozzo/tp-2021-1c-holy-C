#ifndef EJERCICIO10_H_
#define EJERCICIO10_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <memory.h>
#include <commons/config.h>


void iniciar_conexion(void*);
void comunicarse();
void crearConfig();

#endif /* EJERCICIO10_H_ */

typedef struct{
	char* IP;
	int puerto;
} puertoEIP;

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    uint8_t codigo_operacion;
    t_buffer* buffer;
} t_paquete;

typedef struct {
	int longitud;
	char* contenidoMensaje; //
} t_mensaje;

typedef enum{
	STRING = 0,
	INT = 1
} tipoDeDato;


