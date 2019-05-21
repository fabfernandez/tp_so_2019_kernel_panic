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
	int socket_memoria;
	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_lfs );
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_lfs);

	//levantar_lfs();

	int server_LFS = iniciar_servidor(ip_lfs, puerto_lfs);

	while (1){
		if ((socket_memoria = esperar_cliente(server_LFS)) == -1) {
			//log_error(logger, "No pudo aceptarse la conexion del cliente");
		} else {
			crear_hilo_memoria(socket_memoria);
		}
	}
	//terminar_programa(socket_memoria_entrante, conexionALFS); // termina conexion, destroy log y destroy config. ???
	return EXIT_SUCCESS;
}


void *atender_pedido_memoria (void* memoria_fd){
	int socket_memoria = *((int *) memoria_fd);
	pthread_t id_hilo = pthread_self();
	while(1){
		int cod_op = recibir_operacion(socket_memoria);
		if (resolver_operacion(socket_memoria, cod_op)!=0){
			pthread_cancel(id_hilo);
		}
	}


	    //free(i);
}

void crear_hilo_memoria(int socket_memoria){
	pthread_t hilo_memoria;
	int *memoria_fd = malloc(sizeof(*memoria_fd));
	*memoria_fd = socket_memoria;
	if (pthread_create(&hilo_memoria, 0, atender_pedido_memoria, memoria_fd) !=0){
		log_error(logger, "Error al crear el hilo");
	}
	if (pthread_detach(hilo_memoria) != 0){
		log_error(logger, "Error al crear el hilo");
	}

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

int resolver_operacion(int socket_memoria, t_operacion cod_op){
	switch(cod_op)
		{
		case HANDSHAKE:
			log_info(logger, "Inicia handshake con memoria");
			recibir_mensaje(logger, socket_memoria);
			enviar_handshake(socket_memoria, "20");
			log_info(logger, "Conexion exitosa con memoria");
			break;
		case SELECT:
			log_info(logger, "memoria solicitó SELECT");
			resolver_select(socket_memoria);
			//aca debería enviarse el mensaje a LFS con SELECT
			break;
		case INSERT:
			log_info(logger, "memoria solicitó INSERT");
			resolver_insert(socket_memoria);
			//aca debería enviarse el mensaje a LFS con INSERT
			break;
		case CREATE:
			log_info(logger, "memoria solicitó CREATE");
			resolver_create(socket_memoria);
			//aca debería enviarse el mensaje a LFS con CREATE
			break;
		case DESCRIBE:
			log_info(logger, "memoria solicitó DESCRIBE");
			resolver_describe_drop(socket_memoria, "DESCRIBE");
			//aca debería enviarse el mensaje a LFS con DESCRIBE
			break;
		case DROP:
			log_info(logger, "memoria solicitó DROP");
			resolver_describe_drop(socket_memoria, "DROP");
			//aca debería enviarse el mensaje a LFS con DROP
			break;
		case -1:
			log_error(logger, "el cliente se desconecto.");
			return EXIT_FAILURE;

		default:
			log_warning(logger, "Operacion desconocida.");
			return EXIT_FAILURE;
		}
	return EXIT_SUCCESS;
}

void resolver_create (int socket_memoria){
	t_paquete_create* consulta_create = deserializar_create(socket_memoria);
	log_info(logger, "Se realiza CREATE");
	log_info(logger, "Tabla: %s", consulta_create->nombre_tabla->palabra);
	log_info(logger, "Num Particiones: %d", consulta_create->num_particiones);
	log_info(logger, "Tiempo compactacion: %d", consulta_create->tiempo_compac);
	log_info(logger, "Consistencia: %d", consulta_create->consistencia);
	enviar_paquete_create(socket_memoria, consulta_create);
	eliminar_paquete_create(consulta_create);
}

void resolver_describe_drop (int socket_memoria, char* operacion){
	t_paquete_drop_describe* consulta_describe_drop = deserealizar_drop_describe(socket_memoria);
	log_info(logger, "Se realiza %s", operacion);
	log_info(logger, "Tabla: %s", consulta_describe_drop->nombre_tabla->palabra);
	enviar_paquete_drop_describe(socket_memoria, consulta_describe_drop);
	eliminar_paquete_drop_describe(consulta_describe_drop);
}

void resolver_insert (int socket_memoria){
	t_paquete_insert* consulta_insert = deserealizar_insert(socket_memoria);
	char * nombre_tabla =  consulta_insert->nombre_tabla->palabra;
	uint16_t key = consulta_insert->key;
	log_info(logger, "Se realiza INSERT");
	log_info(logger, "Consulta en la tabla: %s", nombre_tabla );
	log_info(logger, "Consulta por key: %d", key);
/*	if (existe_tabla(nombre_tabla)){

	}*/
	eliminar_paquete_insert(consulta_insert);
}

void resolver_select (int socket_memoria){
	t_paquete_select* consulta_select = deserializar_select(socket_memoria);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);
	eliminar_paquete_select(consulta_select);
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
