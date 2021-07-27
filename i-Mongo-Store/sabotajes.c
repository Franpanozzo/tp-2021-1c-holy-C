#include "sabotajes.h"


void buscarSabotaje(){

	if(haySabotajeCantBloquesEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en la cantidad de bloques en el super bloque");
		//arreglarSabotajeCantBloquesEnSuperBloque();
	}
	else if(haySabotajeBitmapEnSuperBloque()){
		log_info(logImongo,"Se detecto un sabotaje en el bitmap del super bloque");
		//arreglarSabotajeBitmapEnSuperBloque();
	}

	tarea** arrayFilesConsumibles[3] = {oxigeno, comida, basura};
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


//HECHA
bool haySabotajeCantBloquesEnSuperBloque(){
	return cantBloquesEnBlocks() != config_get_long_value(configSuperBloque, "BLOCKS");
}

//HECHA
void arreglarSabotajeCantBloquesEnSuperBloque(){

	log_info(logImongo,"Ya se soluciono el sabotaje, la cant de bloques"
			"del file system no era %d sino %d",
			config_get_long_value(configSuperBloque, "BLOCKS"), cantBloquesEnBlocks());

	config_set_value(configSuperBloque, "BLOCKS", string_itoa(cantBloquesEnBlocks()));
}


bool haySabotajeBitmapEnSuperBloque(){

	char* stringBitmap = config_get_string_value(configSuperBloque, "BITMAP");
	t_bitarray* bitmap = crearBitmap(stringBitmap);

	bool compararConBitmap(int* n){
		return bitarray_test_bit(bitmap, *n);
	}

	t_list* listaBloques = list_create();

	//PARA CADA FILE

	if(verificarSiExiste(pathOxigeno)){
		agregarBloquesOcupados(listaBloques, oxigeno->config);
	}
	if(verificarSiExiste(pathBasura)){
		agregarBloquesOcupados(listaBloques, basura->config);
	}
	if(verificarSiExiste(pathComida)){
		agregarBloquesOcupados(listaBloques, comida->config);
	}

	// FALTA PARTE DE BITACORAS

	bool result = list_any_satisfy(listaBloques, (void*)compararConBitmap);

	list_destroy(listaBloques);
	bitarray_destroy(bitmap);
	free(stringBitmap);

	return result;
}


void arreglarSabotajeBitmapEnSuperBloque(){

	t_list* listaBloques = list_create();

	//PARA CADA FILE

	if(verificarSiExiste(pathOxigeno)){
		agregarBloquesOcupados(listaBloques, oxigeno->config);
	}
	if(verificarSiExiste(pathBasura)){
		agregarBloquesOcupados(listaBloques, basura->config);
	}
	if(verificarSiExiste(pathComida)){
		agregarBloquesOcupados(listaBloques, comida->config);
	}
	// FALTA PARTE DE BITACORAS

	int tamanioBitmap = (int) ceil ((float) config_get_long_value(configSuperBloque, "BLOCKS") / (float) 8);
	t_bitarray* bitmap = bitarray_create_with_mode(malloc(tamanioBitmap), tamanioBitmap, MSB_FIRST);

	for(int i=0; i<list_size(listaBloques); i++){
		bitarray_set_bit(bitmap, i);
	}

	char* stringMap = convertirBitmapEnString(bitmap);
	config_set_value(configSuperBloque, "BITMAP", stringMap);

	log_info(logImongo,"Ya se soluciono el sabotaje, el bitmap indicaba mal el estado de un bloque");

	free(stringMap);
	bitarray_destroy(bitmap);
	list_destroy(listaBloques);
}


//HECHA
bool haySabotajeSizeEnFile(tarea* fileConsumible){
	return sizeSegunBlocks(fileConsumible) !=
			config_get_long_value(fileConsumible->config, "SIZE");
}


// REVISAR QUE STRING_ITOA ANDA YA QUE RECIBE UN INT Y NO UN LONG
void arreglarSabotajeSizeEnFile(tarea* fileConsumible){

	log_info(logImongo,"Ya se soluciono el sabotaje, el tamanio del "
			"file tanto no era %d sino %d",
			config_get_long_value(fileConsumible->config, "SIZE"), cantBloquesEnBlocks());

	config_set_value(fileConsumible->config, "SIZE", string_itoa(sizeSegunBlocks(fileConsumible)));
}


//HECHA
bool haySabotajeCantBloquesEnFile(tarea* fileConsumible){
	return cantBloquesSegunLista(fileConsumible)
			!= config_get_long_value(fileConsumible->config, "BLOCK_COUNT") ;
}


//HECHA
void arreglarSabotajeCantBloquesEnFile(tarea* fileConsumible){

	uint32_t cantBloquesReal = cantBloquesSegunLista(fileConsumible);

	log_info(logImongo,"Ya se soluciono el sabotaje, la cant de bloques"
			"del file tanto no era %d sino %d",
			config_get_long_value(fileConsumible->config, "BLOCK_COUNT"),
			cantBloquesReal);

	config_set_value(fileConsumible->config, "BLOCK_COUNT",
			string_itoa(cantBloquesReal));
}


bool haySabotajeBloquesEnFile(tarea* fileConsumible){

	char** arrayPosiciones = config_get_array_value(fileConsumible->config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);
	char* md5calculado = calcularMd5(listaBloques);

	char* md5real = config_get_string_value(fileConsumible->config, "MD5");

	bool result = strcmp(md5calculado, md5real) != 0;

	free(md5calculado);
	free(md5real);
	//falta liberar los elementos de la lista
	list_destroy(listaBloques);
	// falta liberar en arrayPosiciones

	return result;
}


void arreglarSabotajeBloquesEnFile(tarea* fileConsumible){

	char** arrayPosiciones = config_get_array_value(fileConsumible->config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);
	char* md5calculado = calcularMd5(listaBloques);
	config_set_value(fileConsumible->config, "MD5", md5calculado);

	uint32_t cantCaracteresAescribir = config_get_long_value(fileConsumible->config, "SIZE");
	char* stringCaracter = config_get_string_value(fileConsumible->config, "CARACTER_LLENADO");
	char caracter = * stringCaracter;
	uint32_t tamanioBloque = config_get_long_value(fileConsumible->config, "BLOCKS_SIZE");

	for(uint32_t i = 0; i < list_size(listaBloques); i++){

		consumirBloque(* (uint32_t *)list_get(listaBloques, i), tamanioBloque);
		escribirBloque(* (uint32_t *)list_get(listaBloques, i),
				string_repeat(caracter, min(cantCaracteresAescribir, tamanioBloque)));

		cantCaracteresAescribir = max(cantCaracteresAescribir - tamanioBloque, 0);
	}

	char* md5anterior = config_get_string_value(fileConsumible->config, "MD5");

	log_info(logImongo,"Ya se soluciono el sabotaje, se cambio el md5 de %s a %s "
			"para el nuevo orden de bloques", md5anterior, md5calculado);

	free(md5anterior);
	free(stringCaracter);
}

// escribirBloque(uint32_t posicionDelBloque, char* contenido);
// consumirBloque(uint32_t posicionDelBloque, uint32_t cant);

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
	return tamanioUltimoBloque(fileConsumible) +
			max(config_get_long_value(fileConsumible->config, "BLOCK_COUNT") - 1, 0) *
			config_get_long_value(configSuperBloque, "BLOCK_SIZE");
}


