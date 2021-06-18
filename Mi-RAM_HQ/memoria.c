#include "memoria.h"


void cargar_configuracion() {
	t_config* config = config_create("/home/utnso/tp-2021-1c-holy-C/Mi-RAM_HQ/mi_ram_hq.config"); //Leo el archivo de configuracion

	if (config == NULL) {
		perror("Archivo de configuracion de RAM no encontrado");
		return;
	}

	configRam.tamanioMemoria = config_get_int_value(config, "TAMANIO_MEMORIA");
	configRam.esquemaMemoria = config_get_string_value(config, "ESQUEMA_MEMORIA");
	configRam.tamanioPagina = config_get_int_value(config, "TAMANIO_PAGINA");
	configRam.tamanioSwap = config_get_int_value(config, 	"TAMANIO_SWAP");
	configRam.pathSwap = config_get_string_value(config, 	"PATH_SWAP");
	configRam.algoritmoReemplazo = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
	configRam.criterioSeeleccion = config_get_string_value(config, "CRITERIO_SELECCION");
	configRam.puerto = config_get_int_value(config, "PUERTO");
}


char* asignar_bytes(int cant_frames)
{
    char* buf;
    int bytes;
    if(cant_frames < 8)
        bytes = 1;
    else
    {
        double c = (double) cant_frames;
        bytes = (int) ceil(c/8.0);
    }
    log_info(logMemoria,"BYTES: %d\n", bytes);
    buf = malloc(bytes);
    memset(buf,0,bytes);
    return buf;
}


void iniciarMemoria() {

	tablasPaginasPatotas = list_create();

    pthread_mutex_init(&mutexMemoria, NULL);

	log_info(logMemoria, "TAMANIO RAM: %d", configRam.tamanioMemoria);

	memoria_principal = malloc(configRam.tamanioMemoria);

	cant_frames_ppal = configRam.tamanioMemoria / configRam.tamanioPagina;

    log_info(logMemoria, "RAM FRAMES: %d", cant_frames_ppal);

    char* data = asignar_bytes(cant_frames_ppal);

    frames_ocupados_ppal = bitarray_create_with_mode(data, cant_frames_ppal/8, MSB_FIRST);

	log_info(logMemoria,"El esquema usado es %s", configRam.esquemaMemoria);

}


// LA MAYORIA DE LO COMENTADO ES PARA CUANDO IMPLEMENTEMOS LA MEMORIA VIRTUAL

