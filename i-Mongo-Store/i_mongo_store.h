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
#include <string.h>
#include <commons/config.h>


typedef enum{
	PERSONA
} tipoDeDato;

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
    tipoDeDato codigo_operacion;
    t_buffer* buffer;
} t_paquete;

typedef struct {
	uint32_t dni;
	uint8_t edad;
	uint32_t pasaporte;
	uint32_t nombre_length;
	char* nombre;
} t_persona;


t_persona* deserializarPersona(t_buffer*);
void iniciarConexion();
t_paquete* recibirPaquete();
void enviarPaquete(t_persona);


#endif




