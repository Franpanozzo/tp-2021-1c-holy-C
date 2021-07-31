#include "sabotajes.h"


void buscarSabotaje(){

	if(haySabotajeCantBloquesEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en la cantidad de bloques en el super bloque");
		arreglarSabotajeCantBloquesEnSuperBloque();
	}
	else if(haySabotajeBitmapEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en el bitmap del super bloque");
		arreglarSabotajeBitmapEnSuperBloque();
	}
	else{
		log_info(logImongo,"No se detecto ningun sabotaje en el super bloque");
	}

	sabotajesFile(oxigeno);
	sabotajesFile(comida);
	sabotajesFile(basura);

}


void sabotajesFile(tarea* structTarea){

	if(verificarSiExiste(structTarea->path)){

		if(haySabotajeBloquesEnFile(structTarea)){
			log_info(logImongo,"Se detecto un sabotaje en los bloques del file %s", structTarea->path);
			arreglarSabotajeBloquesEnFile(structTarea);
		}
		else if(haySabotajeCantBloquesEnFile(structTarea)){
			log_info(logImongo,"Se detecto un sabotaje en la cantidad de bloques del file %s", structTarea->path);
			arreglarSabotajeCantBloquesEnFile(structTarea);
		}

		else if(haySabotajeSizeEnFile(structTarea)){
			log_info(logImongo,"Se detecto un sabotaje en el tamanio del file %s", structTarea->path);
			arreglarSabotajeSizeEnFile(structTarea);
		}
		else{
			log_info(logImongo,"No se detecto ningun sabotaje en el file %s", structTarea->path);
		}
	}

}



//PROBADA V.1
bool haySabotajeCantBloquesEnSuperBloque(){

	uint32_t cant1 = cantBloquesEnBlocks();
	t_config* config = config_create(pathSuperBloque);
	uint32_t cant2 = config_get_long_value(config, "BLOCKS");


	bool result =  cant1 != cant2;
	log_info(logImongo,"el cantEnBlocks es %d y en el config %d", cant1, cant2);

	config_destroy(config);
	return result;
}

//PROBADA V.1
void arreglarSabotajeCantBloquesEnSuperBloque(){

	t_config* config = config_create(pathSuperBloque);
	uint32_t valor = config_get_long_value(config, "BLOCKS");

	log_info(logImongo,"Ya se soluciono el sabotaje, la cant de bloques"
			"del file system no era %d sino %d", valor, cantBloquesEnBlocks());

	config_set_value(configSuperBloque, "BLOCKS", string_itoa(cantBloquesEnBlocks()));

	config_destroy(config);
}


