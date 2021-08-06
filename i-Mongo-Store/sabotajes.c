#include "sabotajes.h"

void fsck(){

	if(haySabotajeCantBloquesEnSuperBloque()){
		log_info(logImongo,"---- Se detecto un sabotaje en la cantidad de bloques en el super bloque ----");
		arreglarSabotajeCantBloquesEnSuperBloque();
	}
	else if(haySabotajeBitmapEnSuperBloque()){
		log_info(logImongo,"---- Se detecto un sabotaje en el bitmap del super bloque ----");
		arreglarSabotajeBitmapEnSuperBloque();
	}
	else{
		log_info(logImongo,"---- NO SE DETECTO NINGUN SABOTAJE EN EL SUPER BLOQUE ----");
	}

	sabotajesFile(oxigeno);
	sabotajesFile(comida);
	sabotajesFile(basura);

	modificarHaySabotaje(false);

	while(leerCantEscriturasPendientes() > 0){
		modificarCantEscriturasPendientes(leerCantEscriturasPendientes() - 1);
		sem_post(&sabotajeResuelto);
	}
}

/*
 * 1.FIJARSE SI HAY UN BLOQUE FANTASMA, NO PUEDO HACER ANTES EL BITMAP NI
 *
 * ME FIJO SI EN EL BLOCK.IMS ESTAN TODOS LOS BLOQUES ESCRITOS CON EL
 * CARACTER DE LLENADO.
 *
 * AHI YA SE QUE TODO_ EL RESTO DE LAS COSAS ESTAN BIEN, LO UNICO QUE HAY Q HACER SE SACARLO DE LA LISTA
 *
 *
 * 2.FIJARSE SI HAY ALGUN BLOQUE CON UN TAMANIO IRREAL EXTRA
 *
 * ME FIJO CON EL BLOCKS DEL SUPER BLOQUE. SOLO NECESITA QUE SE HAYA DETECTADO
 * SI HUBO O NO SABOTAJE EN EL BLOCKS DEL SUPER BLOQUE ANTES.
 *
 * LA RESOLUCION TAMBIEN TENGO QUE VER EL BITMAP Y LOS BLOQUES QUE OCUPAN LOS OTROS
 * PARA SABER SI ME SACARO UN BLOQUE Q YO TENIA Y VOLVERLO A AGREGAR Y TAMBIEN
 * TENGO QUE VER EL SIZE PARA VER SI ME LO SACARON EL BLOQUE O LOS QUE TENGO ESTAN BIEN
 *
 *
 * 3.FIJARSE SI ME ELIMINARON UN BLOQUE
 *
 *
 *
 */





void sabotajesFile(t_file* archivo){

	if(haySabotajeBlocksBloqueExtra2(archivo)){
		log_info(logImongo,"---- Se detecto un sabotaje, hay un bloque extra en el file %s ----", archivo->path);
		arreglarSabotajeBlocksBloqueExtra(archivo);
	}
	else if(haySabotajeBloquesEnFile(archivo)){
		log_info(logImongo,"---- Se detecto un sabotaje en el orden de los bloques del file %s ----", archivo->path);
		arreglarSabotajeBloquesEnFile(archivo);
	}
	else if(haySabotajeCantBloquesEnFile(archivo)){
		log_info(logImongo,"---- Se detecto un sabotaje en la cantidad de bloques del file %s ----", archivo->path);
		arreglarSabotajeCantBloquesEnFile(archivo);
	}

	else if(haySabotajeSizeEnFile(archivo)){
		log_info(logImongo,"---- Se detecto un sabotaje en el tamanio del file %s ----", archivo->path);
		arreglarSabotajeSizeEnFile(archivo);
	}
	else{
		log_info(logImongo,"---- NO SE DETECTO NINGUN SABOTAJE EN EL %s ----", archivo->path);
	}
}



//PROBADA V.1
bool haySabotajeCantBloquesEnSuperBloque(){

	uint32_t cant1 = cantBloquesEnBlocks();
	t_config* config;
	crearConfig(&config,superBloque->path);
	uint32_t cant2 = config_get_long_value(config, "BLOCKS");
	config_destroy(config);

	//log_info(logImongo,"el cantEnBlocks es %d y en el config %d", cant1, cant2);

	return cant1 != cant2;
}

