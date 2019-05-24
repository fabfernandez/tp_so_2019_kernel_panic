/*
 * LFS_Consola.h
 *
 *  Created on: 24 may. 2019
 *      Author: utnso
 */

#ifndef LFS_CONSOLA_H_
#define LFS_CONSOLA_H_

#include <readline/readline.h>
#include<global/parser.h>
#include "LFS.h"


int resolver_operacion_por_consola(t_instruccion_lql instruccion);
void crear_hilo_consola();
void *levantar_consola();
void resolver_select_consola (char* nombre_tabla, uint16_t key);
#endif /* LFS_CONSOLA_H_ */
