/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: LOS DINOS
 */

#ifndef KERNEL_H_
#define KERNEL_H_
#include <pthread.h>
#include <readline/readline.h>
#include <global/parser.h>
#include <global/utils.h>
#include <global/protocolos.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <stdint.h>
#include <time.h>

							// ******* TIPOS NECESARIOS ******* //
t_log* logger;
t_config* archivoconfig;



typedef char* t_valor;					// VALOR QUE DEVUELVE EL SELECT(TODAVIA NO SABEMOS QUE ALMACENA EN TABLAS?)



typedef struct memoria{
		char* ip;
		char* puerto;
		uint16_t numero_memoria;
		int socket_memoria;
} t_memoria ;

typedef struct script{
	int id;
	char* path;
	long int offset;
} t_script ;

t_list* memorias_sin_asignar;

t_list* strong_consistency;
t_list* eventual_consistency;
t_list* strong_hash_consistency;

int SLEEP_EJECUCION;




						// ******* FIN VARIABLES NECESARIAS ******* //


							// ******* API KERNEL ******* //
void resolver_describe_drop(t_instruccion_lql instruccion, t_operacion operacion);
void resolver_create(t_instruccion_lql instruccion);
void resolver_insert(t_instruccion_lql instruccion);
void resolver_select(t_instruccion_lql instruccion);
void resolver_run(t_instruccion_lql instruccion);
void resolver_add (t_instruccion_lql instruccion);

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
void asignar_consistencia(t_memoria* memoria, t_consistencia consistencia);
char leer_archivo(FILE* archivo);
void ejecutar_instruccion(t_instruccion_lql instruccion);
void parsear_y_ejecutar(char* linea, int flag_de_consola);
void ejecutar_script(t_script* script_a_ejecutar);
int conseguir_memoria(char* nombre_tabla);



//				***** REVISAR COMO CREAR LAS CONEXIONES *****					//


					// ******* FIN DEFINICION DE FUNCIONES A UTILIZAR ******* //

#endif /* TP0_H_ */

