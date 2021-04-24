#include "discordiador.h"

int main(int argc, char *argv[]){

	struct sockaddr_in direccionServidor;

	t_config* ipDiscordiador = config_create("discordiador.config");

	char *IP = config_get_string_value(ipDiscordiador,"IP_MI_RAM_HQ");

	int PORT = config_get_int_value(ipDiscordiador,"PUERTO_MI_RAM_HQ");

	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(IP);
	direccionServidor.sin_port = htons((uint16_t)PORT);

	int cliente = socket(AF_INET,SOCK_STREAM,0);

	if(connect(cliente,(void*) &direccionServidor,sizeof(direccionServidor)) != 0){
		perror("No se pudo conectar");
		return 1;
	}

	while(1){
		char mensaje[1000];

		scanf("%s",mensaje);

		send(cliente,mensaje,strlen(mensaje + 1),0);
	}

	return 0;
}



