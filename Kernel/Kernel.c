/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "Kernel.h"

int main(void)
{
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	IP_MEMORIA = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s", IP_MEMORIA);
	PUERTO_MEMORIA = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s", PUERTO_MEMORIA);

//	// CREO SOCKET DESCRIPTOR KERNEL //
//
//	socketKernel = socket(AF_INET,SOCK_STREAM,0);
//
//	// SETEO SERVIDOR AL QUE ME VOY A CONECTAR
//
//	struct sockaddr_in destino_addr; // direccion a la que me voy a conectar(este caso memoria)
//	 destino_addr.sin_family = AF_INET; // por defecto aca va siempre AF_INET(ver beej)
//	 destino_addr.sin_port = htons(PUERTO_MEMORIA); // PUERTO EN EL QUE ESCUCHA LA MEMORIA -> PERO LO PASAMOS A ORDENACION DE RED
//	 destino_addr.sin_addr.s_addr = inet_addr(IP_MEMORIA); // IP DE LA MEMORIA
//	 memset(&(destino_addr.sin_zero), '\0', 8); // PONE EN CERO EL RESTO DE LA ESTRUCTURA(SIEMPRE, VER BEEJ)

	// CONECTO!
	int socket_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA); // conecto el socketKernel(ESTE PROCESO) con la memoria
	log_info(logger,"Creada la conexion para la memoria");
	char *mensaje = "Hola, me conecto, soy el Kernel";
	log_info(logger, "Trato de realizar un hasdshake");
	if (enviar_handshake(socket_memoria,mensaje)){
		log_info(logger, "Se envió el mensaje %s", mensaje);

		recibir_handshake(logger, socket_memoria);
		log_info(logger,"Conexion exitosa con Memoria");
	}
	while(1){
		char* linea = readline("Consola kernel>");
		t_instruccion_lql instruccion =parsear_linea(linea);
		ejecutar_instruccion(instruccion, socket_memoria);
		free(linea);
	}
	close(socket_memoria);
	terminar_programa(socket_memoria); // termina conexion, destroy log y destroy config.
}

void ejecutar_instruccion(t_instruccion_lql instruccion, int socket_memoria){
	t_operacion operacion = instruccion.operacion;
	switch(operacion) {
		case SELECT:
			log_info(logger, "Se solicita SELECT a memoria");
			resolver_select(instruccion, socket_memoria);
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
		case RUN:
			log_info(logger, "Kernel solicitó DROP");
			//aca debería enviarse el mensaje a LFS con DROP
			break;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
}

/*void chequearSocket(int socketin){
	if(socketin == -1){ printf("Error creacion de socket"); }
	else { printf("Socket creado exitosamente: %i", socketin);
			//exit(-1);
	}
};*/

void resolver_select(t_instruccion_lql instruccion, int socket_memoria){

	t_paquete_select* paquete_select = crear_paquete_select(instruccion);
	enviar_paquete_select(socket_memoria, paquete_select);
}

void iniciar_logger() { 							// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.log", "kernel", 1, LOG_LEVEL_INFO);
}

void leer_config() {								// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.config");
}

void terminar_programa(int conexion)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(archivoconfig);
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
}

