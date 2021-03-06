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
#include <commons/collections/list.h>
#include <stdbool.h>


	typedef struct{
		uint32_t posX;
		uint32_t posY;
	}t_coordenadas;

	typedef struct{
		char* IP;
		int puerto;
	} puertoEIP;

	typedef enum{

		PATOTA,
		TRIPULANTE,
		EXPULSAR,
		ESTADO_TRIPULANTE,
		SIGUIENTE_TAREA,
		TAREA, // id tripulante con t_tarea
		DESPLAZAMIENTO,// id tripulante con t_coordenada x 2
		INICIO_TAREA, // id tripulante  y nombre tarea solamente (char)
		FIN_TAREA, // id tripulante y nombre tarea solamente (char)
		ID_SABOTAJE, // id tripulante
		RESOLUCION_SABOTAJE,// id tripulante
		COORDENADAS_SABOTAJE,
		STRING,
		OBTENER_BITACORA

	} tipoDeDato;

	typedef struct {

	    char* nombreTarea;
	    uint32_t parametro;
	    t_coordenadas coordenadas;
	    uint32_t tiempo;

	} t_tarea;

	typedef enum{
		NEW,
		READY,
		EXEC,
		BLOCKED,
		SABOTAJE,
		EXIT
	}t_estado;

	typedef struct {
		uint32_t size; // Tamaño del payload
		void* stream; // Payload
	} t_buffer;

	typedef struct {
		tipoDeDato codigoOperacion;
		t_buffer* buffer;
	} t_paquete;

	typedef struct{
		uint32_t idPatota;
		uint32_t idTripulante;
		t_estado estado;
		t_coordenadas coordenadas;
		t_tarea* instruccionAejecutar;
		sem_t semaforoInicio;
		sem_t semaforoFin;
		pthread_mutex_t mutexEstado;
	} t_tripulante;

	typedef struct{
		uint32_t ID;
		uint32_t tamanioTareas;
		char* tareas;
	}t_patota;

	typedef struct{
		t_tripulante* tripulanteSabotaje;
		t_coordenadas coordenadas;
		int haySabotaje;
		int tiempo;
		sem_t semaforoIniciarSabotaje;
		sem_t semaforoCorrerSabotaje;
		sem_t semaforoTerminoTripulante;
		sem_t semaforoTerminoSabotaje;
	} t_sabotaje;

	typedef struct{
		uint32_t idTripulante;
		t_coordenadas inicio;
		t_coordenadas fin;
	} t_desplazamiento;

	typedef struct{
		uint32_t idTripulante;
		char* nombreTarea;
		int numero;
	} t_avisoTarea;

	typedef struct{
		pthread_mutex_t mutex;
		t_list* elementos;
	}t_lista;

	void lock(pthread_mutex_t*);

	void unlock(pthread_mutex_t*);

	int iniciarConexionDesdeServidor(int);

	int iniciarConexionDesdeClienteHacia(void*);

	void liberarConexion(int);

	t_log* iniciarLogger(char*, char*, int);

	t_paquete* recibirPaquete(int);

	void* serializarTarea(void*, void*, int);

	void* serializarTripulante(void*, void*, int);
	void* serializarSolicitudSiguienteTarea(void*, void* , int);
	void* serializarString(void*, void*, int);
	void* serializarSolicitudSiguienteTarea(void*, void*, int);
	void* serializarDesplazamiento(void*, void*, int);
	void* serializarAvisoTarea(void*, void*, int);
	void* serializarAvisoSabotaje(void*, void*, int);
	t_tarea* deserializarTarea(void*);
	void liberarDoblesPunterosAChar(char** );
	t_coordenadas deserializarCoordenadas(void*);
	void* serializarCoordenadas(void*, void*, int);



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
	* @NAME: serializarEstructura
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
