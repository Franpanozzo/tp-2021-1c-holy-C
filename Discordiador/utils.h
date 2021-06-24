#ifndef UTILS_H_
#define UTILS_H_

	#include "discordiador.h"
	#include "variables.h"


	void iniciarTareasIO();
	void iniciarListas();
	void iniciarMutex();
	void cargarConfiguracion();
	char * pathLog();

	void modificarTripulanteBlocked(int);
	int leerTripulanteBlocked();
	void modificarPlanificacion(int);
	int leerPlanificacion();
	void crearConfig();

	t_patota* asignarDatosAPatota(char*);

	void esperarTerminarTripulante(t_tripulante*);
	void avisarTerminoPlanificacion(t_tripulante*);

	void casoBlocked();

	void pasarDeLista(t_tripulante*);
	void meterEnLista(void* , t_lista*);
	void* sacarDeLista(t_lista*);
	void ponerEnSabotaje(void*);
	void elegirTripulanteSabotaje();
	void ponerEnReady(void*);
	uint32_t distancia(t_coordenadas, t_coordenadas);


	char* deserializarString (t_paquete*);
	void actualizarEstadoEnRAM(t_tripulante*);
	void pasarAlistaSabotaje(t_lista*);
	int enviarA(puertoEIP*, void*, tipoDeDato);
	bool tripulanteDeMenorId(void*, void*);
	t_tripulante* elTripuMasCerca(t_coordenadas);

	void recibirPrimerTareaDeMiRAM(t_tripulante*);
	void recibirProximaTareaDeMiRAM(t_tripulante*);
	void recibirTareaDeMiRAM(int, t_tripulante*);
	bool confirmacion(int);

	int esIO(char*);
	uint32_t calculoCiclosExec(t_tripulante*);
	void desplazarse(t_tripulante*, t_coordenadas);
	uint32_t diferencia(uint32_t, uint32_t);

//	void imprimirTripulante(void*);
	void listarLista(t_lista* lista);
	void listarTripulantes();
	char* traducirEstado(t_estado);

	void cambiarDeEstado(t_tripulante*, t_estado);
	bool esElBuscado(void*);
	bool tieneDistintoEstado(void*);
	bool tieneIgualEstado(void*);
	int totalTripulantes();

	void eliminarTripulante();
	void eliminarPatota(t_patota*);
	void liberarTripulante(t_tripulante*);

#endif
