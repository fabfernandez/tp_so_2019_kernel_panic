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

		parsear_y_ejecutar(linea, socket_memoria, 1);

		free(linea);
	}
	close(socket_memoria);
	terminar_programa(socket_memoria); // termina conexion, destroy log y destroy config.
}

void parsear_y_ejecutar(char* linea, int socket_memoria, int flag_de_consola){
	t_instruccion_lql instruccion = parsear_linea(linea);
	if (instruccion.valido) {
		ejecutar_instruccion(instruccion, socket_memoria);
	}else{
		if (flag_de_consola){
			log_error(logger, "Reingrese correctamente la instruccion");
		}
	}
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
			resolver_insert(instruccion, socket_memoria);
			break;
		case CREATE:
			log_info(logger, "Kernel solicitó CREATE");
			resolver_create(instruccion, socket_memoria);
			break;
		case DESCRIBE:
			log_info(logger, "Kernel solicitó DESCRIBE");
			resolver_describe_drop(instruccion, socket_memoria);
			break;
		case DROP:
			log_info(logger, "Kernel solicitó DROP");
			resolver_describe_drop(instruccion, socket_memoria);
			break;
		case RUN:
			log_info(logger, "Kernel solicitó RUN");
			resolver_run(instruccion, socket_memoria);
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
void resolver_describe_drop(t_instruccion_lql instruccion, int socket_memoria){
	t_paquete_drop_describe* paquete_describe = crear_paquete_drop_describe(instruccion);
	enviar_paquete_drop_describe(socket_memoria, paquete_describe);
	eliminar_paquete_drop_describe(paquete_describe);

}

void resolver_create(t_instruccion_lql instruccion, int socket_memoria){

	t_paquete_create* paquete_create = crear_paquete_create(instruccion);
	enviar_paquete_create(socket_memoria, paquete_create);
	eliminar_paquete_create(paquete_create);
}

void resolver_select(t_instruccion_lql instruccion, int socket_memoria){

	t_paquete_select* paquete_select = crear_paquete_select(instruccion);
	enviar_paquete_select(socket_memoria, paquete_select);
	eliminar_paquete_select(paquete_select);
}

void resolver_insert (t_instruccion_lql instruccion, int socket_memoria){
	t_paquete_insert* paquete_insert = crear_paquete_insert(instruccion);
	enviar_paquete_insert(socket_memoria, paquete_insert);
	eliminar_paquete_insert(paquete_insert);
}

void resolver_run(t_instruccion_lql instruccion, int socket_memoria){
	char* path = instruccion.parametros.RUN.path_script;
	FILE *archivo;
	archivo = fopen(path,"r");

	leer_archivo(archivo, socket_memoria);

	fclose(archivo);
}

void leer_archivo(FILE* archivo, int socket_memoria){
	char* linea = NULL;
	int i;
	char letra;
	while((letra = fgetc(archivo)) != EOF){
		linea = (char*)realloc(NULL, sizeof(char));
		i = 0;
		do{
			linea = (char*)realloc(linea, (i+1));
			linea[i] = letra;
			i++;
		}while((letra = fgetc(archivo)) != '\n' && letra != EOF);

		linea = (char*)realloc(linea, (i+1));
		linea[i] = 0;
		parsear_y_ejecutar(linea, socket_memoria, 0);
		free(linea);
		linea = NULL;
	}
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

