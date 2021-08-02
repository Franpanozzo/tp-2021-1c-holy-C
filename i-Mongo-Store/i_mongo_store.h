#ifndef IMONGOSTORE_H_
#define IMONGOSTORE_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <bibliotecas.h>
#include <unistd.h>
#include <sys/select.h>
#include <memory.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/temporal.h>
#include <stdbool.h>
#include "variables.h"
#include "utils.h"
#include "sabotajes.h"
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>


void atenderTripulantes(int*);
int esperarTripulante(int);
void manejarTripulante(int*);
void deserializarSegun(t_paquete*);
void seleccionarTarea(t_tarea*);

void iniciarFileSystem();
void crearFileSystemDesdeCero();
void crearSuperBloque();
void crearFile(t_file2*);

void crearFileSystemExistente();
void cargarSuperBloque();
void cargarBlocks();
void cargarFile(t_file2*);


#endif

