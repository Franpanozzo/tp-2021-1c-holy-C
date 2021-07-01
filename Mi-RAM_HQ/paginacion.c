#include "paginacion.h"

// LA MAYORIA DE LO COMENTADO ES PARA CUANDO IMPLEMENTEMOS LA MEMORIA VIRTUAL

void* leer_memoria_pag(int frame, int mem) {
    //desplazamiento en memoria
    int desp = frame * configRam.tamanioPagina;
    // mostrar_memoria();

	void* pagina = malloc(configRam.tamanioPagina);
	if(mem == MEM_PPAL)
	{
		log_info(logMemoria,"Se va a leer la pagina que arranca en %d", desp);
		lock(&mutexEscribirMemoria);
		memcpy(pagina, memoria_principal+desp, configRam.tamanioPagina);
		unlock(&mutexEscribirMemoria);
	}

	else if(mem == MEM_VIRT)
	{
		FILE * file = fopen(configRam.pathSwap, "r");
		// printf("reading\n");
		fseek(file, desp, SEEK_SET);
		int sz = fread(pagina, 1, sizeof(configRam.tamanioPagina), file);
		fclose(file);

		// printf("bytes read %d\n",sz);
	}

	return pagina;

}


int insertar_en_memoria_pag(t_info_pagina* info_pagina, void* pagina, int mem, int* aMeter, tipoEstructura tipo, int datoAdicional, int* bytesEscribidos) {
    // printf("frame %d -> %d\n",frame, get_frame(frame,mem));
    if(!get_frame(info_pagina->frame_m_ppal,mem)) // no hay nada en el frame
    {

        int despDesdePagina = configRam.tamanioPagina - info_pagina->bytesDisponibles;

        int desp = info_pagina->frame_m_ppal * configRam.tamanioPagina + despDesdePagina;

        int bytesAEscribir =  info_pagina->bytesDisponibles - *aMeter;

        if(bytesAEscribir < 0)
        {
			bytesAEscribir = info_pagina->bytesDisponibles;
			info_pagina->bytesDisponibles = 0;
	    	set_frame(info_pagina->frame_m_ppal,mem); //marco el frame como en uso porque se escribio toda
		}
		else {
			 bytesAEscribir = *aMeter;
			 info_pagina->bytesDisponibles = info_pagina->bytesDisponibles - *aMeter;
			 }

        agregarEstructAdminTipo(info_pagina, despDesdePagina, bytesAEscribir, tipo, datoAdicional);

        if(mem == MEM_PPAL)
        {
        	lock(&mutexEscribirMemoria);
            memcpy(memoria_principal+desp, pagina, bytesAEscribir);
            unlock(&mutexEscribirMemoria);

        }
        else if(mem == MEM_VIRT){
            FILE * file = fopen(configRam.pathSwap, "r+");
            fseek(file, desp, SEEK_SET);
            int sz = fwrite(pagina, configRam.tamanioPagina , 1, file);
            fclose(file);
            // printf("bytes written %d\n",sz);
        }

        log_info(logMemoria, "Se inserto en RAM: FRAME: %d | DESDE: %d | HASTA: %d | TIPO: %d", info_pagina->frame_m_ppal,
        		despDesdePagina, despDesdePagina + bytesAEscribir - 1, tipo);

        *aMeter -= bytesAEscribir;

        *bytesEscribidos = bytesAEscribir;

		log_info(logMemoria, "Quedan por meter %d", *aMeter);
        return 1;
    }
    else
    {
        // printf("frame in use\n");
        return 0;
    }
}





void sobreescribir_memoria(int frame, void* buffer, int mem, int desplInicial, int bytesAEscribir) {


	int desp = frame * configRam.tamanioPagina + desplInicial;

	if(mem == MEM_PPAL)
	{
		lock(&mutexEscribirMemoria);
		memcpy(memoria_principal+desp, buffer, bytesAEscribir);
		unlock(&mutexEscribirMemoria);

		log_info(logMemoria, "Se sobreescribio en RAM: FRAME: %d | DESDE: %d | HASTA: %d ", frame,
		        		desplInicial, bytesAEscribir + desplInicial -1);
	}
	/*
	else if(mem == MEM_VIRT){
		FILE * file = fopen(configRam.pathSwap, "r+");
		fseek(file, desp, SEEK_SET);
		int sz = fwrite(pagina, configRam.tamanioPagina , 1, file);
		fclose(file);
		// printf("bytes written %d\n",sz);
	} */



}


