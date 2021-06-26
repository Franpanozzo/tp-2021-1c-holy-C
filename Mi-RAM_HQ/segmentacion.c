#include "segmentacion.h"


t_tarea* guardarTCBSeg(tcb* tcbAGuardar, int idPatota) {

	t_tablaSegmentosPatota* tablaSegmentosPatotaActual = buscarTablaDeSegmentosDePatota(idPatota);
	tcbAGuardar->dlPatota = 0;
	tcbAGuardar->proximaAEjecutar = 0; //desplazamiento dentro del segmento de las tareas (el cual es siempre el segundo segmento).

	existenciaDeTablaSegParaPatota(tablaSegmentosPatotaActual);

	int res = asignarSegmentosEnTabla((void*) tcbAGuardar, tablaSegmentosPatotaActual,TCB);
	if(res == 0) return NULL;

	t_tarea* tarea = irABuscarSiguienteTareaSeg(tablaSegmentosPatotaActual, tcbAGuardar);

	return tarea;
}


t_tarea* asignarProxTareaSeg(int idPatota, int idTripu){

	t_tablaSegmentosPatota* tablaSegmentosPatotaActual = buscarTablaDeSegmentosDePatota(idPatota);

	existenciaDeTablaSegParaPatota(tablaSegmentosPatotaActual);

	t_info_segmento* t_segmentoTripulante = buscarSegmentoTripulante(idTripu,tablaSegmentosPatotaActual);

	void* segmentoConTripu = leer_memoria_seg(t_segmentoTripulante);

	tcb* tcb = cargarEnTripulante(segmentoConTripu);

	log_info(logMemoria, "Tripulante a asignar proxima tarea: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
				tcb->idTripulante, tcb->estado, tcb->posX, tcb->posY, tcb->proximaAEjecutar, tcb->dlPatota);

	t_tarea* tarea = irABuscarSiguienteTareaSeg(tablaSegmentosPatotaActual, tcb);

	if(strcmp(tarea->nombreTarea,"TAREA_NULA") == 0) {
		expulsarTripulanteSeg(idTripu, idPatota);
	}

	free(tcb);
	free(segmentoConTripu);
	return tarea;
}


t_tarea* irABuscarSiguienteTareaSeg(t_tablaSegmentosPatota* tablaSegmentosPatotaActual,tcb* tcbAGuardar) {

	t_info_segmento* t_segmentoConTarea = list_get(tablaSegmentosPatotaActual->tablaDeSegmentos,1);

	void* segmentoConTarea = leer_memoria_seg(t_segmentoConTarea);
	void* recorredorSegmento = segmentoConTarea;
	recorredorSegmento += tcbAGuardar->proximaAEjecutar;
	char* aux = malloc(2);
	*(aux+1) = '\0';
	char* tarea = string_new();

	memcpy(aux,recorredorSegmento,1);

	while(*aux != '|'  && *aux != '\0')
	{
		string_append(&tarea,aux);
		recorredorSegmento++;
		tcbAGuardar->proximaAEjecutar++;
		memcpy(aux,recorredorSegmento,1);

		log_info(logMemoria,"Sacando tarea: %s",tarea);
		log_info(logMemoria,"Proximo a leer: %s",aux);
	}
	tcbAGuardar->proximaAEjecutar++;

	log_info(logMemoria,"TCB prox a ejecutar quedo en: %d", tcbAGuardar->proximaAEjecutar);

	t_info_segmento* t_segmentoTripulante = buscarSegmentoTripulante(tcbAGuardar->idTripulante, tablaSegmentosPatotaActual);

	actualizarTripulanteEnMemSeg(tcbAGuardar, t_segmentoTripulante);

	if(*tarea == '|') tarea = string_substring_from(tarea,1);

	t_tarea* tareaAMandar = armarTarea(tarea);
	free(aux);
	free(segmentoConTarea);
	free(tarea);

	return tareaAMandar;
}


int actualizarTripulanteSeg(tcb* tcbAGuardar, int idPatota) {

	t_tablaSegmentosPatota* tablaConTripulante = buscarTablaDeSegmentosDePatota(idPatota);

	existenciaDeTablaSegParaPatota(tablaConTripulante);

	t_info_segmento* t_segmentoTripulante = buscarSegmentoTripulante(tcbAGuardar->idTripulante,tablaConTripulante);

	void* segmentoConTripu = leer_memoria_seg(t_segmentoTripulante);

	cargarDLTripulante(segmentoConTripu, tcbAGuardar);

	actualizarTripulanteEnMemSeg(tcbAGuardar, t_segmentoTripulante);

	free(segmentoConTripu);
	free(tcbAGuardar);

	return 1;
}


