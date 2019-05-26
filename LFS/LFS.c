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
	char *montaje;
	iniciar_logger(); // creamos log
	leer_config(); // abrimos config

	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_lfs );
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_lfs);
	montaje = config_get_string_value(archivoconfig, "PUNTO_MONTAJE");
	max_size_value = config_get_int_value(archivoconfig, "MAX_SIZE_VALUE");
	//crear_lfs(montaje);
	levantar_lfs(montaje);

	crear_hilo_consola();
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
		log_error(logger, "Error al frenar hilo");
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
	t_status_solicitud* status;
	switch((int)cod_op)
		{
		case HANDSHAKE:
			log_info(logger, "Inicia handshake con memoria");
			recibir_mensaje(logger, socket_memoria);
			enviar_handshake(socket_memoria, string_itoa(max_size_value));
			log_info(logger, "Conexion exitosa con memoria");
			break;
		case SELECT:
			log_info(logger, "memoria solicitó SELECT");
			t_paquete_select* select = deserializar_select(socket_memoria);
			status = resolver_select(select->nombre_tabla->palabra, select->key);
			enviar_status_resultado(status, socket_memoria);
			eliminar_paquete_select(select);
			break;
		case INSERT:
			log_info(logger, "memoria solicitó INSERT");
			t_paquete_insert* consulta_insert = deserealizar_insert(socket_memoria);
			status = resolver_insert(consulta_insert->nombre_tabla->palabra, consulta_insert->key, consulta_insert->valor->palabra, consulta_insert->timestamp);
			enviar_status_resultado(status, socket_memoria);
			eliminar_paquete_insert(consulta_insert);
			break;
		case CREATE:
			log_info(logger, "memoria solicitó CREATE");
			t_paquete_create* create = deserializar_create (socket_memoria);
			status = resolver_create(create->nombre_tabla->palabra, create->consistencia, create->num_particiones, create->tiempo_compac);
			enviar_status_resultado(status, socket_memoria);
			eliminar_paquete_create(create);
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

void crear_lfs(char* montaje){
	log_info(logger, "Inicia filesystem");
	path_montaje = malloc(string_size(montaje));
	memcpy(path_montaje, montaje,string_size(montaje) );
	obtener_info_metadata();
	//CREACION BITMAP PRUEBA

	int tamanioBitarray = blocks/8;
	if(blocks % 8 != 0){
	tamanioBitarray++;
	}
	char* bits=malloc(tamanioBitarray);
	t_bitarray * bitarray = bitarray_create_with_mode(bits,tamanioBitarray,MSB_FIRST);

	for(int cont=0; cont < tamanioBitarray*8; cont++){
		bitarray_clean_bit(bitarray, cont);
	}
	bitarray_set_bit(bitarray, 1);
	bitarray_set_bit(bitarray, 2);
	bitarray_set_bit(bitarray, 10);
	bitarray_set_bit(bitarray, 5);
	bitarray_set_bit(bitarray, 6);
}

void levantar_lfs(char* montaje){

	log_info(logger, "Inicia filesystem");
	path_montaje = malloc(string_size(montaje));
	memcpy(path_montaje, montaje,string_size(montaje) );

	obtener_info_metadata();
	obtener_bitmap();

}

void obtener_bitmap(){
	//TODO: Se lee el bitmap.bin, se lo mapea con un bitarray, tiene que ser global y tiene que estar inicializado.
}

void obtener_info_metadata(){

	char* path_metadata = string_from_format("%s/Metadata/Metadata.bin", path_montaje);
	t_config* metadata  = config_create(path_metadata);
	block_size = config_get_int_value(metadata, "BLOCK_SIZE");
	blocks = config_get_int_value(metadata, "BLOCKS");
	//Abrir carpeta metadata y leer los datos
}

t_status_solicitud* resolver_create (char* nombre_tabla, t_consistencia consistencia, int num_particiones, long compactacion){
	t_status_solicitud* status;
	log_info(logger, "Se realiza CREATE");
	log_info(logger, "Tabla: %s",nombre_tabla);
	log_info(logger, "Num Particiones: %d",num_particiones);
	log_info(logger, "Tiempo compactacion: %d", compactacion);
	log_info(logger, "Consistencia: %d", consistencia);
	return status;

}

void resolver_describe_drop (int socket_memoria, char* operacion){
	t_paquete_drop_describe* consulta_describe_drop = deserealizar_drop_describe(socket_memoria);
	log_info(logger, "Se realiza %s", operacion);
	log_info(logger, "Tabla: %s", consulta_describe_drop->nombre_tabla->palabra);
	eliminar_paquete_drop_describe(consulta_describe_drop);
}

t_status_solicitud*  resolver_insert(char* nombre_tabla, uint16_t key, char* value, long timestamp){

	t_status_solicitud* paquete_a_enviar;
	log_info(logger, "Se realiza INSERT");
	log_info(logger, "Consulta en la tabla: %s", nombre_tabla );
	log_info(logger, "Consulta por key: %d", key);
	log_info(logger, "Valor: %s", value);
	log_info(logger, "TIMESTAMP: %d",timestamp);
	if (existe_tabla(nombre_tabla)){
		//llenar los datos de consistencia, particion que estan en la metadata de la tabla (ingresar al directorio de la tabla) Metadata
		agregar_registro_memtable(crear_registro(value, key,  timestamp), nombre_tabla);
		paquete_a_enviar = crear_paquete_status(true, "OK");
	}else{
		char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
		log_error(logger, mje_error);
		paquete_a_enviar = crear_paquete_status(false, mje_error);
	}

	return paquete_a_enviar;

}

t_registro* crear_registro(char* value, uint16_t key, long timestamp){

	t_registro* nuevo_registro = malloc(sizeof(t_registro));
	nuevo_registro->key=key;
	nuevo_registro->timestamp = timestamp;
	nuevo_registro->value=malloc(string_size(value));
	memcpy(nuevo_registro->value, value, string_size(value));

	return nuevo_registro;
}

void agregar_registro_memtable(t_registro* registro_a_insertar, char * nombre_tabla){


	t_cache_tabla* tabla_cache = obtener_tabla_memtable(nombre_tabla);

	list_add(tabla_cache->registros, registro_a_insertar );

}

t_cache_tabla* obtener_tabla_memtable(char* nombre_tabla){

	if(memtable==NULL){
		memtable = list_create();
	}
	t_cache_tabla* tabla_cache = buscar_tabla_memtable(nombre_tabla);
	if(tabla_cache == NULL){
		tabla_cache = crear_tabla_cache(nombre_tabla);
		list_add(memtable, tabla_cache);
	}
	return tabla_cache;

}

t_cache_tabla* crear_tabla_cache(char* nombre_tabla){

	t_cache_tabla* nueva_tabla_cache = malloc(sizeof(t_cache_tabla));
	nueva_tabla_cache->nombre = malloc(string_size(nombre_tabla));
	memcpy(nueva_tabla_cache->nombre, nombre_tabla, string_size(nombre_tabla));
	nueva_tabla_cache->registros = list_create();
	return nueva_tabla_cache;
}


t_cache_tabla* buscar_tabla_memtable(char* nombre_tabla){
	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}

	return list_find(memtable, (void*) _es_tabla_con_nombre);
}

