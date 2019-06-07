/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
//#include "Kernel.h"
#include "Kernel_Plani.h"

int ID=0;
t_list* memorias_sin_asignar;
int QUANTUM;
int socket_memoria;
int SLEEP_EJECUCION = 0;


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

	QUANTUM = config_get_int_value(archivoconfig, "QUANTUM");
	SLEEP_EJECUCION = config_get_int_value(archivoconfig, "SLEEP_EJECUCION") / 1000;

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
	socket_memoria = crear_conexion(IP_MEMORIA,PUERTO_MEMORIA); // conecto el socketKernel(ESTE PROCESO) con la memoria
	log_info(logger,"Creada la conexion para la memoria");
	char *mensaje = "Hola, me conecto, soy el Kernel";
	log_info(logger, "Trato de realizar un hasdshake");
	if (enviar_handshake(socket_memoria,mensaje)){
		log_info(logger, "Se envió el mensaje %s", mensaje);

		recibir_handshake(logger, socket_memoria);
		log_info(logger,"Conexion exitosa con Memoria");
	}

	t_memoria* memoria_principal = malloc(sizeof(t_memoria));
	memoria_principal->ip = malloc(string_size(IP_MEMORIA));
	memcpy(memoria_principal->ip, IP_MEMORIA, string_size(IP_MEMORIA));
	memoria_principal->puerto = malloc(string_size(PUERTO_MEMORIA));
	memcpy(memoria_principal->puerto, PUERTO_MEMORIA, string_size(PUERTO_MEMORIA));

	memoria_principal->numero_memoria= 1;
	memoria_principal->socket_memoria=socket_memoria;

	memorias_sin_asignar = list_create();
	strong_consistency = list_create();
	strong_hash_consistency = list_create();
	eventual_consistency = list_create();
	list_add(memorias_sin_asignar, memoria_principal);
	new_queue = queue_create();
	ready_queue = queue_create();
	exec_queue = queue_create();
	exit_queue = queue_create();

	iniciar_hilo_planificacion();


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
			resolver_describe_drop(instruccion, socket_memoria, DESCRIBE);
			break;
		case DROP:
			log_info(logger, "Kernel solicitó DROP");
			resolver_describe_drop(instruccion, socket_memoria, DROP);
			break;
		case RUN:
			log_info(logger, "Kernel solicitó RUN");
			resolver_run(instruccion, socket_memoria);
			break;
		case JOURNAL:
			log_info(logger, "Kernel solicitó JOURNAL");
			//aca resuelve el journal//
			break;
		case METRICS:
			log_info(logger, "Kernel solicitó METRICS");
			//aca resuelve metrics//
			break;
		case ADD:
			log_info(logger, "Kernel solicitó ADD");
			resolver_add(instruccion, socket_memoria);
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


void resolver_describe_drop(t_instruccion_lql instruccion, int socket_memoria, t_operacion operacion){
	//separar entre describe y drop
	t_paquete_drop_describe* paquete_describe = crear_paquete_drop_describe(instruccion);
	paquete_describe->codigo_operacion=operacion;
	enviar_paquete_drop_describe(socket_memoria, paquete_describe);
	//esperar numero de tblas si fue DESCRIBE
	//deserealizar_metadata(socket_memoria) en un for
	eliminar_paquete_drop_describe(paquete_describe);

}

void resolver_create(t_instruccion_lql instruccion, int socket_memoria){

	t_paquete_create* paquete_create = crear_paquete_create(instruccion);

	enviar_paquete_create(socket_memoria, paquete_create);
	t_status_solicitud* status = desearilizar_status_solicitud(socket_memoria);
	if(status->es_valido){
		log_info("Resultado: %s", status->mensaje->palabra);
	}else{
		log_error("Error: %s", status->mensaje->palabra);
	}

	eliminar_paquete_status(status);
	eliminar_paquete_create(paquete_create);
}

void resolver_select(t_instruccion_lql instruccion, int socket_memoria){

	t_paquete_select* paquete_select = crear_paquete_select(instruccion);
	enviar_paquete_select(socket_memoria, paquete_select);
	t_status_solicitud* status = desearilizar_status_solicitud(socket_memoria);
	if(status->es_valido){
		log_info("Resultado: %s", status->mensaje->palabra);
	}else{
		log_error("Error: %s", status->mensaje->palabra);
	}
	eliminar_paquete_status(status);
	eliminar_paquete_select(paquete_select);
}

void resolver_insert (t_instruccion_lql instruccion, int socket_memoria){
	t_paquete_insert* paquete_insert = crear_paquete_insert(instruccion);
	enviar_paquete_insert(socket_memoria, paquete_insert);
	eliminar_paquete_insert(paquete_insert);
}

void ejecutar_script(t_script* script_a_ejecutar){
	char* path = script_a_ejecutar->path;
	FILE* archivo = fopen(path,"r");
	fseek(archivo, script_a_ejecutar->offset, SEEK_SET);

	//int socket_memoria = memoria_apta();

	char ultimo_caracter_leido = leer_archivo(archivo, socket_memoria);

	if(ultimo_caracter_leido != EOF){
		script_a_ejecutar->offset = ftell(archivo) - 1;
	} else {
	script_a_ejecutar->offset = NULL;
	}

	fclose(archivo);
}



void resolver_run(t_instruccion_lql instruccion, int socket_memoria){
	char* path = instruccion.parametros.RUN.path_script;
	queue_push(new_queue,path);
}


void resolver_add (t_instruccion_lql instruccion, int socket_memoria){
	uint16_t numero_memoria = instruccion.parametros.ADD.numero_memoria;
	t_consistencia consistencia = instruccion.parametros.ADD.consistencia;

	if(consistencia == STRONG && list_size(strong_consistency)==1) {
		t_memoria* memoria_antigua = list_remove(strong_consistency, 0);
		list_add(memorias_sin_asignar, memoria_antigua);
	}

	int es_la_memoria(t_memoria* memoria){
		return memoria->numero_memoria == numero_memoria;
	}

	t_memoria* memoria = list_find(memorias_sin_asignar, (void*) es_la_memoria);
	if(memoria != NULL){
		asignar_consistencia(memoria, consistencia);
	}
}

void asignar_consistencia(t_memoria* memoria, t_consistencia consistencia){
	switch(consistencia){
		case STRONG:
			list_add(strong_consistency, memoria);
			break;
		case STRONG_HASH:
			list_add(strong_hash_consistency, memoria);
			break;
		case EVENTUAL:
			list_add(eventual_consistency, memoria);
			break;
		default:
			break;
	}
}

char leer_archivo(FILE* archivo, int socket_memoria){
	char* linea = NULL;
	int lineas_leidas = 0;
	int i;
	char letra;
	while((letra = fgetc(archivo)) != EOF && lineas_leidas < QUANTUM){
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
		printf("%s\n", linea);
		free(linea);
		lineas_leidas++;
		linea = NULL;
	}
	return letra;
}

void iniciar_logger() { 							// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.log", "kernel", 1, LOG_LEVEL_INFO);
}

void leer_config() {								// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.config");
}

int generarID(){
	ID++;
	return ID;
}


void terminar_programa(int conexion)
{
	liberar_conexion(conexion);
	log_destroy(logger);
	config_destroy(archivoconfig);
	//Y por ultimo, para cerrar, hay que liberar lo que utilizamos (conexion, log y config) con las funciones de las commons y del TP mencionadas en el enunciado
}