void agregarEstructAdminTipo(t_info_pagina* info_pagina,int despDesdePagina,int bytesAlojados, tipoEstructura tipo, int datoAdicional){

	t_alojado* nuevo_alojado = malloc(sizeof(t_alojado));
	log_info(logMemoria, "Se va a agregar a la pagina %d , la carga de %d desde %d , y son %d", info_pagina->indice,
			tipo,despDesdePagina,bytesAlojados);
	nuevo_alojado->indice = list_size(info_pagina->estructurasAlojadas);
	nuevo_alojado->desplazamientoInicial = despDesdePagina;
	nuevo_alojado->bytesAlojados = bytesAlojados;
	nuevo_alojado->tipo = tipo;
	nuevo_alojado->datoAdicional = datoAdicional;
	list_add(info_pagina->estructurasAlojadas, nuevo_alojado);
}


bool get_frame(int frame, int mem) {
    if(mem == MEM_PPAL)
    {
    	bool a = bitarray_test_bit(frames_ocupados_ppal, frame);
        return a;
    }

    /*else if(mem == MEM_VIRT)
        return bitarray_test_bit(frames_ocupados_virtual, frame);*/
    else
        log_error(logMemoria, "El frame que se quiere acceder es invalido");
    	exit(1);
}


void set_frame(int frame, int mem) {
    if(mem == MEM_PPAL) {
        bitarray_set_bit(frames_ocupados_ppal, frame);
    }


    /*else if (mem == MEM_VIRT)
        bitarray_set_bit(frames_ocupados_virtual, frame);*/
    else
    {
    	log_error(logMemoria, "El frame que se quiere acceder es invalido");
    	exit(1);
    }
}


void clear_frame(int frame, int mem)
{
    if(mem == MEM_PPAL) {
    	lock(&mutexBitarray);
        bitarray_clean_bit(frames_ocupados_ppal, frame);
        unlock(&mutexBitarray);
    }

    /*else if (mem == MEM_VIRT)
        bitarray_clean_bit(frames_ocupados_virtual, frame); */
    else
       {
       	log_error(logMemoria, "El frame que se quiere acceder es invalido");
       	exit(1);
       }
}


uint32_t buscar_frame_disponible(int mem) {
    int size = 0;
    if(mem == MEM_PPAL)
        size = cant_frames_ppal;
    /*else if(mem == MEM_VIRT)
        size = cant_frames_virtual;*/

    for(uint32_t f = 0; f < size; f++)
    {
        if(!get_frame(f, mem) && frameTotalmenteLibre(f)) {
    	//if(!get_frame(f, mem))
            return f;
        }
    }

    log_info(logMemoria, "No se encontro un frame disponible");
    return FRAME_INVALIDO;
}


int frameTotalmenteLibre(int frame) {

	bool frameEnUso(t_tablaPaginasPatota* tablaPaginas) {

		t_list_iterator* iteradorTablaPaginas = list_iterator_create(tablaPaginas->tablaDePaginas);

		while(list_iterator_has_next(iteradorTablaPaginas))
		{
			t_info_pagina* infoPagina = list_iterator_next(iteradorTablaPaginas);
			if(infoPagina->frame_m_ppal == frame) {
				list_iterator_destroy(iteradorTablaPaginas);
				return 1;
			}
		}

		list_iterator_destroy(iteradorTablaPaginas);

		return 0;
	}

	bool a = list_any_satisfy(tablasPaginasPatotas,(void*) frameEnUso);

	return !a;
}


void* buscar_pagina(t_info_pagina* info_pagina) {
    void* pagina = NULL;
    int frame_ppal = info_pagina->frame_m_ppal;
    //int frame_virtual = info_pagina->frame_m_virtual;
    if(frame_ppal != FRAME_INVALIDO)
        pagina = leer_memoria_pag(frame_ppal, MEM_PPAL);

        // log_info(logger, "pagina encontrada en memoria principal");
    return pagina;
}


t_tarea* asignarProxTareaPag(int idPatota,int idTripulante) {

	t_tablaPaginasPatota* tablaPaginasPatotaActual = buscarTablaDePaginasDePatota(idPatota);

	existenciaDeTablaParaPatota(tablaPaginasPatotaActual);

	lock(&mutexTablaPaginasPatota);
	log_info(logMemoria,"Se encontro la tabla de paginas_ PATOTA: %d - CANT PAGINAS: %d",
					tablaPaginasPatotaActual->idPatota, list_size(tablaPaginasPatotaActual->tablaDePaginas));
	unlock(&mutexTablaPaginasPatota);

	tcb* tcb = obtenerTripulante(tablaPaginasPatotaActual, idTripulante);

	log_info(logMemoria, "Tripulante a asignar proxima tarea: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
			tcb->idTripulante, tcb->estado, tcb->posX, tcb->posY, tcb->proximaAEjecutar, tcb->dlPatota);

	t_tarea* tarea = irABuscarSiguienteTareaPag(tablaPaginasPatotaActual, tcb);


	free(tcb);
	return tarea;
}


void existenciaDeTablaParaPatota(t_tablaPaginasPatota* tablaPaginasPatotaActual) {
	if(tablaPaginasPatotaActual == NULL) {
		log_error(logMemoria, "No existe TCB para ese PCB");
		exit(1);
	}
}