//PROBADA V.1
bool haySabotajeBitmapEnSuperBloque(){

	t_config* config = config_create(pathSuperBloque);
	char* stringBitmap = config_get_string_value(config, "BITMAP");
	uint32_t cantBloques = config_get_long_value(config, "BLOCKS");

	t_bitarray* bitmap = crearBitmap(stringBitmap);
	t_list* listaBloques = list_create();

	bool estaEnLaLista(int i){

		bool result = false;
		for(int c=0; c<list_size(listaBloques) && result == false; c++){

			result = i == * (int*) list_get(listaBloques, c);
		}
		return result;
	}


	if(verificarSiExiste(oxigeno->path)){
		t_config* config = config_create(oxigeno->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}
	if(verificarSiExiste(basura->path)){
		t_config* config = config_create(basura->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}
	if(verificarSiExiste(comida->path)){
		t_config* config = config_create(comida->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}

	for(int i=1; existeBitacoraTripulante(i); i++){
		char* path = pathBitacoraTripulante(i);
		t_config* configBitacora = config_create(path);
		agregarBloquesOcupados(listaBloques, configBitacora);
		free(path);
		config_destroy(configBitacora);
	}

	bool result = false;

	if(list_is_empty(listaBloques)){
		for(int i=0; i<cantBloques && result == false; i++){

			result = bitarray_test_bit(bitmap, i);
		}
	}
	else{
		//result = list_any_satisfy(listaBloques, (void*)compararConBitmap);
		for(int i=0; i<cantBloques && result == false; i++){

			if(estaEnLaLista(i)){
				result = !bitarray_test_bit(bitmap, i);
			}
			else{
				result = bitarray_test_bit(bitmap, i);
			}
		}
	}

	/*
	list_destroy(listaBloques);
	bitarray_destroy(bitmap);
	free(stringBitmap);
	config_destroy(config);
*/

	return result;
}


//PROBADA V.1
void arreglarSabotajeBitmapEnSuperBloque(){

	t_list* listaBloques = list_create();

	bool estaEnLaLista(int i){

		bool result = false;
		for(int c=0; c<list_size(listaBloques) && result == false; c++){

			result = i == * (int*) list_get(listaBloques, c);
		}
		return result;
	}

	if(verificarSiExiste(oxigeno->path)){
		t_config* config = config_create(oxigeno->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}
	if(verificarSiExiste(basura->path)){
		t_config* config = config_create(basura->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}
	if(verificarSiExiste(comida->path)){
		t_config* config = config_create(comida->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);
	}
	for(int i=1; existeBitacoraTripulante(i); i++){
		char* path = pathBitacoraTripulante(i);
		t_config* configBitacora = config_create(path);
		agregarBloquesOcupados(listaBloques, configBitacora);
		free(path);
		config_destroy(configBitacora);
	}

	t_config* configSB = config_create(pathSuperBloque);
	uint32_t cantBloques = config_get_long_value(configSB, "BLOCKS");
	int tamanioBitmap = (int) ceil ((float) cantBloques / (float) 8);
	t_bitarray* bitmap = bitarray_create_with_mode(malloc(tamanioBitmap), tamanioBitmap, MSB_FIRST);

	for(int i=0; i<cantBloques; i++){

		if(estaEnLaLista(i)){
			bitarray_set_bit(bitmap, i);
		}
		else{
			bitarray_clean_bit(bitmap, i);
		}
	}

	char* stringMap = convertirBitmapEnString(bitmap);
	config_set_value(configSB, "BITMAP", stringMap);

	log_info(logImongo,"Ya se soluciono el sabotaje, el bitmap indicaba mal el estado de un bloque");

	config_save(configSB);

	/*
	free(stringMap);
	bitarray_destroy(bitmap);
	list_destroy(listaBloques);
	*/
}


//PROBADA V.1
bool haySabotajeSizeEnFile(tarea* fileConsumible){

	uint32_t cant1 = sizeSegunBlocks(fileConsumible);
	t_config* config = config_create(fileConsumible->path);
	uint32_t cant2 = config_get_long_value(config, "SIZE");

	bool result =  cant1 != cant2;
	log_info(logImongo,"el size segun blocks es %d y en el config %d", cant1, cant2);

	return result;
}


// REVISAR QUE STRING_ITOA ANDA YA QUE RECIBE UN INT Y NO UN LONG
//PROBADA V.1
void arreglarSabotajeSizeEnFile(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);

	log_info(logImongo,"Ya se soluciono el sabotaje, el tamanio del "
			"file tanto no era %d sino %d",
			config_get_long_value(config, "SIZE"), sizeSegunBlocks(fileConsumible));

	config_set_value(config, "SIZE", string_itoa(sizeSegunBlocks(fileConsumible)));

	config_save(config);
}


//PROBADA V.1
bool haySabotajeCantBloquesEnFile(tarea* fileConsumible){

	uint32_t cant1 = cantBloquesSegunLista(fileConsumible);
	t_config* config = config_create(fileConsumible->path);
	uint32_t cant2 = config_get_long_value(config, "BLOCK_COUNT");

	bool result =  cant1 != cant2;
	log_info(logImongo,"la cant segun lista es %d y en el config %d", cant1, cant2);

	return result;
}


//PROBADA V.1
void arreglarSabotajeCantBloquesEnFile(tarea* fileConsumible){

	uint32_t cantBloquesReal = cantBloquesSegunLista(fileConsumible);
	t_config* config = config_create(fileConsumible->path);

	log_info(logImongo,"Ya se soluciono el sabotaje, la cant de bloques"
			"del file tanto no era %d sino %d",
			config_get_long_value(config, "BLOCK_COUNT"), cantBloquesReal);

	config_set_value(config, "BLOCK_COUNT", string_itoa(cantBloquesReal));

	config_save(config);
}


bool haySabotajeBloquesEnFile(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);
	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);
	char* md5calculado = obtenerMD5(listaBloques);

	char* md5real = config_get_string_value(config, "MD5_ARCHIVO");

	log_info(logImongo,"el md5 calculado es %s y el del config %s", md5calculado, md5real);

	bool result = strcmp(md5calculado, md5real) != 0;

	/*
	free(md5calculado);
	free(md5real);
	//falta liberar los elementos de la lista
	list_destroy(listaBloques);
	// falta liberar en arrayPosiciones
*/
	return result;
}


void guardarEnMemoriaSecundaria2(int* posicionesQueOcupa, char* contenido, int cantBloquesAocupar){

	int offsetMemoria = 0;
	int offsetContenido = 0;

	for(int i = 0; i < cantBloquesAocupar;i++){

		char* contenidoBloque = string_substring(contenido,
				offsetContenido, min(offsetContenido, superBloque->block_size));

		offsetContenido = superBloque->block_size;

		offsetMemoria = posicionesQueOcupa[i] * superBloque->block_size;

		lock(&mutexMemoriaSecundaria);
		memcpy(copiaMemoriaSecundaria + offsetMemoria , contenidoBloque, strlen(contenidoBloque));
		unlock(&mutexMemoriaSecundaria);

		free(contenidoBloque);
	}
//	actualizarBitArray(posicionesQueOcupa, cantBloquesAocupar);
}



void arreglarSabotajeBloquesEnFile(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);
	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);
	char* md5calculado = obtenerMD5(listaBloques);
	char* md5anterior = config_get_string_value(config, "MD5_ARCHIVO");
	config_set_value(config, "MD5_ARCHIVO", md5calculado);

	log_info(logImongo,"YA CAMBIO EL MD5");

	uint32_t cantCaracteresAescribir = config_get_long_value(config, "SIZE");
	log_info(logImongo,"PROBANDO 1");
	char* stringCaracter = config_get_string_value(config, "CARACTER_LLENADO");
	log_info(logImongo,"PROBANDO 2");
	char caracter = * stringCaracter;
	t_config* configSB = config_create(pathSuperBloque);
	log_info(logImongo,"PROBANDO 3");
	uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");

	log_info(logImongo,"PREPARO TODO");

	for(uint32_t i = 0; i < list_size(listaBloques); i++){

		consumirBloque(* (uint32_t *)list_get(listaBloques, i), tamanioBloque);
		escribirBloque(* (uint32_t *)list_get(listaBloques, i),
				string_repeat(caracter, min(cantCaracteresAescribir, tamanioBloque)));

		cantCaracteresAescribir = max(cantCaracteresAescribir - tamanioBloque, 0);
	}

	config_save(config);

	log_info(logImongo,"Ya se soluciono el sabotaje, se cambio el md5 de %s a %s "
			"para el nuevo orden de bloques", md5anterior, md5calculado);

	/*
	free(md5anterior);
	free(stringCaracter);
	*/
}


