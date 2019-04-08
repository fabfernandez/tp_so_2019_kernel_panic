/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"

int main(void)
{
	int conexionAMemoria;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	IP_MEMORIA = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	PUERTO_MEMORIA = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	conexionAMemoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA); // --> VER <--

	// enviar_mensaje("TESTINGGGGG", conexionAMemoria); // Mensaje de prueba

	while (1) {

	}
	terminar_programa(conexionAMemoria); // termina conexion, destroy log y destroy config.
}


void iniciar_logger() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.log", "kernel", 1, LOG_LEVEL_INFO);
}

void leer_config(char * nombre_config) {								// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.config");
}

void terminar_programa(int conexion)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(archivoconfig);
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
}


