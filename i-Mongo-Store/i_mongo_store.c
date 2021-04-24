#include "i_mongo_store.h"

#define TAMANIO_BUFFER 200
#define BACKLOG 10

int main(){
	char buffer[TAMANIO_BUFFER];
	int socketMiRAM,puerto;
		int yes=1;
		t_config* MiRAM_Config = config_create("i_mongo_store.config");
		puerto = config_get_int_value(MiRAM_Config,"PUERTO");


		struct sockaddr_in* direccionMiRAM = malloc(sizeof(struct sockaddr_in));
		direccionMiRAM->sin_addr.s_addr = inet_addr("127.0.0.1");
		direccionMiRAM->sin_port = htons(puerto);
		direccionMiRAM->sin_family = AF_INET;
		memset(direccionMiRAM->sin_zero,'\0',8);

		if ((socketMiRAM = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		 perror("socket");
		 exit(1);
		 }
		if (setsockopt(socketMiRAM,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(socketMiRAM, (struct sockaddr *) direccionMiRAM->sin_addr.s_addr, sizeof(struct sockaddr))== -1) {
			perror("bind");
			exit(1);
		 }
		if (listen(socketMiRAM, BACKLOG) == -1) {
			 perror("listen");
			 exit(1);
		}
		while(1){
		memset(buffer,'\0',TAMANIO_BUFFER);
		int rec = recv(socketMiRAM,buffer,sizeof(int),MSG_WAITALL);
			if(rec <= 0){
				if(rec == -1){
					perror("recv");
				}
				close(socketMiRAM);
				break;
			}
			printf("Recibi del buffer el siguiente mensaje: %s", buffer);
		}



	return 0;
}


