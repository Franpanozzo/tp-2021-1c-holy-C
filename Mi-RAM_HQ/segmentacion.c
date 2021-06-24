#include "segmentacion.h"


t_tarea* guardarTCBSeg(tcb* tcbAGuardar, int idPatota) {

	t_tablaSegmentosPatota* tablaSegmentosPatotaActual = buscarTablaDeSegmentosDePatota(idPatota);
	tcbAGuardar->dlPatota = 0;
	tcbAGuardar->proximaAEjecutar = 1;

	log_info(logMemoria,"Se encontro la tabla de segmentos -- PATOTA: %d - CANT SEGMENTOS: %d",
				tablaSegmentosPatotaActual->idPatota, list_size(tablaSegmentosPatotaActual->tablaDeSegmentos));

	int res = asignarSegmentosEnTabla((void*) tcbAGuardar, tablaSegmentosPatotaActual,TCB);
	if(res == 0) return NULL;

	t_tarea* tarea = irABuscarSiguienteTarea(tablaSegmentosPatotaActual, tcbAGuardar);

	return tarea;
}


t_tablaSegmentosPatota* buscarTablaDeSegmentosDePatota(int idPatotaABuscar) {

	bool idIgualA(t_tablaSegmentosPatota* tablaSegmentosBuscada)
	    {
	        bool a;

	        a = tablaSegmentosBuscada->idPatota == idPatotaABuscar;

	        return a;
	    }
		t_tablaSegmentosPatota* tablaSegmentosBuscada = list_find(tablasSegmentosPatotas, (void*)idIgualA);

	    if(tablaSegmentosBuscada == NULL)
	    {
	        log_error(logMemoria,"Tabla de Segmentos de patota %d no encontrada!! - No existe PCB para ese TCB \n", idPatotaABuscar);
			exit(1);
	    }
	    return tablaSegmentosBuscada;
}


int guardarPCBSeg(pcb* pcbAGuardar, char* stringTareas) {

	int pcbGuardado, tareasGuardadas;
	t_tablaSegmentosPatota* tablaSegmentosPatotaActual = malloc(sizeof(t_tablaSegmentosPatota));
	tablaSegmentosPatotaActual->idPatota = pcbAGuardar->pid;
	tablaSegmentosPatotaActual->tablaDeSegmentos = list_create();
	list_add(tablasSegmentosPatotas,tablaSegmentosPatotaActual);

	log_info(logMemoria, "Se creo la tabla de segmentos para la patota: %d", pcbAGuardar->pid);

	pcbAGuardar->dlTareas = 1;
	pcbGuardado = asignarSegmentosEnTabla((void*) pcbAGuardar, tablaSegmentosPatotaActual,PCB);

	log_info(logMemoria, "La direccion logica de las tareas es: %d", pcbAGuardar->dlTareas);

	tareasGuardadas = asignarSegmentosEnTabla((void*) stringTareas, tablaSegmentosPatotaActual,TAREAS);

	free(pcbAGuardar);

	return pcbGuardado && tareasGuardadas;
}


void expulsarTripulanteSeg(int idTripu, int idPatota) {

	t_tablaSegmentosPatota* tablaSegmentosBuscada = buscarTablaDeSegmentosDePatota(idPatota);

	bool segConTripu(t_info_segmento* info_segmento)
	{
		return info_segmento->tipo == TCB && info_segmento->datoAdicional == idTripu;
	}

	t_info_segmento* info_segmento = list_find(tablaSegmentosBuscada, (void*) segConTripu);

	bool despuesQueSeg(t_lugarLibre* lugarLibre)
	{
		int finSegmento = info_segmento->deslazamientoInicial + info_segmento->bytesAlojados;
		return lugarLibre->inicio == finSegmento + 1;
	}

	bool antesQueSeg(t_lugarLibre* lugarLibre)
	{
		int fin = lugarLibre->inicio + lugarLibre->bytesAlojados;
		return fin + 1 == info_segmento->deslazamientoInicial;
	}

	t_lugarLibre* lugarLibreAntes = list_find(lugaresLibres, (void*) antesQueSeg);
	t_lugarLibre* lugarLibreDespues = list_find(lugaresLibres, (void*) despuesQueSeg);

	if(lugarLibreAntes != NULL)
	{
		if(lugarLibreDespues != NULL)
		{
			lugarLibreAntes->bytesAlojados += info_segmento->bytesAlojados + lugarLibreDespues->bytesAlojados;
			borrarLugarLibre(lugarLibreDespues);
		}
		else
		{
			lugarLibreAntes->bytesAlojados += info_segmento->bytesAlojados;
		}
	}
	else if(lugarLibreDespues != NULL)
	{
		lugarLibreDespues->inicio = info_segmento->deslazamientoInicial;
	}
	else // NO TIENE UN LUGAR ANTES NI UNO DESPPUES, CREO UN NUEVO LUGAR LIBRE
	{
		t_lugarLibre* lugarLibre = malloc(sizeof(t_lugarLibre));
		lugarLibre->inicio = info_segmento->deslazamientoInicial;
		lugarLibre->bytesAlojados = info_segmento->bytesAlojados;
		list_add(lugaresLibres,lugarLibre);
	}


	bool mismoIndice(t_info_segmento* info_segmento2)
	{
		return info_segmento2->indice == info_segmento->indice;
	}

	list_remove_by_condition(tablaSegmentosBuscada->tablaDeSegmentos, (void*) mismoIndice);

	chequearUltimoTripulanteSeg(tablaSegmentosBuscada);
}