//PROBADA V.1
void arreglarSabotajeCantBloquesEnSuperBloque(){

	t_config* config;
	t_config* configSB;

	crearConfig(&config,superBloque->path);
	crearConfig(&configSB,superBloque->path);

	uint32_t valor = config_get_long_value(config, "BLOCKS");

	log_info(logImongo,"YA SE SOLUCIONO EL SABOTAJE, la cant de bloques"
			" del file system no era %d sino %d", valor, cantBloquesEnBlocks());

	char* stringCantBloques = string_itoa(cantBloquesEnBlocks());

	config_set_value(configSB, "BLOCKS", stringCantBloques);

	config_save(configSB);
	config_destroy(config);
	config_destroy(configSB);
	free(stringCantBloques);
}


//PROBADA V.1
bool haySabotajeBitmapEnSuperBloque(){

	bool result = false;

	if(!haySabotajeBlocksBloqueExtra2(oxigeno) &&
			!haySabotajeBlocksBloqueExtra2(comida) && !haySabotajeBlocksBloqueExtra2(basura)){

		t_config* configSB;
		crearConfig(&configSB,superBloque->path);
		char* stringBitmap = config_get_string_value(configSB, "BITMAP");
		uint32_t cantBloques = config_get_long_value(configSB, "BLOCKS");

		int tamanioBitmap = (int) ceil ((float) strlen(stringBitmap) / (float) 8);
		char* espacioBitmap = malloc(tamanioBitmap);
		t_bitarray* bitmap = bitarray_create_with_mode(espacioBitmap, tamanioBitmap, MSB_FIRST);
		cargarBitmap(stringBitmap, bitmap);

		t_list* listaBloques = list_create();

		bool estaEnLaLista(int i){

			bool result = false;
			for(int c=0; c<list_size(listaBloques) && result == false; c++){

				result = i == * (int*) list_get(listaBloques, c);
			}
			return result;
		}

		t_config* config;
		crearConfig(&config,oxigeno->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);

		crearConfig(&config,basura->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);

		crearConfig(&config,comida->path);
		agregarBloquesOcupados(listaBloques, config);
		config_destroy(config);


		for(int i=1; existeBitacoraTripulante(i); i++){
			char* stringIdTripulante = string_itoa(i);
			char* path = pathBitacoraTripulante(stringIdTripulante);
			t_config* configBitacora = config_create(path);
			agregarBloquesOcupados(listaBloques, configBitacora);
			free(stringIdTripulante);
			free(path);
			config_destroy(configBitacora);
		}


		if(list_is_empty(listaBloques)){
			for(int i=0; i<cantBloques && result == false; i++){

				result = bitarray_test_bit(bitmap, i);
			}
		}
		else{
			for(int i=0; i<cantBloques && result == false; i++){

				if(estaEnLaLista(i)){
					result = !bitarray_test_bit(bitmap, i);
				}
				else{
					result = bitarray_test_bit(bitmap, i);
				}
			}
		}

		config_destroy(configSB);
		free(espacioBitmap);
		bitarray_destroy(bitmap);

		void eliminarEntero(int* n){
			free(n);
		}

		list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);
		//log_info(logImongo,"SABO 2 SIN FALLA 1.5");
	}

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

	t_config* config;
	crearConfig(&config,oxigeno->path);
	agregarBloquesOcupados(listaBloques, config);
	config_destroy(config);

	crearConfig(&config,basura->path);
	agregarBloquesOcupados(listaBloques, config);
	config_destroy(config);

	crearConfig(&config,comida->path);
	agregarBloquesOcupados(listaBloques, config);
	config_destroy(config);


	for(int i=1; existeBitacoraTripulante(i); i++){
		char* stringIdTripulante = string_itoa(i);
		char* path = pathBitacoraTripulante(stringIdTripulante);
		t_config* configBitacora = config_create(path);
		agregarBloquesOcupados(listaBloques, configBitacora);
		free(path);
		free(stringIdTripulante);
		config_destroy(configBitacora);
	}

	t_config* configSB;
	crearConfig(&configSB,superBloque->path);
	uint32_t cantBloques = config_get_long_value(configSB, "BLOCKS");
	int tamanioBitmap = (int) ceil ((float) cantBloques / (float) 8);
	char* espacioBitmap = malloc(tamanioBitmap);
	t_bitarray* bitmap = bitarray_create_with_mode(espacioBitmap, tamanioBitmap, MSB_FIRST);

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

	log_info(logImongo,"---- YA SE SOLUCIONO EL SABOTAJE, "
			"el bitmap indicaba mal el estado de un bloque ----");

	config_save(configSB);
	config_destroy(configSB);
	free(espacioBitmap);
	bitarray_destroy(bitmap);
	free(stringMap);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);
}