t_tarea* guardarTCBPag(tcb* tcbAGuardar,int idPatota) {

	t_tablaPaginasPatota* tablaPaginasPatotaActual = buscarTablaDePaginasDePatota(idPatota);
	tcbAGuardar->dlPatota = 00;
	tcbAGuardar->proximaAEjecutar = buscarInicioDLTareas(tablaPaginasPatotaActual);
	existenciaDeTablaParaPatota(tablaPaginasPatotaActual);

	int res = asignarPaginasEnTabla((void*) tcbAGuardar, tablaPaginasPatotaActual,TCB);
	if(res == 0) return NULL;

	t_tarea* tarea = irABuscarSiguienteTareaPag(tablaPaginasPatotaActual, tcbAGuardar);

	return tarea;
}


t_list_iterator* iterarHastaIndice(t_list* tablaPaginas, int indicePagina) {

	t_list_iterator* iteradorTablaPaginas = list_iterator_create(tablaPaginas);

	while(indicePagina > 0) {
		list_iterator_next(iteradorTablaPaginas);
		indicePagina--;
	}

	return iteradorTablaPaginas;
}


t_tarea* irABuscarSiguienteTareaPag(t_tablaPaginasPatota* tablaPaginasPatotaActual, tcb* tcbAGuardar) {

	log_info(logMemoria,"Soy el tripu %d voy a buscar tarea a %d", tcbAGuardar->idTripulante, tcbAGuardar->proximaAEjecutar);

	char* tarea = string_new();
	char* aux = malloc(2);
	*(aux+1) = '\0';
	char* proximoALeer = malloc(2);
	*(proximoALeer+1) = '\0';
	void* pagina;
	void* recorredorPagina;
	int indicePagina = (int) floor((double) tcbAGuardar->proximaAEjecutar / 100.0);
	int desplazamiento = tcbAGuardar->proximaAEjecutar % 100;
	t_info_pagina* info_pagina;

	lock(&mutexTablaPaginasPatota);
	log_info(logMemoria, "PAGINAS EN TABLA: %d - ME MUEVO %d PAGINAS",
			list_size(tablaPaginasPatotaActual->tablaDePaginas), indicePagina);
	unlock(&mutexTablaPaginasPatota);

	t_list_iterator* iteradorTablaPaginas = iterarHastaIndice(tablaPaginasPatotaActual->tablaDePaginas, indicePagina);

	// info_pagina = list_get(tablaPaginasPatotaActual->tablaDePaginas, indicePagina);
	*proximoALeer = '0';

	log_info(logMemoria,"Sacando tarea arrancando de indice: %d - desplazamiento: %d ", indicePagina, desplazamiento);

	lock(&mutexTablaPaginasPatota);
	while(list_iterator_has_next(iteradorTablaPaginas))
	{
		info_pagina = list_iterator_next(iteradorTablaPaginas);

		if(tieneEstructuraAlojada(info_pagina->estructurasAlojadas, TAREAS))
		{
		pagina = leer_memoria_pag(info_pagina->frame_m_ppal,MEM_PPAL);
		recorredorPagina = pagina;
		recorredorPagina += desplazamiento;

			while(desplazamiento != configRam.tamanioPagina && *proximoALeer != '|'  && *proximoALeer != '\0')
				{
					memcpy(aux,recorredorPagina,1);
					string_append(&tarea,aux);
					recorredorPagina++;
					desplazamiento++;

					if(desplazamiento != configRam.tamanioPagina)
					{
						memcpy(proximoALeer,recorredorPagina,1);
					}

					/*
					if(recorredorPagina != NULL)
					{
						memcpy(proximoALeer,recorredorPagina,1);
					}*/

					log_info(logMemoria,"Sacando tarea: %s",tarea);
					log_info(logMemoria,"Proximo a leer: %s",proximoALeer);
				}


		log_info(logMemoria,"Asignando al TCB prox a ejecutar - indice: %d - desplazamiento: %d ", info_pagina->indice, desplazamiento);

		tcbAGuardar->proximaAEjecutar = info_pagina->indice * 100 + desplazamiento;

		desplazamiento = 0;
		free(pagina);
		}

		if(*proximoALeer == '|' || *proximoALeer == '\0') break;
	}
	unlock(&mutexTablaPaginasPatota);

	actualizarTripulanteEnMemPag(tablaPaginasPatotaActual, tcbAGuardar);


	if(*tarea == '|') tarea = string_substring_from(tarea,1);

	t_tarea* tareaAMandar = armarTarea(tarea);
	free(aux);
	free(proximoALeer);
	list_iterator_destroy(iteradorTablaPaginas);
	free(tarea);

	return tareaAMandar;
}