void actualizarTripulanteEnMemSeg(tcb* tcbAGuardar, t_info_segmento* t_segmentoTripulante) {

	log_info(logMemoria, "Se va a actualizar el tripulante: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
			tcbAGuardar->idTripulante, tcbAGuardar->estado, tcbAGuardar->posX, tcbAGuardar->posY, tcbAGuardar->proximaAEjecutar, tcbAGuardar->dlPatota);

	int aMeter, relleno;
	void* segmentoActualizado = meterEnBuffer(tcbAGuardar, TCB, &aMeter, &relleno);

	insertar_en_memoria_seg(t_segmentoTripulante, segmentoActualizado);
	free(segmentoActualizado);
}


void existenciaDeTablaSegParaPatota(t_tablaSegmentosPatota* tablaSegmentosPatotaActual) {
	if(tablaSegmentosPatotaActual == NULL) {
		log_error(logMemoria, "No existe TCB para ese PCB");
		exit(1);
	}
	log_info(logMemoria,"Se encontro la tabla de segmentos -- PATOTA: %d - CANT SEGMENTOS: %d",
			tablaSegmentosPatotaActual->idPatota, list_size(tablaSegmentosPatotaActual->tablaDeSegmentos));
}


t_info_segmento* buscarSegmentoTripulante(int idTripulante, t_tablaSegmentosPatota* tablaConTripulante) {

	bool segConTripu(t_info_segmento* info_segmento)
	{
		return info_segmento->tipo == TCB && info_segmento->datoAdicional == idTripulante;
	}

	return list_find(tablaConTripulante->tablaDeSegmentos, (void*) segConTripu);
}


t_tablaSegmentosPatota* buscarTablaDeSegmentosDePatota(int idPatotaABuscar) {

	bool idIgualA(t_tablaSegmentosPatota* tablaSegmentosBuscada)
	    {
	        bool a;

	        a = tablaSegmentosBuscada->idPatota == idPatotaABuscar;

	        return a;
	    }
		log_info(logMemoria, "Patotas con tabla totales: %d",list_size(tablasSegmentosPatotas));

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

	log_info(logMemoria, "Se creo la tabla de segmentos para la patota: %d - ahora hay %d patotas", pcbAGuardar->pid, list_size(tablasSegmentosPatotas));

	pcbAGuardar->dlTareas = 1;
	pcbGuardado = asignarSegmentosEnTabla((void*) pcbAGuardar, tablaSegmentosPatotaActual,PCB);

	log_info(logMemoria, "La direccion logica de las tareas es: %d", pcbAGuardar->dlTareas);

	tareasGuardadas = asignarSegmentosEnTabla((void*) stringTareas, tablaSegmentosPatotaActual,TAREAS);

	free(pcbAGuardar);
	free(stringTareas);

	return pcbGuardado && tareasGuardadas;
}


void expulsarTripulanteSeg(int idTripu, int idPatota) {

	t_tablaSegmentosPatota* tablaSegmentosBuscada = buscarTablaDeSegmentosDePatota(idPatota);

	t_info_segmento* info_segmento = buscarSegmentoTripulante(idTripu, tablaSegmentosBuscada);

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
	else // NO TIENE UN LUGAR ANTES NI UNO DESPUES, CREO UN NUEVO LUGAR LIBRE
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
	free(info_segmento);

	chequearUltimoTripulanteSeg(tablaSegmentosBuscada);
}


void chequearUltimoTripulanteSeg(t_tablaSegmentosPatota* tablaSegmentosPatota) {
	if(!tieneTripulantesSeg(tablaSegmentosPatota)) {

		log_info(logMemoria,"La patota %d no tiene mas tripulantes. Se procede a borrarla de memoria", tablaSegmentosPatota->idPatota);

		void borrarSegmento(t_info_segmento* info_segmento)
		{
			free(info_segmento);
		}

		bool tablaConID(t_tablaSegmentosPatota* tablaPatota2)
		{
			return tablaPatota2->idPatota == tablaSegmentosPatota->idPatota;
		}

		log_info(logMemoria,"La patota tiene %d segmentos", list_size(tablaSegmentosPatota->tablaDeSegmentos));

		list_remove_by_condition(tablasSegmentosPatotas, (void*) tablaConID);
		list_destroy_and_destroy_elements(tablaSegmentosPatota->tablaDeSegmentos, (void*) borrarSegmento);

		free(tablaSegmentosPatota);
	}
}


bool tieneTripulantesSeg(t_tablaSegmentosPatota* tablaSegmentosPatota) {

	bool tieneSegmentoTripulante(t_info_segmento* info_segmento)
	{
		return info_segmento->tipo == TCB;
	}

	return list_any_satisfy(tablaSegmentosPatota->tablaDeSegmentos, (void*) tieneSegmentoTripulante);
}



