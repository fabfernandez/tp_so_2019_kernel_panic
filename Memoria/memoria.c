/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"
int main(void)
{
	//char* MI_IP;
//	char* PUERTO;
	//iniciar_logger(); // creamos log
	//leer_config(); // abrimos config
//	PUERTO = 4444;
	#define IP_MEMORIA	"127.0.0.2"
	#define PUERTO	4444
	int activado = 1;
	int servidorMemoria = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(servidorMemoria, SOL_SOCKET,SO_REUSEADDR, &activado, sizeof(activado)); // PARA QUE SE PUEDAN REUSAR LAS ADDR Y NO QUEDEN BLOQUEADAS

	// ---------------------- CONFIGURO SERVIDOR
	struct sockaddr_in direccionMemoria; //creo direccion a la que se va a conectar el kernel
	direccionMemoria.sin_family = AF_INET;
	direccionMemoria.sin_addr.s_addr = inet_addr(IP_MEMORIA); // CUAL ES MI IP?
	direccionMemoria.sin_port = htons(PUERTO); // EN QUE PUERTO VOY A RECIBIR CONEXIONES?

	if( bind(servidorMemoria,(void*) &direccionMemoria, sizeof(direccionMemoria)) != 0) {
		perror("Fallo el bind"); // ME PONGO A ESCUCHAR CONEXIONES ;)
		return 1;
	}

	printf("Estoy escuchando\n"); // NO FALLO EL BIND ASI QUE YA PUEDO RECIBIR
	listen(servidorMemoria, 10); // HASTA 10 CONEXIONES

	// ---------------------- CONFIGURO CLIENTE

	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion;
	int cliente = accept(servidorMemoria, (void*) &direccionCliente, &tamanioDireccion); // ACEPTO CLIENTE Y CONECTO

	printf("Recibi una conexion en %d", cliente);
}


/* void iniciar_logger() { 								// CREACION DE LOG
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

*/
