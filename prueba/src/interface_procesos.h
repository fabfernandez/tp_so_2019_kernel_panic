/*
 * interfaz_procesos.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */

#ifndef INTERFACE_PROCESOS_H_
#define INTERFACE_PROCESOS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>

typedef enum operaciones {
	INSERT,SELECT,CREATE,DESCRIBE,DROP
}t_operacion;

typedef enum consistencias{
	STRONG, STRONG_HASH, EVENTUAL
}t_consistencia;

typedef struct metadata{
	char* nombre;
	t_consistencia consistencia;
	int n_particiones;
}t_metadata;

typedef char* t_valor;	//valor que devuelve el select


int insert(char* tabla, uint16_t key, long timestamp);

t_valor select_(char* tabla, uint16_t key);

int drop(char* tabla);

int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);

t_metadata describe(char* tabla);


#endif /* INTERFACE_PROCESOS_H_ */