void chequearUltimoTripulanteSeg(t_tablaSegmentosPatota* tablaSegmentosPatota) {
	if(!tieneTripulantesSeg()) {

		void borrarSegmento(t_info_segmento* info_segmento)
		{
			free(info_segmento);
		}

		bool tablaConID(t_tablaSegmentosPatota* tablaPatota2)
		{
			return tablaPatota2->idPatota == tablaSegmentosPatota->idPatota;
		}

		list_remove_by_condition(tablasPaginasPatotas, (void*) tablaConID);
		list_destroy_and_destroy_elements(tablaSegmentosPatota->tablaDeSegmentos, (void*) borrarSegmento);
	}
}



int asignarSegmentosEnTabla(void* aGuardar, t_tablaSegmentosPatota* tablaSegmentosPatotaActual, tipoEstructura tipo){

	int aMeter = 0;
	int datoAdicional;
	void* bufferAMeter = meterEnBuffer(aGuardar, tipo, &aMeter, &datoAdicional);
	t_info_segmento* info_segmento;
	info_segmento->tipo = tipo;

	int inicioSegmentoLibre = buscarSegmentoSegunAjuste(aMeter);

	if(inicioSegmentoLibre == -1) {
		log_info(logMemoria, "Memoria principal llena");
		return 0;
	}

	info_segmento = crearSegmentoEnTabla(tablaSegmentosPatotaActual,tipo);
	info_segmento->datoAdicional = datoAdicional;
	info_segmento->deslazamientoInicial = inicioSegmentoLibre;
	info_segmento->bytesAlojados = aMeter;

	insertar_en_memoria_seg(info_segmento,bufferAMeter);

	return 1;
}


t_info_segmento* crearSegmentoEnTabla(t_tablaSegmentosPatota tablaSegmentosPatotaActual, tipoEstructura tipo) {

	t_info_segmento* info_segmento = malloc(sizeof(t_info_segmento));
	info_segmento->indice = list_size(tablaSegmentosPatotaActual->tablaDeSegmentos);
	info_segmento->tipo = tipo;

	log_info(logMemoria, "Se creo el t_info_segmento de tipo: %d", tipo);

	list_add(tablaSegmentosPatotaActual->tablaDeSegmentos,info_segmento);

	return info_segmento;
}


void borrarLugarLibre(t_lugarLibre* lugarAEliminar) {
	bool empiezaEn(t_lugarLibre* lugarLibre)
	{
		return lugarLibre->inicio == lugarAEliminar->inicio;
	}
	list_remove_by_condition(lugaresLibres, (void*) empiezaEn);
	free(lugarAEliminar);
}


int buscarSegmentoSegunAjuste(int aMeter) {

	int inicio;

	bool entraAMeter(t_lugarLibre* lugarLibre)
	{
		return (lugarLibre - aMeter) > 0;
	}
	t_list* lugaresQueEntra = list_filter(lugaresLibres, (void*) entraAMeter);


	if(strcmp(configRam.criterioSeeleccion,"BF") == 0)
	{

		t_lugarLibre* sobraMenos(t_lugarLibre* lugarLibre1, t_lugarLibre* lugarLibre2)
		{
			if(lugarLibre1->bytesAlojados < lugarLibre2->bytesAlojados) return lugarLibre1;
			else return lugarLibre2;
		}

		t_lugarLibre* lugarLibreOptimo = list_get_minimum(lugaresQueEntra, (void*) sobraMenos);
		inicio = lugarLibreOptimo->inicio;
		lugarLibreOptimo->inicio += aMeter;
		lugarLibreOptimo->bytesAlojados -= aMeter;

		if(lugarLibreOptimo->bytesAlojados == 0)
		{
			borrarLugarLibre(lugarLibreOptimo);
		}
		return inicio;
	}


	else if(strcmp(configRam.criterioSeeleccion,"FF") == 0)
	{
		t_lugarLibre* empiezaAntes(t_lugarLibre* lugarLibre1, t_lugarLibre* lugarLibre2)
		{
			if(lugarLibre1->inicio < lugarLibre2->inicio) return lugarLibre1;
			else return lugarLibre2;
		}

		t_lugarLibre* primerLugarLibre = list_get_minimum(lugaresQueEntra, (void*) empiezaAntes);

		return primerLugarLibre->inicio;
	}
	else
	{
		log_error(logMemoria, "El criterio de seleccion de particion libre no es valido");
		exit(1);
	}
}


void insertar_en_memoria_seg(t_info_segmento* info_segmento, void* bufferAMeter) {

	lock(mutexEscribirMemoria);
	memcpy(memoria_principal + info_segmento->deslazamientoInicial, bufferAMeter, info_segmento->bytesAlojados);
	unlock(mutexEscribirMemoria);
}