int actualizarTripulanteEnMemPag(t_tablaPaginasPatota* tablaPaginasPatotaActual, tcb* tcbAGuardar) {

	t_list* tablaPaginasConTripu = paginasConTripu(tablaPaginasPatotaActual->tablaDePaginas, tcbAGuardar->idTripulante);

	return sobreescribirTripu(tablaPaginasConTripu, tcbAGuardar);
}


int sobreescribirTripu(t_list* paginasConTripu, tcb* tcbAGuardar) {

	int aMeter, relleno, offset = 0;
	void* bufferAMeter = meterEnBuffer(tcbAGuardar, TCB, &aMeter, &relleno);

	int cantPaginasConTripu = list_size(paginasConTripu);

	if(cantPaginasConTripu == 0) {
		log_error(logMemoria, "No hay paginas que contengan al tripulante %d en memoria" , tcbAGuardar->idTripulante);
		free(paginasConTripu);
		return 0;
	}

	log_info(logMemoria, "La cantidad de paginas que contienen al tripulante %d son %d", tcbAGuardar->idTripulante, cantPaginasConTripu);
	int i = 0;

	while(i < cantPaginasConTripu)
	{
		t_info_pagina* info_pagina = list_get(paginasConTripu,i);
		lock(&mutexAlojados);
		t_alojado* alojado = obtenerAlojadoPagina(info_pagina->estructurasAlojadas, tcbAGuardar->idTripulante);
		unlock(&mutexAlojados);

		log_info(logMemoria, "Se va a sobreescrbir el tripulante: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
				tcbAGuardar->idTripulante, tcbAGuardar->estado, tcbAGuardar->posX, tcbAGuardar->posY, tcbAGuardar->proximaAEjecutar, tcbAGuardar->dlPatota);

		sobreescribir_memoria(info_pagina->frame_m_ppal, bufferAMeter + offset, MEM_PPAL, alojado->desplazamientoInicial, alojado->bytesAlojados);
		offset += alojado->bytesAlojados;
		i++;
	}
	list_destroy(paginasConTripu);
	free(bufferAMeter);

	return 1;
}



t_list* paginasConTripu(t_list* tablaDePaginas, uint32_t idTripu) {

	bool tieneTripu(t_info_pagina* info_pagina)
	{
	  return tieneTripulanteAlojado(info_pagina->estructurasAlojadas, idTripu);
	}

	lock(&mutexTablaPaginasPatota);
	t_list* tablaPaginasConTripu = list_filter(tablaDePaginas, (void*) tieneTripu);
	unlock(&mutexTablaPaginasPatota);

	return tablaPaginasConTripu;
}


int actualizarTripulantePag(tcb* tcbAGuardar, int idPatota) {

	t_tablaPaginasPatota* tablaPaginasPatotaActual = buscarTablaDePaginasDePatota(idPatota);

	existenciaDeTablaParaPatota(tablaPaginasPatotaActual);

	t_list* paginasConTripulante = paginasConTripu(tablaPaginasPatotaActual->tablaDePaginas, tcbAGuardar->idTripulante);

	int cantPaginasConTripu = list_size(paginasConTripulante);

		if(cantPaginasConTripu == 0) {
			log_error(logMemoria, "No hay paginas que contengan al tripulante %d en memoria" , tcbAGuardar->idTripulante);
			free(paginasConTripulante);
			return 0;
		}

		log_info(logMemoria, "La cantidad de paginas que contienen al tripulante %d son %d", tcbAGuardar->idTripulante, cantPaginasConTripu);
		int i = 0;

		void* bufferTripu = malloc(21);
		int offset = 0;

		while(i < cantPaginasConTripu)
		{
			t_info_pagina* info_pagina = list_get(paginasConTripulante,i);
			lock(&mutexAlojados);
			t_alojado* alojado = obtenerAlojadoPagina(info_pagina->estructurasAlojadas, tcbAGuardar->idTripulante);
			unlock(&mutexAlojados);


			lock(&mutexTablaPaginasPatota);
			void* pagina = leer_memoria_pag(info_pagina->frame_m_ppal, MEM_PPAL);
			unlock(&mutexTablaPaginasPatota);

			log_info(logMemoria, "SE LEE DEL TRIPU: %d - FRAME: %d | D_INCIAL: %d | BYTES_ALOJ: %d", tcbAGuardar->idTripulante,
					info_pagina->frame_m_ppal, alojado->desplazamientoInicial, alojado->bytesAlojados);

			if(pagina != NULL) {
			memcpy(bufferTripu + offset,pagina + alojado->desplazamientoInicial, alojado->bytesAlojados);
			offset += alojado->bytesAlojados;
			i++;
			free(pagina);
			}
			else {
				log_error(logMemoria, "Se leyo mal la pagina mi bro");
				free(paginasConTripulante);
				return 0;
			}
		}

		cargarDLTripulante(bufferTripu, tcbAGuardar);

		log_info(logMemoria, "Se va a actualizar el tripulante: ID: %d | ESTADO: %c | POS_X: %d | POS_Y: %d | DL_TAREA: %d | DL_PATOTA: %d",
		tcbAGuardar->idTripulante, tcbAGuardar->estado, tcbAGuardar->posX, tcbAGuardar->posY, tcbAGuardar->proximaAEjecutar, tcbAGuardar->dlPatota);

		free(bufferTripu);
		list_destroy(paginasConTripulante);
		return actualizarTripulanteEnMemPag(tablaPaginasPatotaActual, tcbAGuardar);
}


