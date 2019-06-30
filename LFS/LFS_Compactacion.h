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
void realizar_compactacion(char* path_tabla, char* registros_filtrados);
void renombrar_archivos_para_compactar(char* path_tabla);


#endif /* LFS_COMPACTACION_H_ */