//PROBADA V.1
bool haySabotajeSizeEnFile(t_file* archivo){

	uint32_t cant1 = sizeSegunBlocks(archivo);
	t_config* config = config_create(archivo->path);
	uint32_t cant2 = config_get_long_value(config, "SIZE");
	config_destroy(config);

	//log_info(logImongo,"el size segun blocks es %d y en el config %d", cant1, cant2);
	return cant1 != cant2;
}


// REVISAR QUE STRING_ITOA ANDA YA QUE RECIBE UN INT Y NO UN LONG
//PROBADA V.1
void arreglarSabotajeSizeEnFile(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	uint32_t size = sizeSegunBlocks(archivo);

	log_info(logImongo,"---- YA SE SOLUCIONO EL SABOTAJE, el tamanio del file %s"
			" no era %d sino %d ----", archivo->path, config_get_long_value(config, "SIZE"), size);

	char* stringSize = string_itoa(size);

	config_set_value(config, "SIZE", stringSize);

	config_save(config);
	config_destroy(config);
	free(stringSize);
}


//PROBADA V.1
bool haySabotajeCantBloquesEnFile(t_file* archivo){

	uint32_t cant1 = cantBloquesSegunLista(archivo);
	t_config* config;
	crearConfig(&config,archivo->path);
	uint32_t cant2 = config_get_long_value(config, "BLOCK_COUNT");

	//log_info(logImongo,"la cant segun lista es %d y en el config %d", cant1, cant2);
	config_destroy(config);

	return cant1 != cant2;
}


//PROBADA V.1
void arreglarSabotajeCantBloquesEnFile(t_file* archivo){

	uint32_t cantBloquesReal = cantBloquesSegunLista(archivo);
	t_config* config;
	crearConfig(&config,archivo->path);

	log_info(logImongo,"---- YA SE SOLUCIONO EL SABOTAJE, la cant de bloques"
			"del file tanto no era %d sino %d ----",
			config_get_long_value(config, "BLOCK_COUNT"), cantBloquesReal);

	char* stringCantBloques = string_itoa(cantBloquesReal);
	config_set_value(config, "BLOCK_COUNT", stringCantBloques);

	config_save(config);
	config_destroy(config);
	free(stringCantBloques);
}


bool haySabotajeBloquesEnFile(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);

	bool result = false;
	//log_info(logImongo,"LA LISTA ES %s", convertirListaEnString(listaBloques));

	if(!list_is_empty(listaBloques)){

		char* md5calculado = obtenerMD5(listaBloques);

		char* md5real = config_get_string_value(config, "MD5_ARCHIVO");

		log_info(logImongo,"el md5 calculado es %s y el del config %s", md5calculado, md5real);

		result = strcmp(md5calculado, md5real) != 0;


		free(md5calculado);
		//free(md5real);
	}

	config_destroy(config);
	liberarDoblesPunterosAChar(arrayPosiciones);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);

	return result;
}


void arreglarSabotajeBloquesEnFile(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);

	uint32_t cantCaracteresAescribir = config_get_long_value(config, "SIZE");
	char* stringCaracter = config_get_string_value(config, "CARACTER_LLENADO");
	char caracter = * stringCaracter;
	t_config* configSB;
	crearConfig(&configSB,superBloque->path);
	uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");

	for(uint32_t i = 0; i < list_size(listaBloques); i++){

		uint32_t bloque = * (uint32_t *)list_get(listaBloques, i);
		consumirBloque(bloque, tamanioBloque, tamanioBloque);
		char* contenido = string_repeat(caracter, min(cantCaracteresAescribir, tamanioBloque));
		escribirBloque(bloque, contenido, 0);
		free(contenido);
		cantCaracteresAescribir = max(cantCaracteresAescribir - tamanioBloque, 0);
	}


	log_info(logImongo,"---- YA SE SOLUCIONO EL SABOTAJE, se cambio el orden de los bloques ----");

	config_save(config);
	config_destroy(config);
	config_destroy(configSB);
	//free(stringCaracter);
	liberarDoblesPunterosAChar(arrayPosiciones);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(archivo->bloques, (void*) eliminarEntero);


	archivo->bloques = listaBloques;
	/*
	free(md5anterior);
	free(stringCaracter);
	*/
}