int asignarSegmentosEnTabla(void* aGuardar, t_tablaSegmentosPatota* tablaSegmentosPatotaActual, tipoEstructura tipo){

	int aMeter = 0;
	int datoAdicional;
	void* bufferAMeter = meterEnBuffer(aGuardar, tipo, &aMeter, &datoAdicional);
	t_info_segmento* info_segmento;

	int inicioSegmentoLibre = buscarSegmentoSegunAjuste(aMeter);

	if(inicioSegmentoLibre == -1) {
		log_info(logMemoria, "Memoria principal llena");
		return 0;
	}

	info_segmento = crearSegmentoEnTabla(tablaSegmentosPatotaActual,tipo);
	info_segmento->datoAdicional = datoAdicional;
	info_segmento->deslazamientoInicial = inicioSegmentoLibre;
	info_segmento->bytesAlojados = aMeter;
	info_segmento->tipo = tipo;


	insertar_en_memoria_seg(info_segmento,bufferAMeter);
	free(bufferAMeter);

	return 1;
}


t_info_segmento* crearSegmentoEnTabla(t_tablaSegmentosPatota* tablaSegmentosPatotaActual, tipoEstructura tipo) {

	t_info_segmento* info_segmento = malloc(sizeof(t_info_segmento));
	info_segmento->indice = list_size(tablaSegmentosPatotaActual->tablaDeSegmentos);
	info_segmento->tipo = tipo;

	log_info(logMemoria, "Se creo el t_info_segmento %d de tipo: %d", info_segmento->indice, tipo);

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
		return (lugarLibre->bytesAlojados - aMeter) > 0;
	}
	t_list* lugaresQueEntra = list_filter(lugaresLibres, (void*) entraAMeter);

	if(list_size(lugaresQueEntra) == 0)
	{
		//ACA HAY QUE COMPACTAR
		log_info(logMemoria, "No hay lugares libres en donde entren %d bytes, se procede a compactar", aMeter);
		list_destroy(lugaresQueEntra);
		return -1;
	}


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

		list_destroy(lugaresQueEntra);
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

		list_destroy(lugaresQueEntra);
		return primerLugarLibre->inicio;
	}
	else
	{
		log_error(logMemoria, "El criterio de seleccion de particion libre no es valido");
		list_destroy(lugaresQueEntra);
		exit(1);
	}
}


void insertar_en_memoria_seg(t_info_segmento* info_segmento, void* bufferAMeter) {

	lock(&mutexEscribirMemoria);
	memcpy(memoria_principal + info_segmento->deslazamientoInicial, bufferAMeter, info_segmento->bytesAlojados);
	unlock(&mutexEscribirMemoria);

	log_info(logMemoria, "Se escribio en RAM: SEGMENTO--> INICIO: %d | HASTA: %d ",
			info_segmento->deslazamientoInicial, info_segmento->deslazamientoInicial + info_segmento->bytesAlojados -1);
}


void* leer_memoria_seg(t_info_segmento* info_Segmento) {

	void* segmento = malloc(info_Segmento->bytesAlojados);

	log_info(logMemoria, "Se va a leer el segmento que arranca en %d", info_Segmento->deslazamientoInicial);
	lock(&mutexEscribirMemoria);
	memcpy(segmento, memoria_principal + info_Segmento->deslazamientoInicial, info_Segmento->bytesAlojados);
	unlock(&mutexEscribirMemoria);

	return segmento;
}



void dumpSeg() {

	char* nombreArchivo = temporal_get_string_time("DUMP_%y%m%d%H%M%S%MS.dmp");
	char* rutaAbsoluta = string_from_format("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/Dump/%s",nombreArchivo);

	FILE* archivoDump = txt_open_for_append(rutaAbsoluta);

	if(archivoDump == NULL){
		log_error(logMemoria, "No se pudo abrir el archivo correctamente");
		exit(1);
	}

	char* fechaYHora = temporal_get_string_time("%d/%m/%y %H:%M:%S \n");
	char* dump = string_from_format("DUMP: %s",fechaYHora);

	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n");
	txt_write_in_file(archivoDump, dump);

	void imprimirTabla(t_tablaSegmentosPatota* tablaSegPatota) {

		imprimirDatosSegmento(tablaSegPatota,archivoDump);

	}

	list_iterate(tablasSegmentosPatotas,(void*) imprimirTabla);


	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n");

	txt_close_file(archivoDump);
	free(rutaAbsoluta);
	free(fechaYHora);
	free(dump);
	free(nombreArchivo);
}


void imprimirDatosSegmento(t_tablaSegmentosPatota* tablaSegPatota, FILE* archivoDump) {

	void imprimirSegmento(t_info_segmento* info_segmento) {

		  //char* posEnHexa = mem_hexstring(memoria_principal, info_segmento->deslazamientoInicial);

		char* dumpMarco = string_from_format("Proceso:%d   Segmento:%d	 Inicio:0x%X	 Tam:%db \n",
				tablaSegPatota->idPatota, info_segmento->indice, info_segmento->deslazamientoInicial, info_segmento->bytesAlojados);

		txt_write_in_file(archivoDump, dumpMarco);
		free(dumpMarco);

	}
	list_iterate(tablaSegPatota->tablaDeSegmentos, (void*) imprimirSegmento);
}
















