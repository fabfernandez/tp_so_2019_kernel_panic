/*
 * parser.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef GLOBAL_PARSER_H_
#define GLOBAL_PARSER_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <commons/string.h>
#include "protocolos.h"

t_instruccion_lql lanzar_error(char* mensaje);
t_instruccion_lql parsear_linea(char* line);
int check_consistencia(char* consistencia_ingresada);


#endif /* GLOBAL_PARSER_H_ */
