/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"
int main(void)
{
//	//char* MI_IP;
////	char* PUERTO;
//	//iniciar_logger(); // creamos log
//	//leer_config(); // abrimos config
////	PUERTO = 4444;
//	#define IP_MEMORIA	"127.0.0.2"
//	#define PUERTO	4444
//	int activado = 1;
//	int servidorMemoria = socket(AF_INET, SOCK_STREAM, 0);
//	setsockopt(servidorMemoria, SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado)); // PARA QUE SE PUEDAN REUSAR LAS ADDR Y NO QUEDEN BLOQUEADAS
//
//	// ---------------------- CONFIGURO SERVIDOR
//	struct sockaddr_in direccionMemoria; //creo direccion a la que se va a conectar el kernel
//	direccionMemoria.sin_family = AF_INET;
//	direccionMemoria.sin_addr.s_addr = inet_addr(IP_MEMORIA); // CUAL ES MI IP?
//	direccionMemoria.sin_port = htons(PUERTO); // EN QUE PUERTO VOY A RECIBIR CONEXIONES?
//
//	if( bind(servidorMemoria,(void*) &direccionMemoria, sizeof(direccionMemoria)) != 0) {
//		perror("Fallo el bind"); // ME PONGO A ESCUCHAR CONEXIONES ;)
//		return 1;
//	}
//
//	printf("Estoy escuchando\n"); // NO FALLO EL BIND ASI QUE YA PUEDO RECIBIR
//	listen(servidorMemoria, 10); // HASTA 10 CONEXIONES
//
//	// ---------------------- CONFIGURO CLIENTE
//
//	struct sockaddr_in direccionCliente;
//	unsigned int tamanioDireccion;
//	int cliente = accept(servidorMemoria, (void*) &direccionCliente, &tamanioDireccion); // ACEPTO CLIENTE Y CONECTO
//
//	printf("Recibi una conexion en %d", cliente);
	int conexionALFS;
	char* ip_memoria;
	char* puerto_memoria;
	char* ip_lfs;
	char* puerto_lfs;
	iniciar_logger(); // creamos log
	leer_config("asda"); // abrimos config

	ip_memoria = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_memoria );
	puerto_memoria = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_memoria);
	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG


	int server_fd = iniciar_servidor(ip_memoria, puerto_memoria);
	log_info(logger, "Memoria lista para recibir a peticiones de Kernel");
	log_info(logger, "Memoria espera peticiones");
	int socket_kernel_fd = esperar_cliente(server_fd);
	log_info(logger, "Memoria se conectó con Kernel");
	//conexionALFS = crear_conexion(ip_lfs, puerto_lfs); // --> VER <--

	while(1)
	{
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


	//terminar_programa(socket_kernel_fd, conexionALFS); // termina conexion, destroy log y destroy config.
	return EXIT_SUCCESS;
}


 void iniciar_logger() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
}

void leer_config(char * nombre_config) {								// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config");
}

void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
}
