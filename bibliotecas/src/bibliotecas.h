#ifndef BIBLIOTECAS_H_
#define BIBLIOTECAS_H_


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
#include <commons/log.h>

typedef struct{
	char* IP;
	int puerto;
} puertoEIP;

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


t_log* iniciarLogger(char*, char*, int);
void deserializarPersona(t_buffer*);
void recibirPaquete(int);
void* serializarPaquete(t_paquete*, int);
void crearBuffer(t_paquete*);
void iniciarConexionDesdeServidor(int*,int);
int iniciarConexionDesdeClienteHacia(void*);
void deserializarSegun(t_paquete*);
t_paquete* crearPaquete(tipoDeDato);
void agregar_a_paquete(t_paquete*,void*,int);
void eliminarPaquete(t_paquete*);
void liberarConexion(int);
int tamanioEstructura(void*,tipoDeDato);
void* serializarPersona(void*, void*, int);
void* serializarEstructura(void* ,int,tipoDeDato );
void enviarPaquete(t_paquete*, int);
t_paquete* armarPaqueteCon(void*,tipoDeDato);


#endif
