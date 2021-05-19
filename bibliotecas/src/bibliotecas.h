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
		uint32_t posX;
		uint32_t posY;
	}t_coordenadas;

	typedef struct{
		char* IP;
		int puerto;
	} puertoEIP;

	typedef enum{
		PERSONA,
		PATOTA,
		TRIPULANTE,
		STRING
	} tipoDeDato;

	typedef struct {
	    char* cod_op;
	    uint32_t parametro;
	    uint32_t posX;
	    uint32_t posY;
	    uint32_t tiempo;
	} t_tarea;

	typedef enum{
		NEW,
		READY,
		EXEC,
		BLOCKED
	}t_estado;

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

	typedef struct{
		uint32_t ID;
		t_estado estado;
		uint32_t posX;
		uint32_t posY;
		char* instruccionAejecutar;
		sem_t semaforo;
	} t_tripulante;

	typedef struct{
		uint32_t ID;
		uint32_t tamanioTareas;
		char* tareas;
	}t_patota;


	void lock(pthread_mutex_t);

	void unlock(pthread_mutex_t);

	int iniciarConexionDesdeServidor(int);

	int iniciarConexionDesdeClienteHacia(void*);

	void liberarConexion(int);

	t_log* iniciarLogger(char*, char*, int);

	t_paquete* recibirPaquete(int);

	void* serializarString(void*, void*);

	void* serializarTripulante(void*, void*, int);


	/**
	* @NAME: deserializarPersona
	* @DESC: recibe un t_buffer* para deserializarlo a un t_persona* y operar
	* con ese TAD, la funcion libera la memoria alocada
	*/
	void deserializarPersona(t_buffer*);


	/**
	* @NAME: serializarPaquete
	* @DESC: recibe un t_paquete* y el tamaño que ocupa en Bytes para serializarlo
	* y que pueda ser enviado por socket
	*/
	void* serializarPaquete(t_paquete*, int);


	/**
	* @NAME: crearBuffer
	* @DESC: recibe un t_paquete* y crea el TAD t_buffer* contenido en el t_paquete*
	*/
	void crearBuffer(t_paquete*);

	/**
	* @NAME: crearPaquete
	* @DESC: recibe un tipoDeDato y retorna el TAD t_paquete* con el tipoDeDato asignado y
	* todas los TAD que contiene ya alocados
	*/
	t_paquete* crearPaquete(tipoDeDato);



	//void agregarAPaquete(t_paquete*,void*,int);

	/**
	* @NAME: eliminarPaquete
	* @DESC: recibe un t_paquete* y libera toda la memoria asignada,
	* inclueyendo la de los TAD que contiene
	*/
	void eliminarPaquete(t_paquete*);

	/**
	* @NAME: tamanioEstructura
	* @DESC: recibe un *TAD cualquiera y su codigo de operacion
	* y te retorna el tamaño
	*/
	int tamanioEstructura(void*,tipoDeDato);

	/**
	* @NAME: serializarPersona
	* @DESC: recibe un stream de datos, un *esrctura cualquiera y un int offset
	* y te retorna el stream cargado con la informacion cargada como t_persona
	* y te retorna el stream
	*/
	void* serializarPersona(void*, void*, int);

	/**
	* @NAME: serializarPersona
	* @DESC: recibe un stream de datos, un *estructura cualquiera el tamanio de la
	* estructuroa y el tipoDeDato de la estructura para serializar segun corresponda
	* y te devuelve el stream de datos para empaquetar
	*/
	void* serializarEstructura(void* ,int,tipoDeDato );

	/**
	* @NAME: armarPaqueteCon
	* @DESC: recibe una *estructura cualquiera y el tipoDeDato y te retorna el t_paquete*
	* serializado listo para enviar
	*/
	t_paquete* armarPaqueteCon(void*,tipoDeDato);


	/**
	* @NAME: enviarPaquete
	* @DESC: recibe un t_paquete* y el socket de la conexion a donde enviar el paquete
	*/
	void enviarPaquete(t_paquete*, int);

	/**
	* @NAME: serializarString
	* @DESC: recibe una tarea patota y la serializa como string
	*/
	void* serializarPatota(void*,void*,int);


#endif
