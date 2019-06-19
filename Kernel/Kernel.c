/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
//#include "Kernel.h"
#include "Kernel_Metrics.h"

int ID=0;
t_list* memorias_sin_asignar;
int QUANTUM;
int socket_memoria;
int SLEEP_EJECUCION = 0;
int CANT_EXEC = 0;
int error = 0;
int CANT_METRICS = 0;


int main(void)
{
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;

	iniciar_logger(); // creamos log
	leer_config(); // abrimos config
	IP_MEMORIA = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	retardo_gossiping = config_get_int_value(archivoconfig, "RETARDO_GOSSIPING");

	log_info(logger, "La IP de la memoria es %s", IP_MEMORIA);
	PUERTO_MEMORIA = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s", PUERTO_MEMORIA);

	QUANTUM = config_get_int_value(archivoconfig, "QUANTUM");
	SLEEP_EJECUCION = config_get_int_value(archivoconfig, "SLEEP_EJECUCION") / 1000;
	CANT_EXEC = config_get_int_value(archivoconfig, "NIVEL_MULTIPROCESAMIENTO");

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

	tablaGossiping = list_create();
	memorias_sin_asignar = list_create();
	strong_consistency = list_create();
	strong_hash_consistency = list_create();
	eventual_consistency = list_create();
	list_add(memorias_sin_asignar, memoria_principal);
	new_queue = queue_create();
	ready_queue = queue_create();
	exec_queue = queue_create();
	exit_queue = queue_create();

	metricas = list_create();

	iniciar_hilo_planificacion();
	iniciar_hilo_metrics();
	iniciarHiloGossiping(tablaGossiping);

	while(1){
		char* linea = readline("Consola kernel>");

		parsear_y_ejecutar(linea, 1);

		free(linea);
	}
	close(socket_memoria);
	terminar_programa(socket_memoria); // termina conexion, destroy log y destroy config.
}

void iniciarHiloGossiping(t_list* tablaGossiping){ // @suppress("Type cannot be resolved")
	pthread_t hiloGossiping;
	if (pthread_create(&hiloGossiping, 0, iniciar_peticion_tablas, tablaGossiping) !=0){
			log_error(logger, "Error al crear el hilo");
		}
	if (pthread_detach(hiloGossiping) != 0){
			log_error(logger, "Error al crear el hilo");
		}
}

void* iniciar_peticion_tablas(void* tablaGossiping){
	t_list* tablag = (t_list *) tablaGossiping;
		while(1){
			sleep(retardo_gossiping);
			log_info(logger, "Inicio PEDIDO DE TABLAS A MEMORIA");
			// IMPLEMENTACION , ENVIAR UNA OPERACION A MEMORIA DE TABLA_GOSSIPING O COMO CARAJOS SEA Q CREE RECIEN EN EL PARSER
			log_info(logger, "FIN PEDIDO DE TABLAS");
		}
	}

void parsear_y_ejecutar(char* linea, int flag_de_consola){
	t_instruccion_lql instruccion = parsear_linea(linea);
	if (instruccion.valido) {
		ejecutar_instruccion(instruccion);
	}else{
		if (flag_de_consola){
			log_error(logger, "Reingrese correctamente la instruccion");
		}
	}
}

void ejecutar_instruccion(t_instruccion_lql instruccion){
	t_operacion operacion = instruccion.operacion;
	switch(operacion) {
		case SELECT:
			log_info(logger, "Se solicita SELECT a memoria");
			resolver_select(instruccion);
			break;
		case INSERT:
			log_info(logger, "Kernel solicitó INSERT");
			resolver_insert(instruccion);
			break;
		case CREATE:
			log_info(logger, "Kernel solicitó CREATE");
			resolver_create(instruccion);
			break;
		case DESCRIBE:
			log_info(logger, "Kernel solicitó DESCRIBE");
			resolver_describe(instruccion);
			break;
		case DROP:
			log_info(logger, "Kernel solicitó DROP");
			resolver_describe_drop(instruccion, DROP);
			break;
		case RUN:
			log_info(logger, "Kernel solicitó RUN");
			resolver_run(instruccion);
			break;
		case JOURNAL:
			log_info(logger, "Kernel solicitó JOURNAL");
			//aca resuelve el journal//
			//no olvidar registrar_metricas_operacion(1);
			break;
		case METRICS:
			log_info(logger, "Kernel solicitó METRICS");
			//aca resuelve metrics//
			break;
		case ADD:
			log_info(logger, "Kernel solicitó ADD");
			resolver_add(instruccion);
			break;
		default:
			log_warning(logger, "Operacion desconocida.");
			break;
		}
}