bool haySabotajeBlocksBloqueExtra(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);

	t_config* configSB;
	crearConfig(&configSB,superBloque->path);

	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);

	char* stringCaracterLlenado = config_get_string_value(config, "CARACTER_LLENADO");
	char caracterLLenado = *stringCaracterLlenado;

	uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");
	uint32_t cantBloques = config_get_long_value(configSB, "BLOCKS");


	bool noPertenceAlArchivo(uint32_t* bloque){

		lock(&mutexMemoriaSecundaria);
		bool result = *(copiaMemoriaSecundaria + *bloque * tamanioBloque) != caracterLLenado
				|| *bloque > cantBloques;
		unlock(&mutexMemoriaSecundaria);

		return result;
	}

	config_destroy(config);
	config_destroy(configSB);

	liberarDoblesPunterosAChar(arrayPosiciones);

	bool result = false;

	if(!list_is_empty(listaBloques)){

		result = list_any_satisfy(listaBloques, (void *) noPertenceAlArchivo);

		void eliminarEntero(uint32_t* n){
			free(n);
		}

		list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);
	}
	else{
		list_destroy(listaBloques);
	}


	return result;
}


bool haySabotajeBlocksBloqueExtra2(t_file* archivo){

	t_config* configSB;
	crearConfig(&configSB,superBloque->path);

	long int tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");

	t_config* config;
	crearConfig(&config,archivo->path);

	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);

	long int tamanioArchivo = config_get_long_value(config, "SIZE");

	long int cantBloquesMenosUno = (list_size(listaBloques) - 1);

	log_info(logImongo, "LA CANT DE BLOQUE ES %d, EL TAMANIO DEL ARCHIVO ES %d",
			cantBloquesMenosUno, tamanioArchivo);

	bool result = cantBloquesMenosUno * tamanioBloque >= tamanioArchivo;

	config_destroy(config);
	config_destroy(configSB);

	liberarDoblesPunterosAChar(arrayPosiciones);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);

	return haySabotajeCantBloquesEnFile(archivo) && result;
}


void arreglarSabotajeBlocksBloqueExtra(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);

	t_config* configSB;
	crearConfig(&configSB,superBloque->path);

	char** arrayPosiciones = config_get_array_value(config, "BLOCKS");
	t_list* listaBloques = convertirEnLista(arrayPosiciones);

	char* stringCaracterLlenado = config_get_string_value(config, "CARACTER_LLENADO");
	char caracterLLenado = *stringCaracterLlenado;

	uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");
	uint32_t cantBloques = config_get_long_value(configSB, "BLOCKS");

	bool noPertenceAlArchivo(uint32_t* bloque){

		log_info(logImongo, "LA CANT DE BLOQUE ES %d "
				"Y EL BLOQUE QUE SE ESTA COMPARANDO ES %d", cantBloques, *bloque);

		bool result = *bloque >= cantBloques;

		if(!result){
			lock(&mutexMemoriaSecundaria);
			result = *(copiaMemoriaSecundaria + *bloque * tamanioBloque) != caracterLLenado;
			unlock(&mutexMemoriaSecundaria);
		}

		return result;
	}

	config_destroy(configSB);

	liberarDoblesPunterosAChar(arrayPosiciones);

	bool sigueConsultando = true;

	for(uint32_t i=0; i < list_size(listaBloques) && sigueConsultando; i++){

		if(noPertenceAlArchivo(list_get(listaBloques, i))){

			uint32_t* bloque = list_remove(listaBloques, i);
			log_info(logImongo, "EL BLOQUE %d NO PERTENECE AL ARCHIVO", *bloque);
			sigueConsultando = false;
			free(bloque);
		}
	}

	char* stringListaBloques = convertirListaEnString(listaBloques);

	config_set_value(config, "BLOCKS", stringListaBloques);
	config_save(config);
	config_destroy(config);

	log_info(logImongo,"---- YA SE SOLUCIONO EL SABOTAJE, "
			"la lista de bloques en realidad era %s ----", stringListaBloques);

	free(stringListaBloques);

	if(list_is_empty(listaBloques)){

		list_destroy(listaBloques);
	}
	else{
		void eliminarEntero(uint32_t* n){
			free(n);
		}

		list_destroy_and_destroy_elements(listaBloques, (void*) eliminarEntero);
	}
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

	t_config* configSB = config_create(superBloque->path);
	uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");
	config_destroy(configSB);

	return sb.st_size / tamanioBloque;
}


