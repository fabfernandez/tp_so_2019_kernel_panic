/*
 * LFS.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */
/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "LFS.h"
int main(void)
{
	char* ip_lfs;
	char* puerto_lfs;

	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_lfs );
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_lfs);


	int server_LFS = iniciar_servidor(ip_lfs, puerto_lfs);
	log_info(logger, "LFS listo para recibir a peticiones de memoria");
	log_info(logger, "LFS espera peticiones");
	int socket_memoria_entrante = esperar_cliente(server_LFS); // deberia esperar hilos-multiplexacion <-
	log_info(logger, "LFS se conectó con memoria");


	while(1)
	{
		int cod_op = recibir_operacion(socket_memoria_entrante);
		switch(cod_op)
		{
		case HANDSHAKE:
			log_info(logger, "Inicia handshake con memoria");
			recibir_mensaje(logger, socket_memoria_entrante);
			enviar_handshake(socket_memoria_entrante, "OK");
			log_info(logger, "Conexion exitosa con memoria");
			break;
		case SELECT:
			log_info(logger, "memoria solicitó SELECT");
			//aca debería enviarse el mensaje a LFS con SELECT
			break;
		case INSERT:
			log_info(logger, "memoria solicitó INSERT");
			//aca debería enviarse el mensaje a LFS con INSERT
			break;
		case CREATE:
			log_info(logger, "memoria solicitó CREATE");
			//aca debería enviarse el mensaje a LFS con CREATE
			break;
		case DESCRIBE:
			log_info(logger, "memoria solicitó DESCRIBE");
			//aca debería enviarse el mensaje a LFS con DESCRIBE
			break;
		case DROP:
			log_info(logger, "memoria solicitó DROP");
			//aca debería enviarse el mensaje a LFS con DROP
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida.");
			return EXIT_FAILURE;
			break;
		}
	}
	//terminar_programa(socket_memoria_entrante, conexionALFS); // termina conexion, destroy log y destroy config. ???
	return EXIT_SUCCESS;
}


 void iniciar_logger() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/lfs.log", "Memoria", 1, LOG_LEVEL_INFO);
}

void leer_config() {				// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/lfs.config");
}

/*void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
}
*/