void* leer_memoria(int frame, int mem) {
    //desplazamiento en memoria
    int desp = frame * configRam.tamanioPagina;
    // mostrar_memoria();
    if(!get_frame(frame, mem))
    {
        return NULL; //frame vacio
    }
    else
    {
    	void* pagina = malloc(sizeof(configRam.tamanioPagina));
        if(mem == MEM_PPAL)
        {
            memcpy(pagina, memoria_principal+desp, configRam.tamanioPagina);

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
}


int insertar_en_memoria(t_info_pagina* info_pagina, void* pagina, int mem, int* aMeter, tipoEstructura tipo) {
    // printf("frame %d -> %d\n",frame, get_frame(frame,mem));
    if(!get_frame(info_pagina->frame_m_ppal,mem)) // no hay nada en el frame
    {

        int despDesdePagina = configRam.tamanioPagina - info_pagina->bytesDisponibles;

        int desp = info_pagina->frame_m_ppal * configRam.tamanioPagina + despDesdePagina;

        int bytesAEscribir =  info_pagina->bytesDisponibles - *aMeter;

        if(bytesAEscribir < 0) {
			bytesAEscribir = info_pagina->bytesDisponibles;
			info_pagina->bytesDisponibles = 0;
	        set_frame(info_pagina->frame_m_ppal,mem); //marco el frame como en uso porque se escribio toda
								}
        	else {
				 bytesAEscribir = *aMeter;
				 info_pagina->bytesDisponibles = info_pagina->bytesDisponibles - *aMeter;
				 }

        agregarEstructAdminTipo(info_pagina, despDesdePagina, despDesdePagina + bytesAEscribir, tipo);

        if(mem == MEM_PPAL)
        {
            memcpy(memoria_principal+desp, pagina, bytesAEscribir);
        }
        else if(mem == MEM_VIRT){
            FILE * file = fopen(configRam.pathSwap, "r+");
            fseek(file, desp, SEEK_SET);
            int sz = fwrite(pagina, configRam.tamanioPagina , 1, file);
            fclose(file);
            // printf("bytes written %d\n",sz);
        }

        log_info(logMemoria, "Se inserto en RAM: FRAME: %d | DESDE: %d | HASTA: %d | TIPO: %d", info_pagina->frame_m_ppal,
        		despDesdePagina, despDesdePagina + bytesAEscribir, tipo);

        *aMeter -= bytesAEscribir;
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
		memcpy(memoria_principal+desp, buffer, bytesAEscribir);
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


void agregarEstructAdminTipo(t_info_pagina* info_pagina,int despDesdePagina,int bytesAlojados, tipoEstructura tipo){

	t_alojado* nuevo_alojado = malloc(sizeof(t_alojado));
	nuevo_alojado->desplazamientoInicial = despDesdePagina;
	nuevo_alojado->bytesAlojados = bytesAlojados;
	nuevo_alojado->tipo = tipo;
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


uint32_t buscar_frame_disponible(int mem) {
    int size = 0;
    if(mem == MEM_PPAL)
        size = cant_frames_ppal;
    /*else if(mem == MEM_VIRT)
        size = cant_frames_virtual;*/

    for(uint32_t f = 0; f < size; f++)
        if(!get_frame(f, mem))
            return f;

    return FRAME_INVALIDO;
}


void* buscar_pagina(t_info_pagina* info_pagina) {
    void* pagina = NULL;
    int frame_ppal = info_pagina->frame_m_ppal;
    //int frame_virtual = info_pagina->frame_m_virtual;
    if(frame_ppal != FRAME_INVALIDO)
        pagina = leer_memoria(frame_ppal, MEM_PPAL);

        // log_info(logger, "pagina encontrada en memoria principal");
    return pagina;
}


t_tarea* guardarTCBPag(tcb* tcbAGuardar,int idPatota) {

	t_tablaPaginasPatota* tablaPaginasPatotaActual = buscarTablaDePaginasDePatota(idPatota);
	t_DL* dlPatota;
	dlPatota->nroPagina = 0;
	dlPatota->desplazamiento = 0;
	tcbAGuardar->dlPatota = dlPatota;
	tcbAGuardar->proximaAEjecutar = buscarInicioDLTareas(tablaPaginasPatotaActual);
	if(tablaPaginasPatotaActual != NULL)
	{
		log_info(logMemoria,"Se encontro la tabla de paginas de la patota correspondiente");
	}
	else
	{
		log_error(logMemoria,"No existe PCB para ese TCB negro");
		exit(1);
	}

	int res = asignarPaginasEnTabla((void*) tcbAGuardar, tablaPaginasPatotaActual,TCB);
	if(res == 0) return NULL;

	t_tarea* tarea = irABuscarSiguienteTarea(tablaPaginasPatotaActual, tcbAGuardar);

	return tarea;
}


t_tarea* irABuscarSiguienteTarea(t_tablaPaginasPatota* tablaPaginasPatotaActual, tcb* tcbAGuardar) {

	char* tarea;
	char* proximoALeer;
	char* aux = malloc(sizeof(1)) + 1;
	*(aux+1) = '/0';
	int noPrimeraVez = 0;
	void* pagina;
	int indicePagina = tcbAGuardar->proximaAEjecutar->nroPagina;
	int desplazamiento = tcbAGuardar->proximaAEjecutar->desplazamiento;


	t_info_pagina* info_pagina = list_get(tablaPaginasPatotaActual->tablaDePaginas, indicePagina);

	while(tieneEstructuraAlojada(info_pagina->estructurasAlojadas, TAREAS)) {

		pagina = leerMemoria(info_pagina->frame_m_ppal,MEM_PPAL);
		pagina += desplazamiento;

		while(pagina != NULL && *proximoALeer != '|'  && *proximoALeer != '/0') {
			memcpy(aux,pagina,1);
			string_append(&tarea,aux);
			pagina++;
			proximoALeer = (char*) pagina;
		    desplazamiento++;
		}

		tcbAGuardar->proximaAEjecutar->nroPagina = indicePagina;
		tcbAGuardar->proximaAEjecutar->desplazamiento = desplazamiento;
		indicePagina++;
		desplazamiento = 0;

		if(noPrimeraVez)
		{
			t_info_pagina* info_pagina = list_get(tablaPaginasPatotaActual->tablaDePaginas, indicePagina);
		}

		if(!noPrimeraVez) noPrimeraVez =1;

	}

	actualizarTripulante(tablaPaginasPatotaActual, tcbAGuardar);

	t_tarea* tareaAMandar = armarTarea(tarea);

	return tareaAMandar;
}


void actualizarTripulante(t_tablaPaginasPatota* tablaPaginasPatotaActual, tcb* tcbAGuardar) {

	bool tieneTripu(t_info_pagina* info_pagina)
	{
	  return tieneTripulanteAlojado(info_pagina->estructurasAlojadas, tcbAGuardar->idTripulante);
	}

	int aMeter, offset = 0;
	t_list* tablaPaginasConTripu = list_filter(tablaPaginasPatotaActual->tablaDePaginas, (void*) tieneTripu);

	void* bufferAMeter = meterEnBuffer(tcbAGuardar, TCB, aMeter);


	int cantPaginasConTripu = list_size(tablaPaginasConTripu);
	int i = 0;

	while(i < cantPaginasConTripu)
	{
		t_info_pagina* info_pagina = list_get(tablaPaginasConTripu,i);
		t_alojado* alojado = obtenerAlojadoPagina(info_pagina->estructurasAlojadas, tcbAGuardar->idTripulante);

		sobreescribir_memoria(info_pagina->frame_m_ppal, bufferAMeter + offset, MEM_PPAL, alojado->desplazamientoInicial, alojado->bytesAlojados);
		offset += alojado->bytesAlojados;
		i++;
	}
}


int guardarPCBPag(pcb* pcbAGuardar,char* stringTareas) {

	int pcbGuardado, tareasGuardadas;
	t_tablaPaginasPatota* tablaPaginasPatotaActual = malloc(sizeof(t_tablaPaginasPatota));
	tablaPaginasPatotaActual->idPatota = pcbAGuardar->pid;
	tablaPaginasPatotaActual->tablaDePaginas = list_create();
	list_add(tablasPaginasPatotas, tablaPaginasPatotaActual);

	log_info(logMemoria, "Se creo la tabla de paginas para la patota: %d", pcbAGuardar->pid);

	log_info(logMemoria, "La direccion logica de las tareas es: %d", pcbAGuardar->dlTareas);


	pcbGuardado = asignarPaginasEnTabla((void*) pcbAGuardar, tablaPaginasPatotaActual,PCB);

	tareasGuardadas = asignarPaginasEnTabla((void*) stringTareas, tablaPaginasPatotaActual,TAREAS);

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
	void* bufferAMeter = meterEnBuffer(aGuardar, tipo, &aMeter);

	t_info_pagina* info_pagina;

	while(aMeter > 0)
	{
		lock(mutexMemoria);
		log_info(logMemoria, "HAY QUE METER %d BYTES",aMeter);

	if(tipo != PCB)
		{
		info_pagina = buscarUltimaPaginaDisponible(tablaPaginasPatotaActual);

			if(info_pagina != NULL)
			{
				log_info(logMemoria, "La pagina en el frame %d tiene lugar y se va a aprovechar", info_pagina->frame_m_ppal);
				insertar_en_memoria(info_pagina, bufferAMeter, MEM_PPAL, &aMeter, tipo);
			} else
			{
				log_info(logMemoria, "No se encontro una pagina con espacio restante");

				info_pagina = crearPaginaEnTabla(tablaPaginasPatotaActual,tipo);

				info_pagina->frame_m_ppal = buscar_frame_disponible(MEM_PPAL);

				if(info_pagina->frame_m_ppal != FRAME_INVALIDO)
				{
					log_info(logMemoria,"Hay un frame disponible, el %d", info_pagina->frame_m_ppal);
					insertar_en_memoria(info_pagina, bufferAMeter, MEM_PPAL, &aMeter, tipo);
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
						insertar_en_memoria(info_pagina, bufferAMeter, MEM_PPAL, &aMeter, tipo);
					}
						else
						{
							log_info(logMemoria, "Memoria principal llena");
							return 0;
						}
				}

	unlock(mutexMemoria);
	}
	log_info(logMemoria,"Se insertaron todos los bytes en ram");
	return 1;
 }


t_tablaPaginasPatota* buscarTablaDePaginasDePatota(int idPatotaABuscar) {

	bool idIgualA(t_tablaPaginasPatota* tablaPaginaBuscada)
	    {
	        bool a;

	        a = tablaPaginaBuscada->idPatota == idPatotaABuscar;

	        return a;
	    }

	    t_tablaPaginasPatota* tablaPaginasBuscada = list_find(tablasPaginasPatotas, (void*)idIgualA);

	    if(tablaPaginasBuscada == NULL)
	    {
	        log_error(logMemoria,"Tabla de pagina de patota %d no encontrada!! \n", idPatotaABuscar);
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


uint32_t estimarDLTareas(){
	if(configRam.tamanioPagina > 8)
	{
	    return 0;
	}
	else
	{
	    return (int) (floor(8/configRam.tamanioPagina) + 1);
	}
}


t_DL* buscarInicioDLTareas(t_tablaPaginasPatota* tablaPaginasPatota) {

    bool buscarDLTarea(t_info_pagina* info_pagina) {

    	return tieneEstructuraAlojada(info_pagina->estructurasAlojadas,TAREAS);
    }

    t_info_pagina* paginaConTarea = list_find(tablaPaginasPatota->tablaDePaginas, (void*) buscarDLTarea);

    bool tieneTarea(t_alojado* estructuraAlojada) {
    	return estructuraAlojada->tipo == TAREA;
    }

    t_alojado* alojadoConTarea = list_find(paginaConTarea->estructurasAlojadas, (void*) tieneTarea);

    t_DL* direccionLogica;
    direccionLogica->nroPagina = paginaConTarea->indice;
    direccionLogica->desplazamiento = alojadoConTarea->desplazamientoInicial;
    //Retornar un struct DL de las tareas que tiene el indice de la pagina y el desplazamiento en esta
    //Se guarda en algun lado cuanto pesa el string

    return direccionLogica;
}


bool tieneEstructuraAlojada(t_list* listaAlojados, tipoEstructura tipo) {

	bool contieneTipo(t_alojado* estructuraAlojada){
	    	return estructuraAlojada->tipo == tipo;
	    }

	t_alojado* alojadoConTarea = list_find(listaAlojados, (void*) contieneTipo);

	return alojadoConTarea != NULL;
}


bool tieneTripulanteAlojado(t_list* listaAlojados, int idTCB) {

	t_alojado* alojadoConTarea = obtenerAlojadoPagina(listaAlojados, idTCB);

	return alojadoConTarea != NULL;
}


t_alojado* obtenerAlojadoPagina(t_list* listaAlojados, int idTCB) {

	bool contieneTipo(t_alojado* estructuraAlojada){
	    	return estructuraAlojada->tipo == TCB && estructuraAlojada->datoAdicional == idTCB;
	    }

	t_alojado* alojadoConTarea = list_find(listaAlojados, (void*) contieneTipo);

	return alojadoConTarea;
}



void* meterEnBuffer(void* aGuardar, tipoEstructura tipo, int* aMeter) {

	void* buffer;
	int offset = 0;

	switch(tipo)
	{
		case PCB:
		{
			pcb* pcbAGuardar = (pcb*) aGuardar;
			*aMeter = 8;
			buffer = malloc(*aMeter);
			pcbAGuardar->dlTareas = estimarDLTareas();
			memcpy(buffer, &(pcbAGuardar->pid), sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(pcbAGuardar->dlTareas), sizeof(uint32_t));
			break;
		}
		case TCB:
		{
			tcb* tcbAGuardar = (tcb*) aGuardar;
			*aMeter = 21;
			buffer = malloc(*aMeter);
			memcpy(buffer + offset, &(tcbAGuardar->idTripulante),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->dlPatota),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->estado),sizeof(t_estado));
			offset += sizeof(t_estado);
			memcpy(buffer + offset, &(tcbAGuardar->posX),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			memcpy(buffer + offset, &(tcbAGuardar->posY),sizeof(uint32_t));
			offset += sizeof(uint32_t);
			break;
		}
		case TAREAS:
		{
			char* tareas = (char*) aGuardar;
			*aMeter = strlen(tareas) + 1;
			buffer = malloc(*aMeter);
			memcpy(buffer, tareas, strlen(tareas) + 1);
			break;
		}
		default:
			log_error(logMemoria,"No puedo guardar eso en una pagina negro");
			exit(1);
	}

	return buffer;
}


int guardarPCB(pcb* pcbAGuardar, char* stringTareas) {

	if(strcmp(configRam.esquemaMemoria,"PAGINADO") == 0)
	{
		return guardarPCBPag(pcbAGuardar, stringTareas);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTADO") == 0)
	{
		return guardarPCBSeg(pcbAGuardar, stringTareas);
	}
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}

int guardarTCB(tcb* tcbAGuardar, int idPatota) {
	if(strcmp(configRam.esquemaMemoria,"PAGINADO") == 0)
	{
		return guardarTCBPag(tcbAGuardar, idPatota);
	}
	if(strcmp(configRam.esquemaMemoria,"SEGMENTADO") == 0)
	{
		return guardarTCBSeg(tcbAGuardar, idPatota);
	}
	log_info(logMemoria,"Esquema de memoria no valido: %s", configRam.esquemaMemoria);
	exit(1);
}


int guardarPCBSeg(pcb* pcbAGuardar, char* stringTareas){
	return 0;
}


int guardarTCBSeg(tcb* tcbAGuardar, int idPatota){
	return 0;
}











