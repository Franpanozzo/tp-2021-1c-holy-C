#include "sabotajes.h"

/*
void buscarSabotaje(){

	if(haySabotajeCantBloquesEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en la cantidad de bloques en el super bloque");
		//arreglarSabotajeCantBloquesEnSuperBloque();
	}
	else if(haySabotajeBitmapEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en el bitmap del super bloque");
		//arreglarSabotajeBitmapEnSuperBloque();
	}

	tarea* arrayFilesConsumibles[3] = {oxigeno, comida, basura};
	for(int i=0; i<3; i++){

		if(haySabotajeBloquesEnFile(arrayFilesConsumibles[i])){
			log_info(logImongo,"Se detecto un sabotaje en los bloques del file %s", "oxigeno");
			//arreglarSabotajeBloquesEnFile(arrayFilesConsumibles[i]);
		}
		else if(haySabotajeCantBloquesEnFile(arrayFilesConsumibles[i])){
			log_info(logImongo,"Se detecto un sabotaje en la cantidad de bloques del file %s", "oxigeno");
			//arreglarSabotajeCantBloquesEnFile(arrayFilesConsumibles[i]);
		}
		else if(haySabotajeSizeEnFile(arrayFilesConsumibles[i])){
			log_info(logImongo,"Se detecto un sabotaje en el tamanio del file %s", "oxigeno");
			//arreglarSabotajeSizeEnFile(arrayFilesConsumibles[i]);
		}
	}

	log_info(logImongo,"No se detecto ningun sabotaje en el super bloque o en los files");
}


bool haySabotajeCantBloquesEnSuperBloque(){
	return cantBloquesEnBlocks() != config_get_long_value(configSuperBloque, "BLOCKS");
}


void arreglarSabotajeCantBloquesEnSuperBloque(){

	config_set_value(configSuperBloque, "BLOCKS", string_itoa(cantBloquesEnBlocks()));

	log_info(logImongo,"Ya se soluciono el sabotaje, la cant de bloques"
			"del file system no era %d sino %d", cantBloquesEnBlocks(),
			config_get_long_value(configSuperBloque, "BLOCKS"));
}


bool haySabotajeBitmapEnSuperBloque(){


	t_list* listaBloques = list_create();


	//PARA CADA FILE
	obtenerBloquesOcupadosSegunFiles(pathOxigeno, listaBloques);
	obtenerBloquesOcupadosSegunFiles(pathComida, listaBloques);
	obtenerBloquesOcupadosSegunFiles(pathBasura, listaBloques);

	int idTripulante = 0;
	char* pathBitacoraTripulante = obtenerPathBitacora(idTripulante);

	while(pathBitacoraTripulante != NULL){

	}








	return result;
}


void obtenerBloquesOcupadosSegunFiles(t_file* archivo, t_list* listaBloques){

	char** arrayStringBloques = string_get_string_as_array(archivo->bloquesQueOcupa); // ["1", "2"]

	for(int i=0; i < archivo->cantidadBloques; i++){

		int numeroBloque = atoi(*(arrayStringBloques + i));
		list_add(listaBloques, &numeroBloque);
	}
}


char* obtenerPathBitacora(int idTripulante){

	char* pathBitacoraTripulante = strdup(pathBitacora);
	string_append(&pathBitacoraTripulante, "/Tripulante");
	string_append(&pathBitacoraTripulante, string_from_format("%d", idTripulante));
	string_append(&pathBitacoraTripulante, ".ims");

	if(verificarSiExiste(pathBitacoraTripulante)){
		return pathBitacoraTripulante;
	}
	else{
		return NULL;
	}
}


void arreglarSabotajeBitmapEnSuperBloque(){

}


bool haySabotajeSizeEnFile(tarea* fileConsumible){
	return sizeSegunBlocks(fileConsumible) !=
			config_get_long_value(fileConsumible->config, "SIZE");
}


// REVISAR QUE STRING_ITOA ANDA YA QUE RECIBE UN INT Y NO UN LONG
void arreglarSabotajeSizeEnFile(tarea* fileConsumible){
	config_set_value(fileConsumible->config, "SIZE", string_itoa(sizeSegunBlocks(fileConsumible)));
}


bool haySabotajeCantBloquesEnFile(tarea* fileConsumible){
	return cantBloquesSegunLista(fileConsumible)
			!= fileConsumible->file->cantidadBloques;
}


void arreglarSabotajeCantBloquesEnFile(tarea* fileConsumible){
	fileConsumible->file->cantidadBloques =
			cantBloquesSegunLista(fileConsumible->file->bloquesQueOcupa);
}


bool haySabotajeBloquesEnFile(tarea* fileConsumible){
	return true;
}


void arreglarSabotajeBloquesEnFile(tarea* fileConsumible){

}


// ------ FUNCIONAS AUXILIARES ------


uint32_t cantBloquesEnBlocks(){

	int fd = open(pathBloque,O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	return sb.st_size / config_get_long_value(configSuperBloque, "BLOCK_SIZE");
}


uint32_t sizeSegunBlocks(tarea* fileConsumible){
	return tamanioUltimoBloque(fileConsumible) +
			max(config_get_long_value(fileConsumible->config, "BLOCK_COUNT") - 1, 0) *
			config_get_long_value(configSuperBloque, "BLOCK_SIZE");
}


uint32_t tamanioUltimoBloque(tarea* fileConsumible){
	uint32_t posicionUltimoBloque = atoi(*(fileConsumible->file->bloquesQueOcupa
			+ strlen(fileConsumible->file->bloquesQueOcupa) - 1)) * superBloque->block_size;
	uint32_t offset=0;
	lock(&mutexMemoriaSecundaria);
	while(*(copiaMemoriaSecundaria + posicionUltimoBloque + offset) == fileConsumible->file->caracterLlenado
			&& offset < superBloque->block_size){

		offset ++;
	}
	unlock(&mutexMemoriaSecundaria);
	return offset;
}


uint32_t cantBloquesSegunLista(tarea* fileConsumible){
	//usar funcion q haga juancito
	t_list* listaBloquesOcupados = funcionJuancito(fileConsumible) // retorna una lista de ints
	uint32_t cantidadBloques = list_size(listaBloquesOcupados);
	list_destroy(listaBloquesOcupados);
	return cantidadBloques;
}
*/









t_list* convertirEnLista(char** arrayValores){

	t_list* lista = list_create();

	for(uint32_t i=0; *(arrayValores + i) != NULL; i++){

		uint32_t* valor = malloc(sizeof(uint32_t));
		*valor = atol(*(arrayValores + i));

		list_add(lista, valor);
	}

	return lista;
}


char* convertirEnString(t_list* listaEnteros){

	char* stringLista = string_new();
	string_append(&stringLista, "[");

	for(uint32_t i=0; i < list_size(listaEnteros) - 1; i++){

		string_append_with_format(&stringLista, "%d, ", *(long int*)(list_get(listaEnteros, i)));
	}

	string_append_with_format(&stringLista, "%d", *(int*)(list_get(listaEnteros, list_size(listaEnteros) - 1)));
	string_append(&stringLista, "]");

	return stringLista;
}