/*void chequearSocket(int socketin){
	if(socketin == -1){ printf("Error creacion de socket"); }
	else { print("Socket creado exitosamente: %i", socketin);
			//exit(-1);
	}
};*/


void resolver_describe_drop(t_instruccion_lql instruccion, t_operacion operacion){
	//separar entre describe y drop
	t_paquete_drop_describe* paquete_describe = crear_paquete_drop_describe(instruccion);
	paquete_describe->codigo_operacion=operacion;

	char* nombre_tabla = paquete_describe->nombre_tabla;
	int socket_memoria_a_usar = conseguir_memoria(nombre_tabla);

	enviar_paquete_drop_describe(socket_memoria_a_usar, paquete_describe);
	//esperar numero de tblas si fue DESCRIBE
	//deserealizar_metadata(socket_memoria) en un for

	eliminar_paquete_drop_describe(paquete_describe);

}

void resolver_describe(t_instruccion_lql instruccion){
	t_paquete_drop_describe* paquete_describe = crear_paquete_drop_describe(instruccion);
	paquete_describe->codigo_operacion=DESCRIBE;

	char* nombre_tabla = paquete_describe->nombre_tabla->palabra;
	int socket_memoria_a_usar = conseguir_memoria(nombre_tabla);
	enviar_paquete_drop_describe(socket_memoria_a_usar, paquete_describe);

	log_info(logger, "Se realiza DESCRIBE");
	if(string_is_empty(paquete_describe->nombre_tabla->palabra)){
		log_info(logger, "Se trata de un describe global.");
		int cant_tablas= recibir_numero_de_tablas (socket_memoria);
		log_info(logger, "Cantidad de tablas en LFS: %i", cant_tablas);
		for(int i=0; i<cant_tablas; i++){
			t_metadata* metadata = deserealizar_metadata(socket_memoria);
			log_info(logger, "Metadata tabla: %s", metadata->nombre_tabla->palabra);
			log_info(logger, "Consistencia: %s", consistencia_to_string(metadata->consistencia));
			log_info(logger, "Numero de particiones: %d", metadata->n_particiones);
			log_info(logger, "Tiempo de compactacion: %d", metadata->tiempo_compactacion);
		}
	}else{
		t_metadata* t_metadata = deserealizar_metadata(socket_memoria_a_usar);
		//aca se está mostrando pero deberia guardarselo no?
		log_info(logger, "Metadata tabla: %s", t_metadata->nombre_tabla->palabra);
		log_info(logger, "Consistencia: %s", consistencia_to_string(t_metadata->consistencia));
		log_info(logger, "Numero de particiones: %d", t_metadata->n_particiones);
		log_info(logger, "Tiempo de compactacion: %d", t_metadata->tiempo_compactacion);
	}
	eliminar_paquete_drop_describe(paquete_describe);
}

void resolver_create(t_instruccion_lql instruccion){
	t_paquete_create* paquete_create = crear_paquete_create(instruccion);

	char* nombre_tabla = paquete_create->nombre_tabla->palabra;
	int socket_memoria_a_usar = conseguir_memoria(nombre_tabla);

	enviar_paquete_create(socket_memoria_a_usar, paquete_create);
	t_status_solicitud* status = desearilizar_status_solicitud(socket_memoria_a_usar);
	if(status->es_valido){
		log_info(logger, "Resultado: %s", status->mensaje->palabra);
		error = 0;
	}else{
		log_error(logger, "Error: %s", status->mensaje->palabra);
		error = 1;
	}

	eliminar_paquete_status(status);
	eliminar_paquete_create(paquete_create);
}

