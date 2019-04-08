/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: LOS DINOS
 */

#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>
#include <Global/utils.h>
//#include "../Global/interface_procesos.h"


							// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;


typedef char* t_valor;					// VALOR QUE DEVUELVE EL SELECT(TODAVIA NO SABEMOS QUE ALMACENA EN TABLAS?)

						// ******* FIN VARIABLES NECESARIAS ******* //



							// ******* API KERNEL ******* //

int insert(char* tabla, uint16_t key, long timestamp); 	// INSERT PROTOTIPO (1)
t_valor select_(char* tabla, uint16_t key); 			// SELECT PROTOTIPO (2)
int drop(char* tabla);									// DROP PROTOTIPO	(3)
int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion); // CREATE PROTOTIPO (4)
t_metadata describe(char* tabla); 						// DESCRIBE PROTOTIPO (5)
int journal(void);										// JOURNAL PROTOTIPO(6)
int run(FILE* archivo);									// RUN PROTOTIPO	(7)
int add(int memoria, t_consistencia consistencia);		// ADD PROTOTIPO	(8)

							// ******* FIN API KERNEL ******* //



							// ******* DEFINICION DE FUNCIONES A UTILIZAR ******* //

void iniciar_logger(void);
void leer_config(void);
void terminar_programa(int conexion);


//				***** REVISAR COMO CREAR LAS CONEXIONES *****					//


					// ******* FIN DEFINICION DE FUNCIONES A UTILIZAR ******* //

#endif /* TP0_H_ */

