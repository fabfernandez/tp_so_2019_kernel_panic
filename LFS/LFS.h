/*
 * LFS.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#ifndef LFS_H_
#define LFS_H_


#include <stdio.h>

#include <stdlib.h>

#include <commons/log.h>

#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>


#include <stdint.h>

#include <time.h>

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

t_valor SELECT(char* tabla, uint16_t key);

int drop(char* tabla);

int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);

t_metadata describe(char* tabla);

#endif /* LFS_H_ */
