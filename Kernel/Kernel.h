/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: LOS DINOS
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <readline/readline.h>
#include <global/parser.h>
#include <global/utils.h>
#include <global/protocolos.h>
#include <stdio.h>
#include <stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include <stdint.h>
#include <time.h>

							// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;
t_queue* ready_queue;
t_queue* exec_queue;
t_queue* exit_queue;



typedef char* t_valor;					// VALOR QUE DEVUELVE EL SELECT(TODAVIA NO SABEMOS QUE ALMACENA EN TABLAS?)
typedef struct script{
	int id;
	char* path;
	FILE* readfrom;
	} t_script ;


typedef struct memoria{
		char* ip;
		char* puerto;
		uint16_t numero_memoria;
		int socket_memoria;
	} t_memoria ;

t_list* memorias_sin_asignar;

t_list* strong_consistency;
t_list* eventual_consistency;
t_list* strong_hash_consistency;




						// ******* FIN VARIABLES NECESARIAS ******* //


							// ******* API KERNEL ******* //
void resolver_describe_drop(t_instruccion_lql instruccion, int socket_memoria, t_operacion operacion);
void resolver_create(t_instruccion_lql instruccion, int socket_memoria);
void resolver_insert(t_instruccion_lql instruccion, int socket_memoria);
void resolver_select(t_instruccion_lql instruccion, int socket_memoria);
void resolver_run(t_instruccion_lql instruccion, int socket_memoria);

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
void chequearSocket(int socketin);
void iniciar_logger(void);
void leer_config(void);
void terminar_programa(int conexion);
int generarID();


//				***** REVISAR COMO CREAR LAS CONEXIONES *****					//


					// ******* FIN DEFINICION DE FUNCIONES A UTILIZAR ******* //

#endif /* TP0_H_ */

