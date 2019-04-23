/*
 * LFS.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#ifndef LFS_H_
#define LFS_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>
#include <global/utils.h>
#include<global/protocolos.h>




#include <stdint.h>

#include <time.h>


typedef char* t_valor;	//valor que devuelve el select


// ******* DEFINICION DE FUNCIONES A UTILIZAR ******* //
void chequearSocket(int socketin);
void iniciar_logger(void);
void leer_config();
void terminar_programa(int conexion);
					// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;

typedef char* t_valor;	//valor que devuelve el select
int insert(char* tabla, uint16_t key, long timestamp);
t_valor select_(char* tabla, uint16_t key);
int drop(char* tabla);
int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);
void resolver_select (int socket_memoria);
//t_metadata describe(char* tabla);

#endif /* LFS_H_ */
