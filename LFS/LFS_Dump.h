/*
 * LFS_Dump.h
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */

#ifndef LFS_DUMP_H_
#define LFS_DUMP_H_

#include "LFS.h"

void crear_hilo_dump();
t_list* copiar_y_limpiar_memtable();
void dump_por_tabla(t_cache_tabla* tabla);
t_list* bajo_registros_a_blocks(int size_registros, char* registros);
void escribir_bloque(int bloque, char* datos);
int tamanio_bloque(int bloque_por_escribir, int bloques_totales, int size_datos);
int proximo_archivo_temporal_para(char* tabla);
void eliminar_registro(t_registro* registro);
void eliminar_tabla(t_cache_tabla* tabla_cache);
int div_redondeada_a_mayor(int a, int b);

#endif /* LFS_DUMP_H_ */