// PROBADA
uint32_t sizeSegunBlocks(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	t_config* configSB;
	crearConfig(&configSB,superBloque->path);

	uint32_t blockCount = max(config_get_long_value(config, "BLOCK_COUNT") - 1, 0);
	uint32_t blockSize = config_get_long_value(configSB, "BLOCK_SIZE");
	config_destroy(config);
	config_destroy(configSB);

	return tamanioUltimoBloque(archivo) + blockCount * blockSize;
}


// PROBADA
uint32_t tamanioUltimoBloque(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	char** arrayBloques = config_get_array_value(config, "BLOCKS");
	t_list* bloquesOcupados = convertirEnLista(arrayBloques);
	uint32_t offset=0;

	if(!list_is_empty(bloquesOcupados)){

		uint32_t numeroUltimoBloque = * (long int*)list_get(bloquesOcupados, list_size(bloquesOcupados) - 1);

		//log_info(logImongo,"El numero del utlimo bloque cuando la lista tiene %d elementos es %d",
		//		list_size(bloquesOcupados), numeroUltimoBloque);


		char* stringCaracterLlenado = config_get_string_value(config, "CARACTER_LLENADO");
		char caracterLLenado = *stringCaracterLlenado;

		//log_info(logImongo,"El caracter de llenado es %c", caracterLLenado);

		t_config* configSB;
		crearConfig(&configSB,superBloque->path);
		uint32_t tamanioBloque = config_get_long_value(configSB, "BLOCK_SIZE");

		lock(&mutexMemoriaSecundaria);
		while(*(copiaMemoriaSecundaria + numeroUltimoBloque * tamanioBloque + offset) == caracterLLenado
				&& offset < tamanioBloque){

			offset ++;
		}
		unlock(&mutexMemoriaSecundaria);

		config_destroy(configSB);
	}

	config_destroy(config);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(bloquesOcupados, (void*) eliminarEntero);
	liberarDoblesPunterosAChar(arrayBloques);
	return offset;
}


// PROBADAs
uint32_t cantBloquesSegunLista(t_file* archivo){

	t_config* config;
	crearConfig(&config,archivo->path);
	char** arrayBloques = config_get_array_value(config, "BLOCKS");
	t_list* listaBloquesOcupados = convertirEnLista(arrayBloques);
	uint32_t cantidadBloques = list_size(listaBloquesOcupados);

	void eliminarEntero(uint32_t* n){
		free(n);
	}

	list_destroy_and_destroy_elements(listaBloquesOcupados, (void*) eliminarEntero);
	liberarDoblesPunterosAChar(arrayBloques);
	config_destroy(config);

	return cantidadBloques;
}


//PROBADA
void cargarBitmap(char* stringBitmap,  t_bitarray* bitmap){

	for(int i=0; i<strlen(stringBitmap); i++){

		if(*(stringBitmap + i) == '1'){
			bitarray_set_bit(bitmap, i);
		}
		else{
			bitarray_clean_bit(bitmap, i);
		}
	}
}


// HECHA
void agregarBloquesOcupados(t_list* listaBloques, t_config* config){

	char** arrayBloques = config_get_array_value(config, "BLOCKS");
	t_list* listaAux = convertirEnLista(arrayBloques);
	list_add_all(listaBloques, listaAux);
	list_destroy(listaAux);
	liberarDoblesPunterosAChar(arrayBloques);
}


bool existeBitacoraTripulante(int idTripulante){
	char* stringIdTripulante = string_itoa(idTripulante);
	char* path = pathBitacoraTripulante(stringIdTripulante);
	bool result = verificarSiExiste(path);
	free(path);
	free(stringIdTripulante);
	return result;
}
