#include "bibliotecas.h"


int tamanioEstructura(void* estructura ,tipoDeDato cod_op){

	t_persona* persona;


	switch(cod_op){
		case PERSONA:
				persona = (void*) estructura;
				return  sizeof(uint32_t) * 3 + sizeof(uint8_t) + strlen(persona->nombre) + 1;


		default:
				printf("\n No pusiste el tipo de estructura para ver el tamanio negro \n");
				exit(1);
	}

}


void* serializarEstructura(void* estructura,int tamanio,tipoDeDato cod_op){

	t_persona* persona;

	void* stream = malloc(tamanio);

	int offset = 0;

	switch(cod_op){
		case PERSONA:
				persona = (void*) estructura;
				memcpy(stream + offset, &(persona->dni), sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(stream + offset, &(persona->edad), sizeof(uint8_t));
				offset += sizeof(uint8_t);
				memcpy(stream + offset, &(persona->pasaporte), sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(stream + offset, &(persona->nombre_length), sizeof(uint32_t));
				offset += sizeof(uint32_t);
				memcpy(stream + offset, persona->nombre, strlen(persona->nombre) + 1);

				return stream;

		default:
				printf("\n No pusiste el tipo de estructura para poder serializar negro \n");
				exit(1);
	}

}


t_paquete* armarPaqueteCon(void* estructura,tipoDeDato cod_op){

	t_paquete* paquete = crearPaquete(cod_op);
	paquete->buffer->size = tamanioEstructura(estructura,paquete->codigo_operacion);
	paquete->buffer->stream = serializarEstructura(estructura,paquete->buffer->size,paquete->codigo_operacion);

	return  paquete;

}


void enviarPaquete(t_paquete* paquete, int socket) {
	int tamanioTotal = paquete->buffer->size + sizeof(tipoDeDato) + sizeof(uint32_t);
	void* a_enviar = serializarPaquete(paquete,tamanioTotal);
	send(socket, a_enviar, tamanioTotal,
	0);

	free(a_enviar);
	//hay que tener cuidado, definir donde hacer free's y que seamos consitente en el tp
	eliminarPaquete(paquete);
}


t_paquete* recibirPaquete(int server_socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	recv(server_socket, &(paquete->codigo_operacion), sizeof(tipoDeDato), 0);

	recv(server_socket, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(server_socket, paquete->buffer->stream, paquete->buffer->size, 0);

	return paquete;
}


t_persona* deserializarPersona(t_buffer* buffer) {

	t_persona* persona = malloc(sizeof(t_persona));

	void* stream = buffer->stream;

	memcpy(&(persona->dni), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(persona->edad), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(persona->pasaporte), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(persona->nombre_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	persona->nombre = malloc(persona->nombre_length);
	memcpy(persona->nombre, stream, persona->nombre_length);

	return persona;
}


void* serializarPaquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}



void crearBuffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}


t_paquete* crearPaquete(tipoDeDato cod_op)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = cod_op;
	crearBuffer(paquete);
	return paquete;
}


void eliminarPaquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


void liberarConexion(int socket_cliente)
{
	close(socket_cliente);
}