tcb* obtenerTripulante(t_tablaPaginasPatota* tablaPaginasPatotaActual, int idTripulante) {

	t_list* paginasConTripulante = paginasConTripu(tablaPaginasPatotaActual->tablaDePaginas, idTripulante);

		int cantPaginasConTripu = list_size(paginasConTripulante);

			if(cantPaginasConTripu == 0) {
				log_error(logMemoria, "No hay paginas que contengan al tripulante %d en memoria" , idTripulante);
				return 0;
			}

			log_info(logMemoria, "La cantidad de paginas que contienen al tripulante %d son %d", idTripulante, cantPaginasConTripu);
			int i = 0;

			void* bufferTripu = malloc(21);
			int offset = 0;

			while(i < cantPaginasConTripu)
			{
				t_info_pagina* info_pagina = list_get(paginasConTripulante,i);
				lock(&mutexAlojados);
				t_alojado* alojado = obtenerAlojadoPagina(info_pagina->estructurasAlojadas, idTripulante);
				unlock(&mutexAlojados);

				lock(&mutexTablaPaginasPatota);
				void* pagina = leer_memoria_pag(info_pagina->frame_m_ppal, MEM_PPAL);
				unlock(&mutexTablaPaginasPatota);

				log_info(logMemoria, "SE LEE DEL TRIPU: %d - FRAME: %d | D_INCIAL: %d | BYTES_ALOJ: %d", idTripulante,
						info_pagina->frame_m_ppal, alojado->desplazamientoInicial, alojado->bytesAlojados);

				memcpy(bufferTripu + offset,pagina + alojado->desplazamientoInicial, alojado->bytesAlojados);
				offset += alojado->bytesAlojados;

				i++;
				free(pagina);
			}

			tcb* tcb = cargarEnTripulante(bufferTripu);
			free(bufferTripu);
			list_destroy(paginasConTripulante);

			return tcb;
}



int guardarPCBPag(pcb* pcbAGuardar,char* stringTareas) {

	int pcbGuardado, tareasGuardadas;
	t_tablaPaginasPatota* tablaPaginasPatotaActual = malloc(sizeof(t_tablaPaginasPatota));
	tablaPaginasPatotaActual->idPatota = pcbAGuardar->pid;
	tablaPaginasPatotaActual->tablaDePaginas = list_create();
	lock(&mutexTablasPaginas);
	list_add(tablasPaginasPatotas, tablaPaginasPatotaActual);
	unlock(&mutexTablasPaginas);

	log_info(logMemoria, "Se creo la tabla de paginas para la patota: %d", pcbAGuardar->pid);

	pcbAGuardar->dlTareas = estimarDLTareasPag();

	pcbGuardado = asignarPaginasEnTabla((void*) pcbAGuardar, tablaPaginasPatotaActual,PCB);

	log_info(logMemoria, "La direccion logica de las tareas es: %d", pcbAGuardar->dlTareas);

	tareasGuardadas = asignarPaginasEnTabla((void*) stringTareas, tablaPaginasPatotaActual,TAREAS);

	free(pcbAGuardar);
	free(stringTareas);

	return pcbGuardado && tareasGuardadas;
}


t_info_pagina* crearPaginaEnTabla(t_tablaPaginasPatota* tablaPaginasPatotaActual,tipoEstructura tipo) {

	log_info(logMemoria, "Ceando pagina en la tabla de la patota: %d", tablaPaginasPatotaActual->idPatota);

	t_info_pagina* info_pagina = malloc(sizeof(t_info_pagina));
	info_pagina->indice = list_size(tablaPaginasPatotaActual->tablaDePaginas); //Si hay 3 info_pagina el indice va de 0 a 2, el prox indice va a ser 3.  eso ya te lo da el size.
	info_pagina->frame_m_ppal = FRAME_INVALIDO;
	info_pagina->bytesDisponibles = configRam.tamanioPagina;
	info_pagina->estructurasAlojadas = list_create();

	log_info(logMemoria, "Se creo el t_info_pagina de tipo: %d", tipo);

	list_add(tablaPaginasPatotaActual->tablaDePaginas, info_pagina);

	return info_pagina;
}


