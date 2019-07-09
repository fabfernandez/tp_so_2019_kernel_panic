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
#include "LFS_Dump.h"
t_list* memtable;
char* bmap;
t_bitarray* bitarray;


// ******* DEFINICION DE FUNCIONES A UTILIZAR ******* //
void chequearSocket(int socketin);
void iniciar_loggers();
void leer_config();
void terminar_programa();
					// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_log* logger_dump;
t_log* logger_compactacion;
t_log* logger_consola;
t_config* archivoconfig;
char* path_montaje;
int  max_size_value, block_size, blocks;
int tiempo_dump;
pthread_mutex_t mutexMemtable;
bool fin_de_programa;
pthread_t hilo_consola;
pthread_t hilo_dump;

t_list* leer_registros_de_bloque(int bloque, int bytes_a_leer);
int crear_directorio_tabla (char* dir_tabla);
void levantar_lfs(char* montaje);
void obtener_bitmap();
void obtener_info_metadata();
int obtener_cantidad_tablas_LFS();
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
typedef char* t_valor;	//valor que devuelve el select
t_registro* buscar_registro_actual(t_list* registros_encontrados);
t_list* buscar_registros_en_particion(char* nombre_tabla,uint16_t key);
t_list* buscar_registros_temporales(char* nombre_tabla, uint16_t key);
t_list* buscar_registros_con_key_en_archivo(char* path_archivo,uint16_t key);
t_status_solicitud* resolver_insert(char* tabla, uint16_t key, char* value, long timestamp);
t_status_solicitud* resolver_select (char* nombre_tabla, uint16_t key);
t_status_solicitud* resolver_create (char* nombre_tabla, t_consistencia consistencia, int num_particiones, long compactacion);
int drop(char* tabla);
void enviar_tabla_para_describe(int socket_memoria, char* nombre_tabla);
int obtener_cantidad_tablas_LFS();
void liberar_bloque(int num_bloque);
void enviar_metadata_todas_tablas (int socket_memoria);
int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);
t_status_solicitud* resolver_drop(char* nombre_tabla);
void resolver_describe(char* nombre_tabla, int socket_memoria);
t_registro* buscar_registro_actual(t_list* registros_encontrados);
t_registro* crear_registro(char* value, uint16_t key, long timestamp);
void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla);
bool validar_datos_describe(char* nombre_tabla, int socket_memoria);
t_cache_tabla* obtener_tabla_memtable(char* nombre_tabla);
t_cache_tabla* crear_tabla_cache(char* nombre_tabla);
t_cache_tabla* buscar_tabla_memtable(char* nombre_tabla);
t_list* buscar_registros_memtable(char* nombre_tabla, uint16_t key);
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
void iniciarMutexMemtable();

//t_metadata describe(char* tabla);

#endif /* LFS_H_ */
