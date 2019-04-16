/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"
int main(void)
{
	int conexionALFS;
	char* ip_memoria = string_new();
	char* puerto_memoria = string_new();
	char* ip__lfs = string_new();
	char* puerto__lfs =string_new();

	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	ip_memoria = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_memoria );
	puerto_memoria = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_memoria);
	ip__lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP del LFS es %s", ip__lfs);
	puerto__lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto del LFS es %s", puerto__lfs);
	int socket_conexion_lfs = crear_conexion(ip__lfs,puerto__lfs);
	log_info(logger,"Creada la conexion para LFS");
	char *mensaje = "Hola, me conecto, soy la memoria";
	log_info(logger, "Trato de realizar un hasdshake");
	if (enviar_handshake(socket_conexion_lfs,mensaje)){
		log_info(logger, "Se envió el mensaje %s", mensaje);

		recibir_handshake(logger, socket_conexion_lfs);
		log_info(logger,"Conexion exitosa con LFS");
	}


	int server_fd = iniciar_servidor(ip_memoria, puerto_memoria);
	log_info(logger, "Memoria lista para recibir a peticiones de Kernel");
	log_info(logger, "Memoria espera peticiones");
	int socket_kernel_fd = esperar_cliente(server_fd);
	log_info(logger, "Memoria se conectó con Kernel");
	// --------------//

	// conexion a lfs //
		//close(socket_conexion_lfs);
		//terminar_programa(socket_conexion_lfs); // termina conexion, destroy log y destroy config.




	//conexionALFS = crear_conexion(ip_lfs, puerto_lfs); // --> VER <--

	while(1){
		int cod_op = recibir_operacion(socket_kernel_fd);
		switch(cod_op)
		{
		case HANDSHAKE:
			log_info(logger, "Inicia handshake con Kernel");
			recibir_mensaje(logger, socket_kernel_fd);
			enviar_handshake(socket_kernel_fd, "OK");
			log_info(logger, "Conexion exitosa con kernel");
			break;
		case SELECT:
			log_info(logger, "Kernel solicitó SELECT");
			//aca debería enviarse el mensaje a LFS con SELECT
			break;
		case INSERT:
			log_info(logger, "Kernel solicitó INSERT");
			//aca debería enviarse el mensaje a LFS con INSERT
			break;
		case CREATE:
			log_info(logger, "Kernel solicitó CREATE");
			//aca debería enviarse el mensaje a LFS con CREATE
			break;
		case DESCRIBE:
			log_info(logger, "Kernel solicitó DESCRIBE");
			//aca debería enviarse el mensaje a LFS con DESCRIBE
			break;
		case DROP:
			log_info(logger, "Kernel solicitó DROP");
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
	terminar_programa(socket_kernel_fd, conexionALFS); // termina conexion, destroy log y destroy config.
	return EXIT_SUCCESS;
}

 void iniciar_logger() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
}

void leer_config() {				// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config");
}

void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
}