int asignarPaginasEnTabla(void* aGuardar, t_tablaPaginasPatota* tablaPaginasPatotaActual, tipoEstructura tipo){

	int aMeter = 0;
	int datoAdicional;
	void* bufferAMeter = meterEnBuffer(aGuardar, tipo, &aMeter, &datoAdicional);
	void* copiaBuffer = bufferAMeter;
	int bytesEscritos;

	t_info_pagina* info_pagina;

	while(aMeter > 0)
	{
		lock(&mutexMemoria);
		log_info(logMemoria, "HAY QUE METER %d BYTES",aMeter);

	if(tipo != PCB)
		{
		info_pagina = buscarUltimaPaginaDisponible(tablaPaginasPatotaActual);

			if(info_pagina != NULL)
			{
				log_info(logMemoria, "La pagina en el frame %d tiene lugar y se va a aprovechar", info_pagina->frame_m_ppal);
				insertar_en_memoria_pag(info_pagina, copiaBuffer, MEM_PPAL, &aMeter, tipo, datoAdicional, &bytesEscritos);
			} else
			{
				log_info(logMemoria, "No se encontro una pagina con espacio restante");

				info_pagina = crearPaginaEnTabla(tablaPaginasPatotaActual,tipo);

				info_pagina->frame_m_ppal = buscar_frame_disponible(MEM_PPAL);

				if(info_pagina->frame_m_ppal != FRAME_INVALIDO)
				{
					log_info(logMemoria,"Hay un frame disponible, el %d", info_pagina->frame_m_ppal);
					insertar_en_memoria_pag(info_pagina, copiaBuffer, MEM_PPAL, &aMeter, tipo, datoAdicional, &bytesEscritos);
				}
					else
					{
						log_info(logMemoria, "Memoria principal llena");
						return 0;
					}
			}
		}
				else
				{
					info_pagina = crearPaginaEnTabla(tablaPaginasPatotaActual,tipo);

					info_pagina->frame_m_ppal = buscar_frame_disponible(MEM_PPAL);

					if(info_pagina->frame_m_ppal != FRAME_INVALIDO)
					{
						log_info(logMemoria,"Hay un frame disponible, el %d", info_pagina->frame_m_ppal);
						insertar_en_memoria_pag(info_pagina, copiaBuffer, MEM_PPAL, &aMeter, tipo, datoAdicional, &bytesEscritos);
					}
						else
						{
							log_info(logMemoria, "Memoria principal llena");
							return 0;
						}
				}

	copiaBuffer += bytesEscritos;
	unlock(&mutexMemoria);
	}
	log_info(logMemoria,"Se insertaron todos los bytes en ram");
	free(bufferAMeter);
	return 1;
 }


t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int idPatotaABuscar) {

	bool idIgualA(t_tablaPaginasPatota* tablaPaginaBuscada)
	    {
	        bool a;

	        a = tablaPaginaBuscada->idPatota == idPatotaABuscar;

	        return a;
	    }

		lock(&mutexTablasPaginas);
	    t_tablaPaginasPatota* tablaPaginasBuscada = list_find(tablasPaginasPatotas, (void*)idIgualA);
	    unlock(&mutexTablasPaginas);

	    if(tablaPaginasBuscada == NULL)
	    {
	        log_error(logMemoria,"Tabla de pagina de patota %d no encontrada!! - No existe PCB para ese TCB negro", idPatotaABuscar);
	        exit(1);
	    }

	    return tablaPaginasBuscada;
}


t_info_pagina* buscarUltimaPaginaDisponible(t_tablaPaginasPatota* tablaPaginasPatota) {

    int indiceDeLaUltimaPagina = list_size(tablaPaginasPatota->tablaDePaginas) - 1;

    t_info_pagina* ultimaPaginaCreada = list_get(tablaPaginasPatota->tablaDePaginas, indiceDeLaUltimaPagina);

    if(ultimaPaginaCreada->bytesDisponibles == 0)
    {
        return NULL;
    }

    return ultimaPaginaCreada;
}


uint32_t estimarDLTareasPag(){

	int nroPagina;
    //HAY QUE HACER OTRA CUENTA
	int desplazamiento;

	if(configRam.tamanioPagina > 8)
	{
	    nroPagina = 0;
	    desplazamiento = 8;
	}
	else
	{
	    nroPagina = (int) (floor(8/configRam.tamanioPagina));
	    desplazamiento = 8 % configRam.tamanioPagina;
	}

	log_info(logMemoria,"DL TAREA: %d \n", nroPagina * 100 + desplazamiento);

	return nroPagina * 100 + desplazamiento;

}


