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
char* leer_registros_temporales(char* path_tabla, int cantidad_temporales);
char* leer_registros_bloques(char* bloques, int size_total);
char* leer_registros_de_bloque(int bloque);
void realizar_compactacion(char* path_tabla, char* registros_filtrados);



#endif /* LFS_COMPACTACION_H_ */
