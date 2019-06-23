/*
 * LFS.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#ifndef LFS_H_
#define LFS_H_

#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <global/protocolos.h>
#include <global/utils.h>
#include <math.h>
#include <pthread.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>


#include "LFS_Consola.h"

t_list* memtable;
char* bmap;
t_bitarray* bitarray;
t_dictionary* temporales_por_tabla;

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
long tiempo_dump;


void levantar_lfs(char* montaje);
void obtener_bitmap();
void obtener_info_metadata();
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
typedef char* t_valor;	//valor que devuelve el select
t_status_solicitud* resolver_insert(char* tabla, uint16_t key, char* value, long timestamp);
t_status_solicitud* resolver_select (char* nombre_tabla, uint16_t key);
t_status_solicitud* resolver_create (char* nombre_tabla, t_consistencia consistencia, int num_particiones, long compactacion);
int drop(char* tabla);
int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);
void resolver_describe_drop (int socket_memoria, char* operacion);
void resolver_describe(int socket_memoria);
t_registro* crear_registro(char* value, uint16_t key, long timestamp);
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
t_cache_tabla* obtener_tabla_memtable(char* nombre_tabla);
t_cache_tabla* crear_tabla_cache(char* nombre_tabla);
t_cache_tabla* buscar_tabla_memtable(char* nombre_tabla);
t_registro* buscar_registro_memtable(char* nombre_tabla, uint16_t key);
bool existe_tabla_fisica(char* nombre_tabla);
void crear_hilo_memoria(int socket_memoria);
int resolver_operacion(int socket_memoria, t_operacion cod_op);
char* string_block();
char* array_int_to_array_char(t_list* array_int);
int obtener_bloque_disponible();
t_metadata* obtener_info_metadata_tabla(char* dir_tabla, char* nombre_tabla);
void crear_particiones(char* dir_tabla,int  num_particiones);
void crear_archivo(char* dir_archivo, int size, t_list* array_bloques);
void guardar_datos_particion_o_temp(char* dir_archivo , int size, t_list* array_bloques);
void crear_archivo_metadata_tabla(char* dir_tabla, int num_particiones,long compactacion,t_consistencia consistencia);
void crear_hilo_dump();
void dump_por_tabla(t_cache_tabla* tabla);
t_list* bajo_registros_a_blocks(int size_registros, char* registros);
void escribir_bloque(int bloque, char* datos);
int tamanio_bloque(int bloque_por_escribir, int bloques_totales, int size_datos);
int proximo_archivo_temporal_para(char* tabla);
void eliminar_registro(t_registro* registro);
void eliminar_tabla(t_cache_tabla* tabla_cache);

//t_metadata describe(char* tabla);

#endif /* LFS_H_ */