uint32_t buscarInicioDLTareas(t_tablaPaginasPatota* tablaPaginasPatota) {


    bool buscarDLTarea(t_info_pagina* info_pagina) {

    	//lock(&mutexAlojados);
    	bool a = tieneEstructuraAlojada(info_pagina->estructurasAlojadas,TAREAS);
    	//unlock(&mutexAlojados);

    	return a;
    }

    lock(&mutexTablaPaginasPatota);
    t_info_pagina* paginaConTarea = list_find(tablaPaginasPatota->tablaDePaginas, (void*) buscarDLTarea);
    unlock(&mutexTablaPaginasPatota);

    bool tieneTarea(t_alojado* estructuraAlojada) {
    	return estructuraAlojada->tipo == TAREAS;
    }

    lock(&mutexAlojados);
    t_alojado* alojadoConTarea = list_find(paginaConTarea->estructurasAlojadas, (void*) tieneTarea);
    unlock(&mutexAlojados);


    //Retornar un struct DL de las tareas que tiene el indice de la pagina y el desplazamiento en esta
    //Se guarda en algun lado cuanto pesa el string

    lock(&mutexAlojados);
    int a = paginaConTarea->indice * 100 + alojadoConTarea->desplazamientoInicial;
    unlock(&mutexAlojados);

    return a;
}