void escribirBloque(uint32_t posicionDelBloque, char* contenido){

	t_config* configSB = config_create(pathSuperBloque);
	int tamanioBloque = config_get_int_value(configSB, "BLOCK_SIZE");
	config_destroy(configSB);

	log_info(logImongo,"ACA 1");

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionDelBloque * tamanioBloque, contenido, strlen(contenido));
	unlock(&mutexMemoriaSecundaria);

	log_info(logImongo,"ACA 2");

	char* contenidoImprimible = string_duplicate(contenido);
	string_append(&contenidoImprimible, "\0");

	log_info(logImongo,"ACA 3");

	log_info(logImongo,"SE PUSO %s EN EL BLOQUE %d", contenidoImprimible, posicionDelBloque);
}


void consumirBloque(uint32_t posicionDelBloque, uint32_t cant){

	char* contenido = string_repeat('\0', cant);
	t_config* configSB = config_create(pathSuperBloque);
	int tamanioBloque = config_get_int_value(configSB, "BLOCK_SIZE");
	config_destroy(configSB);

	log_info(logImongo,"ACA 4");

	lock(&mutexMemoriaSecundaria);
	memcpy(copiaMemoriaSecundaria + posicionDelBloque * tamanioBloque, contenido, cant);
	unlock(&mutexMemoriaSecundaria);

	log_info(logImongo,"ACA 5");

	log_info(logImongo,"SE LLENO DE BARRA CEROS EL BLOQUE %d", posicionDelBloque);
}

