/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"
int main(void)
{
	// ------------------------------------------ INICIO LOGGER, CONFIG Y LEVANTO DATOS
	iniciar_logger();											//
	leer_config();												//
	levantar_datos_memoria();									//
	levantar_datos_lfs();										//
	// ------------------------------------------ ME CONECTO CON LFS E INTENTO UN HANDSHAKE
	socket_conexion_lfs = crear_conexion(ip__lfs,puerto__lfs);	//
	log_info(logger,"Creada la conexion para LFS");				//
	intentar_handshake_a_lfs(socket_conexion_lfs);				//
	// ------------------------------------------ INICIO SERVIDOR Y ESPERO CONEXION
	iniciar_servidor_memoria_y_esperar_conexiones_kernel();		//
	// ------------------------------------------ A ESPERA DE OPERACIONES
	esperar_operaciones(socket_kernel_conexion_entrante);		// -> ESPERO OPERACIONES DE KERNEL(ACA DEBERIA ESPERAR TAMBIEN DE LFS)

//	terminar_programa(socket_kernel_fd, conexionALFS); // termina conexion, destroy log y destroy config.
	return EXIT_SUCCESS;
}

void resolver_select (int socket_kernel_fd, int socket_conexion_lfs){
	t_paquete_select* consulta_select = deserializar_select(socket_kernel_fd);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);

	enviar_paquete_select(socket_conexion_lfs, consulta_select);
}
 void iniciar_logger() {
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
}
void leer_config() {
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config");
}
void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
}
int esperar_operaciones(int de_quien){
	while(1){
			int cod_op = recibir_operacion(de_quien);
			switch(cod_op)
			{
			case HANDSHAKE:
				log_info(logger, "Inicia handshake con Kernel");
				recibir_mensaje(logger, de_quien);
				enviar_handshake(de_quien, "OK");
				log_info(logger, "Conexion exitosa con kernel");
				break;
			case SELECT:
				log_info(logger, "Kernel solicitó SELECT");
				resolver_select(de_quien, socket_conexion_lfs);
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
}
void levantar_datos_memoria(){
	ip_memoria = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_memoria );
	puerto_memoria = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_memoria);

}
void levantar_datos_lfs(){
	ip__lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP del LFS es %s", ip__lfs);
	puerto__lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto del LFS es %s", puerto__lfs);
}
void intentar_handshake_a_lfs(int alguien){
	char *mensaje = "Hola, me conecto, soy la memoria";
		log_info(logger, "Trato de realizar un hasdshake");
		if (enviar_handshake(alguien,mensaje)){
			log_info(logger, "Se envió el mensaje %s", mensaje);

			recibir_handshake(logger, alguien);
			log_info(logger,"Conexion exitosa con LFS");
		}
}
void iniciar_servidor_memoria_y_esperar_conexiones_kernel(){
	server_memoria = iniciar_servidor(ip_memoria, puerto_memoria);
	log_info(logger, "Memoria lista para recibir a peticiones de Kernel");
	log_info(logger, "Memoria espera peticiones");
	socket_kernel_conexion_entrante = esperar_cliente(server_memoria);
	log_info(logger, "Memoria se conectó con Kernel");
}


// ANEXO //
/* CODIGO COMENTADO ENTRE iniciar_servidor_memoria_y_esperar_conexiones_kernel(); Y esperar_operaciones(socket_kernel_conexion_entrante);

	// conexion a lfs
		close(socket_conexion_lfs);
		terminar_programa(socket_conexion_lfs); // termina conexion, destroy log y destroy config.
	    conexionALFS = crear_conexion(ip_lfs, puerto_lfs); // --> VER <--

 FIN */