bool tieneEstructuraAlojada(t_list* listaAlojados, tipoEstructura tipo) {

	bool contieneTipo(t_alojado* estructuraAlojada){
	    	return estructuraAlojada->tipo == tipo;
	    }

	//lock(&mutexAlojados);
	t_alojado* alojadoConTarea = list_find(listaAlojados, (void*) contieneTipo);
	//unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}


bool tieneTripulanteAlojado(t_list* listaAlojados, int idTCB) {

	lock(&mutexAlojados);
	t_alojado* alojadoConTarea = obtenerAlojadoPagina(listaAlojados, idTCB);
	unlock(&mutexAlojados);

	return alojadoConTarea != NULL;
}


t_alojado* obtenerAlojadoPagina(t_list* listaAlojados, int idTCB) {

	bool contieneTipo(t_alojado* estructuraAlojada){
	    	return estructuraAlojada->tipo == TCB && estructuraAlojada->datoAdicional == idTCB;
	    }

	//lock(&mutexAlojados);
	t_alojado* alojadoConTarea = list_find(listaAlojados, (void*) contieneTipo);
	//unlock(&mutexAlojados);

	return alojadoConTarea;
}


void expulsarTripulantePag(int idTripulante,int idPatota) {

	t_tablaPaginasPatota* tablaPatota = buscarTablaDePaginasDePatota(idPatota);
	existenciaDeTablaParaPatota(tablaPatota);
	t_list* paginasTripu = paginasConTripu(tablaPatota->tablaDePaginas,idTripulante);

	log_info(logMemoria, "Se va a eliminar el tripulante %d de la patota %d",idTripulante, tablaPatota->idPatota);

	if(paginasTripu != NULL)
	{
		t_list_iterator* iteradorPaginas = list_iterator_create(paginasTripu);

		while(list_iterator_has_next(iteradorPaginas))
		{
			t_info_pagina* paginaActual = list_iterator_next(iteradorPaginas);
			lock(&mutexAlojados);
			t_alojado* tripuAlojado = obtenerAlojadoPagina(paginaActual->estructurasAlojadas, idTripulante);
			unlock(&mutexAlojados);

			lock(&mutexAlojados);
			paginaActual->bytesDisponibles += tripuAlojado->bytesAlojados;
			unlock(&mutexAlojados);

			lock(&mutexAlojados);
			log_info(logMemoria,"Se va a sacar de la lista de alojados de cant %d el tripu %d",
					list_size(paginaActual->estructurasAlojadas), tripuAlojado->indice);
			unlock(&mutexAlojados);

			if(paginaActual->estructurasAlojadas == NULL) {
				log_error(logMemoria,"No hay na aca");
			}

			bool tripuConID(t_alojado* alojado) {
				return alojado->tipo == TCB && alojado->datoAdicional == idTripulante;
			}

			lock(&mutexAlojados);
			list_remove_by_condition(paginaActual->estructurasAlojadas,(void*) tripuConID);
			unlock(&mutexAlojados);

			//reducirIndiceAlojados(paginaActual->estructurasAlojadas);

			log_info(logMemoria, "Se elimino el dato del TRIPULANTE %d en la PAGINA: %d",idTripulante, paginaActual->indice);

			lock(&mutexTablaPaginasPatota);
			log_info(logMemoria,"PAGINA: %d - BYTES DISPONIBLES: %d",paginaActual->indice,paginaActual->bytesDisponibles);
			unlock(&mutexTablaPaginasPatota);


			free(tripuAlojado);

			lock(&mutexTablaPaginasPatota);
			if(paginaActual->bytesDisponibles == 32)
			{
				unlock(&mutexTablaPaginasPatota);
				log_info(logMemoria,"Pagina %d vacia se procede a liberar el frame y borrarla de tabla",paginaActual->indice);
				clear_frame(paginaActual->frame_m_ppal, MEM_PPAL);

				bool paginaConID(t_alojado* pagina)
				{
					return pagina->indice == paginaActual->indice;
				}

				lock(&mutexTablaPaginasPatota);
				list_remove_by_condition(tablaPatota->tablaDePaginas, (void*) paginaConID);
				unlock(&mutexTablaPaginasPatota);

				lock(&mutexAlojados);
				list_destroy(paginaActual->estructurasAlojadas);
				unlock(&mutexAlojados);

				free(paginaActual);
			}
			else {
				unlock(&mutexTablaPaginasPatota);
			}
		}

		list_destroy(paginasTripu);
		chequearUltimoTripulante(tablaPatota);
		list_iterator_destroy(iteradorPaginas);
	}
}


void chequearUltimoTripulante(t_tablaPaginasPatota* tablaPatota) {

	if(!tieneTripulantesPag(tablaPatota)) {

		log_info(logMemoria,"La patota %d no tiene mas tripulantes. Se procede a borrarla de memoria", tablaPatota->idPatota);

		void borrarProceso(t_info_pagina* info_pagina)
		{

			void borrarAlojados(t_alojado* alojado)
			{
				free(alojado);
			}

			lock(&mutexAlojados);
			list_destroy_and_destroy_elements(info_pagina->estructurasAlojadas, (void*) borrarAlojados);
			unlock(&mutexAlojados);
			log_info(logMemoria,"SE LIBERA  EL FRAME: %d",info_pagina->frame_m_ppal);
			clear_frame(info_pagina->frame_m_ppal, MEM_PPAL);
			free(info_pagina);
		}

		bool tablaConID(t_tablaPaginasPatota* tablaPatota2) {
			return tablaPatota2->idPatota == tablaPatota->idPatota;
		}

		lock(&mutexTablasPaginas);
		list_remove_by_condition(tablasPaginasPatotas, (void*) tablaConID);
		unlock(&mutexTablasPaginas);

		lock(&mutexTablaPaginasPatota);
		list_destroy_and_destroy_elements(tablaPatota->tablaDePaginas, (void*) borrarProceso);
		unlock(&mutexTablaPaginasPatota);

		free(tablaPatota);
	}
}


bool tieneTripulantesPag(t_tablaPaginasPatota* tablaPatota) {

	bool tienePaginaTripulante(t_info_pagina* info_pagina)
	{
		return tieneEstructuraAlojada(info_pagina->estructurasAlojadas, TCB);
	}

	lock(&mutexTablaPaginasPatota);
	bool a = list_any_satisfy(tablaPatota->tablaDePaginas, (void*) tienePaginaTripulante);
	unlock(&mutexTablaPaginasPatota);

	return a;
}


void dumpPag() {

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

	for(int i=0; i< cant_frames_ppal; i++) {

		t_tablaPaginasPatota* tablaPaginasPatota = patotaConFrame(i);

		if(tablaPaginasPatota != NULL)
		{
			t_info_pagina* info_pagina = paginaConFrame(i,tablaPaginasPatota);

			char* dumpMarco = string_from_format("Marco:%d    Estado:Ocupado    Proceso:%d    Pagina:%d \n",
					i, tablaPaginasPatota->idPatota, info_pagina->indice);

			txt_write_in_file(archivoDump, dumpMarco);
			free(dumpMarco);
		}
		else
		{
			char* dumpMarco = string_from_format("Marco:%d    Estado:Libre      Proceso:-    Pagina:- \n",i);
			txt_write_in_file(archivoDump, dumpMarco);
			free(dumpMarco);
		}
	}

	txt_write_in_file(archivoDump, "--------------------------------------------------------------------------\n");

	txt_close_file(archivoDump);
	free(rutaAbsoluta);
	free(dump);
}


t_tablaPaginasPatota* patotaConFrame(int frame) {
		bool frameEnUso(t_tablaPaginasPatota* tablaPaginas) {

			t_list_iterator* iteradorTablaPaginas = list_iterator_create(tablaPaginas->tablaDePaginas);

			while(list_iterator_has_next(iteradorTablaPaginas))
			{
				t_info_pagina* infoPagina = list_iterator_next(iteradorTablaPaginas);
				if(infoPagina->frame_m_ppal == frame) return 1;
			}

			list_iterator_destroy(iteradorTablaPaginas);

			return 0;
		}

		lock(&mutexTablasPaginas);
		t_tablaPaginasPatota* tablaPaginasPatota = list_find(tablasPaginasPatotas,(void*) frameEnUso);
		unlock(&mutexTablasPaginas);

		return tablaPaginasPatota;
}


t_info_pagina* paginaConFrame(int frame,t_tablaPaginasPatota* tablaPaginasPatota) {

	t_list_iterator* iteradorTablaPaginas = list_iterator_create(tablaPaginasPatota->tablaDePaginas);

	while(list_iterator_has_next(iteradorTablaPaginas))
	{
		t_info_pagina* infoPagina = list_iterator_next(iteradorTablaPaginas);
		if(infoPagina->frame_m_ppal == frame) return infoPagina;
	}

	log_error(logMemoria,"No hay pagina con ese frame (?");
	return NULL;
}