// ------ FUNCIONAS AUXILIARES ------


// PROBADA
uint32_t cantBloquesEnBlocks(){

	int fd = open(pathBloque,O_RDWR|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
	struct stat sb;

	if (fstat(fd, &sb) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

	return sb.st_size / config_get_long_value(configSuperBloque, "BLOCK_SIZE");
}


// PROBADA
uint32_t sizeSegunBlocks(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);
	t_config* configSB = config_create(pathSuperBloque);

	return tamanioUltimoBloque(fileConsumible) +
			max(config_get_long_value(config, "BLOCK_COUNT") - 1, 0) *
			config_get_long_value(configSB, "BLOCK_SIZE");
}


// PROBADA
uint32_t tamanioUltimoBloque(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);

	t_list* bloquesOcupados = convertirEnLista(config_get_array_value(config, "BLOCKS"));

	uint32_t numeroUltimoBloque = * (long int*)list_get(bloquesOcupados, list_size(bloquesOcupados) - 1);

	log_info(logImongo,"El numero del utlimo bloque cuando la lista tiene %d elementos es %d",
			list_size(bloquesOcupados), numeroUltimoBloque);

	uint32_t offset=0;

	char caracterLLenado = *config_get_string_value(config, "CARACTER_LLENADO");

	log_info(logImongo,"El caracter de llenado es %c", caracterLLenado);

	uint32_t tamanioBloque = config_get_long_value(configSuperBloque, "BLOCK_SIZE");

	lock(&mutexMemoriaSecundaria);
	while(*(copiaMemoriaSecundaria + numeroUltimoBloque * tamanioBloque + offset) == caracterLLenado
			&& offset < tamanioBloque){

		offset ++;
	}
	unlock(&mutexMemoriaSecundaria);

	return offset;
}

/*
uint32_t max(long int n1, long int n2){

	if(n1 > n2)
		return n1;
	else
		return n2;
}
*/

// PROBADA
uint32_t cantBloquesSegunLista(tarea* fileConsumible){

	t_config* config = config_create(fileConsumible->path);
	t_list* listaBloquesOcupados = convertirEnLista(config_get_array_value(config, "BLOCKS"));
	uint32_t cantidadBloques = list_size(listaBloquesOcupados);
	list_destroy(listaBloquesOcupados);
	config_destroy(config);
	return cantidadBloques;
}


//PROBADA
t_bitarray* crearBitmap(char* stringBitmap){

	int tamanioBitmap = (int) ceil ((float) strlen(stringBitmap) / (float) 8);
			//((float)config_get_long_value(configSuperBloque, "BLOCKS")

	t_bitarray* bitmap = bitarray_create_with_mode(malloc(tamanioBitmap), tamanioBitmap, MSB_FIRST);

	for(int i=0; i<strlen(stringBitmap); i++){

		if(*(stringBitmap + i) == '1'){
			bitarray_set_bit(bitmap, i);
		}
		else{
			bitarray_clean_bit(bitmap, i);
		}
	}

	return bitmap;
}


//PROBADA
char* convertirBitmapEnString(t_bitarray* bitmap){

	char* stringBitmap = string_new();

	for(int i=0; i< bitarray_get_max_bit(bitmap); i++){

		if(bitarray_test_bit(bitmap, i))
			string_append(&stringBitmap, "1");
		else
			string_append(&stringBitmap, "0");
	}

	return stringBitmap;
}


// HECHA
void agregarBloquesOcupados(t_list* listaBloques, t_config* config){

	t_list* listaAux = convertirEnLista(config_get_array_value(config, "BLOCKS"));
	list_add_all(listaBloques, listaAux);
	list_destroy(listaAux);
}
