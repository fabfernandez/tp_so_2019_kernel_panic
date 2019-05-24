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
#include <sys/stat.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>
#include <global/utils.h>
#include<global/protocolos.h>

#include <stdbool.h>


#include <stdint.h>

#include <time.h>


t_list* memtable;


typedef struct{
	char* nombre;
	t_list* registros;
}t_cache_tabla;

// ******* DEFINICION DE FUNCIONES A UTILIZAR ******* //
void chequearSocket(int socketin);
void iniciar_logger(void);
void leer_config();
void terminar_programa(int conexion);
					// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;
char* path_montaje;
int  max_size_value, block_size, blocks;


void levantar_lfs(char* montaje);
void obtener_bitmap();
void obtener_info_metadata();
void resolver_create (int socket_memoria);
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
typedef char* t_valor;	//valor que devuelve el select
t_status_solicitud* resolver_insert(char* tabla, uint16_t key, char* value, long timestamp);
t_status_solicitud* resolver_select (char* nombre_tabla, uint16_t key);
int drop(char* tabla);
int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);
void resolver_describe_drop (int socket_memoria, char* operacion);
void resolver_create (int socket_memoria);
t_registro* crear_registro(char* value, uint16_t key, long timestamp);
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
t_cache_tabla* obtener_tabla_memtable(char* nombre_tabla);
t_cache_tabla* crear_tabla_cache(char* nombre_tabla);
t_cache_tabla* buscar_tabla_memtable(char* nombre_tabla);
t_registro* buscar_registro_memtable(char* nombre_tabla, uint16_t key);
bool existe_tabla(char* nombre_tabla);
//t_metadata describe(char* tabla);

#endif /* LFS_H_ */
