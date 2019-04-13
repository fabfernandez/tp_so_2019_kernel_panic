/*
 * parser.h
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#ifndef GLOBAL_PARSER_H_
#define GLOBAL_PARSER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <commons/string.h>
#include "utils.h"

typedef struct {
	bool valido;
	t_operacion operacion;
	union {
		struct {
			char* tabla;
			long timestamp;
			uint16_t key;
			char* value;
		} INSERT;
		struct {
			char* tabla;
			uint16_t key;
		} SELECT;
		struct {
			char* tabla;
			t_consistencia consistencia;
			uint16_t num_particiones;
			uint16_t compactacion_time;
		} CREATE;
		struct {
			char* tabla;
		} DESCRIBE;
		struct {
			char* tabla;
		} DROP;
		struct {
			int numero_memoria;
			t_consistencia consistencia;
		} ADD;
		struct {
			char * path_script;
		} RUN;

	} parametros;
	char** _raw; //Para uso de la liberación
} t_instruccion_lql;

t_instruccion_lql lanzar_error(char* mensaje);
t_instruccion_lql parsear_linea(char* line);
t_consistencia get_valor_consistencia(char* consistencia_ingresada);

#endif /* GLOBAL_PARSER_H_ */
