/*
 * LFS_Compactacion.h
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */

#ifndef LFS_COMPACTACION_H_
#define LFS_COMPACTACION_H_

#include "LFS_Dump.h"
#include "LFS.h"

void crear_hilo_compactacion(char* nombre_tabla);
void *compactar(void* nombre_tabla);
long obtener_tiempo_compactacion(char* path_tabla);
bool hay_temporales(char* path_tabla);
int renombrar_archivos_para_compactar(char* path_tabla);
t_list* leer_registros_temporales(char* path_tabla, int cantidad_temporales);
t_list* leer_registros_particiones(char* path_tabla);
//t_list* leer_registros_bloques(char* bloques, int size_total);
//char* leer_registros_char_de_bloque(int bloque);
//t_list* transformar_registros(char* registros);
t_list* filtrar_registros_duplicados_segun_particiones(char* path_tabla, t_list* registros_nuevos);
void actualizar_registro(t_list* registros, t_registro* un_registro);
void realizar_compactacion(char* path_tabla, t_list* registros_filtrados);
void eliminar_temp_y_bin_tabla(char* path_tabla);


#endif /* LFS_COMPACTACION_H_ */