bool existe_tabla(char* nombre_tabla){
	char* path_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);

	struct stat sb;

	return (stat(path_tabla, &sb) == 0 && S_ISDIR(sb.st_mode));
}


t_status_solicitud* resolver_select (char* nombre_tabla, uint16_t key){
	t_status_solicitud* status;
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", nombre_tabla);
	log_info(logger, "Consulta por key: %d", key);
	if (existe_tabla(nombre_tabla)){
		t_registro* registro_buscado = buscar_registro_memtable(nombre_tabla, key);
		//TODO: buscar en archivos temporales y en bloques
		if(registro_buscado !=NULL){
			char* resultado = generar_registro_string(registro_buscado->timestamp, registro_buscado->key, registro_buscado->value);
			status = crear_paquete_status(true, resultado );

		}else{
			char * mje_error = string_from_format("No se encontró registro con key: %s en la tabla %s", key, nombre_tabla);
			log_error(logger, mje_error);
			status = crear_paquete_status(false, mje_error);
		}

	}else{
		char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
		log_error(logger, mje_error);
		status = crear_paquete_status(false, mje_error);
	}

	return status;

}


t_registro* buscar_registro_memtable(char* nombre_tabla, uint16_t key){

	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
			return string_equals_ignore_case(tabla->nombre, nombre_tabla);
		}
	int _es_registro_con_key(t_registro* registro){
		return registro->key== key;
	}

	t_registro* registro;
	t_cache_tabla* tabla_cache= list_find(memtable, (void*) _es_tabla_con_nombre);
	if(tabla_cache!=NULL){
		registro = list_find(tabla_cache->registros,(void*) _es_registro_con_key);
	}

	return registro;
}

 void iniciar_logger() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/lfs.log", "LFS", 1, LOG_LEVEL_INFO);
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