void resolver_select(t_instruccion_lql instruccion){

	t_paquete_select* paquete_select = crear_paquete_select(instruccion);

	char* nombre_tabla = paquete_select->nombre_tabla->palabra;
	int socket_memoria_a_usar = conseguir_memoria(nombre_tabla);

	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	int timestamp_origen = spec.tv_sec;

	enviar_paquete_select(socket_memoria_a_usar, paquete_select);

	t_status_solicitud* status = desearilizar_status_solicitud(socket_memoria_a_usar);
	if(status->es_valido){
		log_info(logger, "Resultado: %s", status->mensaje->palabra);
		error = 0;
	}else{
		log_error(logger, "Error: %s", status->mensaje->palabra);
		error = 1;
	}
	clock_gettime(CLOCK_REALTIME, &spec);
	int timestamp_destino = spec.tv_sec;
	int diferencia_timestamp = timestamp_destino - timestamp_origen;

	registrar_metricas(1, diferencia_timestamp); //Ahora es 1 porque es la única memoria que conocemos, pero cambia a nombre_memoria

	eliminar_paquete_status(status);
	eliminar_paquete_select(paquete_select);
}

void resolver_insert (t_instruccion_lql instruccion){
	t_paquete_insert* paquete_insert = crear_paquete_insert(instruccion);

	char* nombre_tabla = paquete_insert->nombre_tabla->palabra;
	int socket_memoria_a_usar = conseguir_memoria(nombre_tabla);

	struct timespec spec;
		clock_gettime(CLOCK_REALTIME, &spec);
		int timestamp_origen = spec.tv_sec;

	enviar_paquete_insert(socket_memoria_a_usar, paquete_insert);

	clock_gettime(CLOCK_REALTIME, &spec);
		int timestamp_destino = spec.tv_sec;
		int diferencia_timestamp = timestamp_destino - timestamp_origen;

		registrar_metricas_insert(1, diferencia_timestamp);

	eliminar_paquete_insert(paquete_insert);
}



void ejecutar_script(t_script* script_a_ejecutar){
	char* path = script_a_ejecutar->path;
	FILE* archivo = fopen(path,"r");
	fseek(archivo, script_a_ejecutar->offset, SEEK_SET);

	char ultimo_caracter_leido = leer_archivo(archivo);

	if(ultimo_caracter_leido != EOF && error == 0){
		script_a_ejecutar->offset = ftell(archivo) - 1;
	} else {
		script_a_ejecutar->offset = NULL;
	}

	fclose(archivo);
}



void resolver_run(t_instruccion_lql instruccion){
	char* path = instruccion.parametros.RUN.path_script;
	queue_push(new_queue,path);
}


void resolver_add (t_instruccion_lql instruccion){
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
		log_info(logger, "Se ha añadido la Memoria: %d a la Consistencia: %c\n", numero_memoria, consistencia);
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

int conseguir_memoria(char *nombre_tabla){

	// DESCRIBE

	return socket_memoria;

}

char leer_archivo(FILE* archivo){
	char* linea = NULL;
	int lineas_leidas = 0;
	int i;
	char letra;
	while((letra = fgetc(archivo)) != EOF && lineas_leidas < QUANTUM && error != 1){
		linea = (char*)realloc(NULL, sizeof(char));
		i = 0;
		do{
			linea = (char*)realloc(linea, (i+1));
			linea[i] = letra;
			i++;
		}while((letra = fgetc(archivo)) != '\n' && letra != EOF);

		linea = (char*)realloc(linea, (i+1));
		linea[i] = 0;
		parsear_y_ejecutar(linea, 0);
		free(linea);
		lineas_leidas++;
		linea = NULL;
	}
	return letra;
}

void iniciar_logger() { 							// CREACION DE LOG
	logger = crear_log("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/kernel.log");
}

t_log* crear_log(char* path){
	return log_create(path, "kernel", 1, LOG_LEVEL_INFO);
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