// PROBADA
uint32_t tamanioUltimoBloque(tarea* fileConsumible){

	t_list* bloquesOcupados = convertirEnLista(config_get_array_value(fileConsumible->config, "BLOCKS"));

	uint32_t numeroUltimoBloque = * (long int*)list_get(bloquesOcupados, list_size(bloquesOcupados) - 1);

	log_info(logImongo,"El numero del utlimo bloque cuando la lista tiene %d elementos es %d",
			list_size(bloquesOcupados), numeroUltimoBloque);

	uint32_t offset=0;

	char caracterLLenado = *config_get_string_value(fileConsumible->config, "CARACTER_LLENADO");

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


uint32_t max(long int n1, long int n2){

	if(n1 > n2)
		return n1;
	else
		return n2;
}


// PROBADA
uint32_t cantBloquesSegunLista(tarea* fileConsumible){
	t_list* listaBloquesOcupados = convertirEnLista(config_get_array_value(fileConsumible->config, "BLOCKS"));
	uint32_t cantidadBloques = list_size(listaBloquesOcupados);
	list_destroy(listaBloquesOcupados);
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


// PROBADA
t_list* convertirEnLista(char** arrayValores){

	t_list* lista = list_create();

	for(uint32_t i=0; *(arrayValores + i) != NULL; i++){

		uint32_t* valor = malloc(sizeof(uint32_t));
		*valor = atol(*(arrayValores + i));

		list_add(lista, valor);
	}

	return lista;
}


// PROBADA
char* convertirListaEnString(t_list* listaEnteros){

	char* stringLista = string_new();
	string_append(&stringLista, "[");

	if(!list_is_empty(listaEnteros)){

		for(uint32_t i=0; i < list_size(listaEnteros) - 1; i++){

			string_append_with_format(&stringLista, "%d, ",
					*(long int*)(list_get(listaEnteros, i)));

		}

		string_append_with_format(&stringLista, "%d", *(int*)(list_get(listaEnteros, list_size(listaEnteros) - 1)));
	}

	string_append(&stringLista, "]");

	return stringLista;
}


// HECHA
void agregarBloquesOcupados(t_list* listaBloques, t_config* config){

	t_list* listaAux = convertirEnLista(config_get_array_value(config, "BLOCKS"));
	list_add_all(listaBloques, listaAux);
	list_destroy(listaAux);
}
