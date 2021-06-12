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
