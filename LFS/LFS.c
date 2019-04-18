/*
 * LFS.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */

#include "LFS.h"
int main(void)
{
	char* ip_lfs;
	char* puerto_lfs;
	fd_set master;    // lista maestra de file descriptor
	fd_set read_fds;  // lista temporal de file descriptor list para select()
	int fdmax;        // maxima cantidad de file descriptor
	int newfd;        // nuevo socket aceptado
	int fd_index;
	int cod_op;

	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_lfs );
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_lfs);


	int server_LFS = iniciar_servidor(ip_lfs, puerto_lfs);
	log_info(logger, "LFS listo para recibir a peticiones de memoria");
	log_info(logger, "LFS espera peticiones");
	// añadir listener al conjunto maestro
	FD_SET(server_LFS, &master);
	fdmax = server_LFS;

	for(;;) {
		read_fds = master;
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error(logger, "Fallo accion select.");
			exit(1);

		}else{
			// explorar conexiones existentes en busca de datos que leer
			for(fd_index = 0; fd_index <= fdmax; fd_index++) {
				if (FD_ISSET(fd_index, &read_fds)) { //entran nuevos datos
					if (fd_index == server_LFS) {
						//Nuevo cliente
						if (newfd = esperar_cliente(server_LFS) == -1) {
							log_error(logger, "No pudo aceptarse la conexion del cliente");
						} else {
							agregar_cliente(&master, newfd, &fdmax);
						}

					} else {
						//Cliente en el set, realizo solicitud
						cod_op = recibir_operacion(fd_index);
						resolver_operacion(cod_op);
					}
				}
			}

		}
	}

	//terminar_programa(socket_memoria_entrante, conexionALFS); // termina conexion, destroy log y destroy config. ???
	return EXIT_SUCCESS;
}

void agregar_cliente(fd_set* master, int cliente, int* fdmax){
	FD_SET(cliente, master);//se agrega nuevo fd al set
	log_info(logger, "LFS se conectó con memoria");
	if (cliente > *fdmax)//actualizar el máximo
		*fdmax = cliente;
	char *mensaje_handshake = "Conexion aceptada de LFS";
	if (enviar_handshake(cliente,mensaje_handshake)!=-1){
		log_info(logger, "Se envió el mensaje %s", mensaje_handshake);
		recibir_handshake(logger, cliente);
		log_info(logger,"Conexion exitosa con Memoria");
	}else{
		log_error(logger, "No pudo realizarse handshake con el nuevo cliente en el socket %d", cliente);
	}

}

void resolver_operacion(int socket_memoria, t_operacion cod_op){
	switch(cod_op)
		{
		case HANDSHAKE:
			log_info(logger, "Inicia handshake con memoria");
			recibir_mensaje(logger, socket_memoria);
			enviar_handshake(socket_memoria, "OK");
			log_info(logger, "Conexion exitosa con memoria");
			break;
		case SELECT:
			log_info(logger, "memoria solicitó SELECT");
			resolver_select(socket_memoria);
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
			break;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
}

void resolver_select (int socket_memoria){
	t_paquete_select* consulta_select = deserializar_select(socket_memoria);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);
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
