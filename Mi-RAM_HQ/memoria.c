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
    double bytes;
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


void inciarMemoria() {

	memoria_principal = malloc(configRam.tamanioMemoria);

	cant_frames_ppal = configRam.tamanioMemoria / configRam.tamanioPagina;

    log_info(logMemoria, "RAM FRAMES: %s", cant_frames_ppal);

    char* data = asignar_bytes(cant_frames_ppal);

    frames_ocupados_ppal = bitarray_create_with_mode(data, cant_frames_ppal/8, MSB_FIRST);

}


// LA MAYORIA DE LO COMENTADO ES PARA CUANDO IMPLEMENTEMOS LA MEMORIA VIRTUAL

t_pagina* leer_memoria(int frame, int mem) {
    //desplazamiento en memoria
    int desp = frame * configRam.tamanioPagina;
    // mostrar_memoria();
    if(!get_frame(frame, mem))
    {
        return NULL; //frame vacio
    }
    else
    {
        t_pagina* pagina = malloc(sizeof(t_pagina));
        if(mem == MEM_PPAL)
        {
            memcpy(pagina, memoria_principal+desp, configRam.tamanioPagina);
        }

        else if(mem == MEM_VIRT)
        {
            FILE * file = fopen(configRam.pathSwap, "r");
            // printf("reading\n");
            fseek(file, desp, SEEK_SET);
            int sz = fread(pagina, 1, sizeof(t_pagina), file);
            fclose(file);

            // printf("bytes read %d\n",sz);
        }

        return pagina;
    }
}


int insertar_en_memoria(int frame, t_pagina* pagina, int mem) {
    // printf("frame %d -> %d\n",frame, get_frame(frame,mem));
    if(!get_frame(frame,mem)) // no hay nada en el frame
    {
        // printf("empty frame, inserting\n");
        set_frame(frame,mem); //marco el frame como en uso

        int desp = frame * configRam.tamanioPagina;
        if(mem == MEM_PPAL)
        {
            memcpy(memoria_principal+desp, pagina, configRam.tamanioPagina);
        }
        else if(mem == MEM_VIRT){
            FILE * file = fopen(configRam.pathSwap, "r+");
            fseek(file, desp, SEEK_SET);
            int sz = fwrite(pagina, configRam.tamanioPagina , 1, file);
            fclose(file);
            // printf("bytes written %d\n",sz);
        }
        return 1;
    }
    else
    {
        // printf("frame in use\n");
        return 0;
    }
}


bool get_frame(int frame, int mem) {
    if(mem == MEM_PPAL)
        return bitarray_test_bit(frames_ocupados_ppal, frame);

    /*else if(mem == MEM_VIRT)
        return bitarray_test_bit(frames_ocupados_virtual, frame);*/
    else
        log_error(logMemoria, "El frame que se quiere acceder es invalido");
    	exit(1);

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


t_pagina* buscar_pagina(t_info_pagina* info_pagina) {
    t_pagina* pagina = NULL;
    int frame_ppal = info_pagina->frame_m_ppal;
    int frame_virtual = info_pagina->frame_m_virtual;
    if(frame_ppal != FRAME_INVALIDO)
        pagina = leer_memoria(frame_ppal, MEM_PPAL);

        // log_info(logger, "pagina encontrada en memoria principal");
    return pagina;
}






