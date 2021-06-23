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


t_tablaSegmentosPatota* buscarTablaDePaginasDePatota(int idPatotaABuscar) {

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
	t_tablaSegmentosPatota* tablaPaginasPatotaActual = malloc(sizeof(t_tablaSegmentosPatota));
	tablaPaginasPatotaActual->idPatota = pcbAGuardar->pid;
	tablaPaginasPatotaActual->tablaDeSegmentos = list_create();

	log_info(logMemoria, "Se creo la tabla de segmentos para la patota: %d", pcbAGuardar->pid);

	pcbGuardado = asignarSegmentosEnTabla((void*) pcbAGuardar, tablaPaginasPatotaActual,PCB);

	log_info(logMemoria, "La direccion logica de las tareas es: %d", pcbAGuardar->dlTareas);

	tareasGuardadas = asignarSegmentosEnTabla((void*) stringTareas, tablaPaginasPatotaActual,TAREAS);

	free(pcbAGuardar);

	return pcbGuardado && tareasGuardadas;

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


int buscarSegmentoSegunAjuste(int aMeter) {

	void* recorredorMemoria = memoria_principal;
	int bytesLibres;
	int desplazamiento = 0;


	t_list* lugaresLibres = list_create();

	//desplazamiento <= configRam.tamanioMemoria
	while(recorredorMemoria != NULL) //recorredorMemoria != NULL  PORQUE NO ANDA?? ANTES PASABA LO MISMO
	{
		if(*recorredorMemoria == '$')
		{
			while(*recorredorMemoria == '$')
			{
				bytesLibres++;
				recorredorMemoria++;
			}
			int loQueSobra = bytesLibres - aMeter;

			if(loQueSobra >= 0) {

				if(strcmp(configRam.criterioSeeleccion,"BF") == 0)
				{
					t_lugarLibre* lugarLibre = malloc(sizeof(t_lugarLibre));
					lugarLibre->inicio = desplazamiento;
					lugarLibre->bytesSobrantes = loQueSobra;
					list_add(lugaresLibres,lugarLibre);
				}
				else if(strcmp(configRam.criterioSeeleccion,"FF") == 0)
				{
					return desplazamiento;
				}
				else
				{
					log_error(logMemoria, "El criterio de seleccion de particion libre no es valido");
					exit(1);
				}
			}

			desplazamiento += bytesLibres;
			bytesLibres = 0;
		}
		else
		{
			recorredorMemoria++;
		}
		desplazamiento++;
	}

	if(list_size(lugaresLibres) == 0) return -1;

	t_lugarLibre* sobraMenos(t_lugarLibre* lugarLibre1, t_lugarLibre* lugarLibre2) {

		if(lugarLibre1->bytesSobrantes < lugarLibre2->bytesSobrantes) return lugarLibre1;
		else return lugarLibre2;
	}

	t_lugarLibre* lugarLibre = list_get_minimum(lugaresLibres, (void*) sobraMenos);

	return lugarLibre->inicio;
}


void insertar_en_memoria_seg(t_info_segmento* info_segmento, void* bufferAMeter) {

	lock(mutexEscribirMemoria);
	memcpy(memoria_principal + info_segmento->deslazamientoInicial, bufferAMeter, info_segmento->bytesAlojados);
	unlock(mutexEscribirMemoria);
}











