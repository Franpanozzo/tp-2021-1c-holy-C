#ifndef UTILS_H_
#define UTILS_H_

  #include "discordiador.h"
	#include "variables.h"


	void iniciarTareasIO();
	void iniciarListas();
	void iniciarMutex();
	void cargarConfiguracion();
	char * pathLog();

	t_estado leerEstado(t_tripulante*);
	void modificarEstado(t_tripulante*, t_estado);
	void modificarTripulanteBlocked(int);
	int leerTripulanteBlocked();
	void modificarPlanificacion(int);
	int leerPlanificacion();
	void crearConfig();

	t_patota* asignarDatosAPatota(char*);

	void esperarTerminarTripulante(t_tripulante*);
	void avisarTerminoPlanificacion(t_tripulante*);

	void elegirTripulanteAbloquear();

	void comunicarseConTripulantes(t_lista*, void(*closure)(void*));

	void pasarDeLista(t_tripulante*);
	bool patotaSinTripulantes(uint32_t, uint32_t);
	void eliminiarPatota(uint32_t idPatota);
	void meterEnLista(t_tripulante* , t_lista*);
	void* sacarDeLista(t_lista*);
	void ponerEnSabotaje(t_tripulante*);
	void elegirTripulanteSabotaje();
	void ponerEnReady(t_tripulante*);
	uint32_t distancia(t_coordenadas, t_coordenadas);

	void sacarDeColas(t_tripulante*);

	char* deserializarString (t_paquete*);
	void actualizarEstadoEnRAM(t_tripulante*);
	void pasarAlistaSabotaje(t_lista*);
	int enviarA(puertoEIP*, void*, tipoDeDato);
	bool tripulanteDeMenorId(t_tripulante*, t_tripulante*);
	t_tripulante* elTripuMasCerca(t_coordenadas);

	void mandarTCBaMiRAM(t_tripulante*);
	void recibirProximaTareaDeMiRAM(t_tripulante*);
	void recibirTareaDeMiRAM(int, t_tripulante*);
	bool confirmacion(int);

	int esIO(char*);
	uint32_t calculoCiclosExec(t_tripulante*);
	void desplazarse(t_tripulante*, t_coordenadas);
	uint32_t diferencia(uint32_t, uint32_t);

	void listarTripulantes();
	char* traducirEstado(t_estado);

	void cambiarDeEstado(t_tripulante*, t_estado);
	bool tieneDistintoEstado(t_tripulante*);
	bool tieneIgualEstado(t_tripulante*);
	int totalTripulantes();

	void eliminarTripulante(int);
	void eliminarPatota(t_patota*);
	void liberarTripulante(t_tripulante*);

#endif
