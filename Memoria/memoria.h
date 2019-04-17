/*
 * memoria.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>
#include <global/utils.h>
#include <global/protocolos.h>


							// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;

typedef char* t_valor;	//valor que devuelve el select

void leer_config();
int insert(char* tabla, uint16_t key, long timestamp);

t_valor select_(char* tabla, uint16_t key);

int drop(char* tabla);

int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);

t_metadata describe(char* tabla);

int journal(void);

#endif /* MEMORIA_H_ */
