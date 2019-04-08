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
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>

							// ******* TIPOS NECESARIOS ******* //

typedef enum consistencias { 			// CONSISTENCIAS
	STRONG, STRONG_HASH, EVENTUAL
} t_consistencia;
typedef struct metadata { 				// METADATA DE TABLAS
	char* nombre;
	t_consistencia consistencia;
	int n_particiones;
} t_metadata;

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

t_log* iniciar_logger() { 								// CREACION DE LOG
	return log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
}

t_config* leer_config() {								// APERTURA DE CONFIG
	return config_create("kernel.config");
}
void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(config);
}


//				***** REVISAR COMO CREAR LAS CONEXIONES *****					//

int crear_conexion(char *ip, char* puerto)				// CREAR CONEXIONES - REVISAR
{
	struct addrinfo hints; // VER!
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}
					// ******* FIN DEFINICION DE FUNCIONES A UTILIZAR ******* //

#endif /* TP0_H_ */

