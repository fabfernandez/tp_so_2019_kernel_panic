/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "Kernel.h"

int main(void)
{
	int conexionAMemoria;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	t_log* logger;
	t_config* config;

	logger = iniciar_logger(); // creamos log
	config = leer_config(); // abrimos config

	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG

	conexionAMemoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA); // --> VER <--

	// enviar_mensaje("TESTINGGGGG", conexionAMemoria); // Mensaje de prueba

	terminar_programa(conexionAMemoria,logger,config); // termina conexion, destroy log y destroy config.
}
