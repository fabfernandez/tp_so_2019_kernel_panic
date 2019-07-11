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
	iniciar_loggers(); // creamos leer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivoleer_bloques_de_archivolog
	leer_config(); // abrimos config
	ip_lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_lfs );
	puerto_lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_lfs);
	montaje = config_get_string_value(archivoconfig, "PUNTO_MONTAJE");
	max_size_value = config_get_int_value(archivoconfig, "MAX_SIZE_VALUE");
	tiempo_dump = config_get_int_value(archivoconfig,"TIEMPO_DUMP");
	iniciarMutexMemtable();

	levantar_lfs(montaje);
	crear_hilo_consola();
	crear_hilo_dump();

	int server_LFS = iniciar_servidor(ip_lfs, puerto_lfs);
	while (!fin_de_programa){
		if ((socket_memoria = esperar_cliente(server_LFS)) == -1) {
			//log_error(logger, "No pudo aceptarse la conexion del cliente");
		} else {
			crear_hilo_memoria(socket_memoria);
		}
	}
	terminar_programa(); // termina conexion, destroy log y destroy config. ???
	return EXIT_SUCCESS;
}

void iniciarMutexMemtable(){
	if(pthread_mutex_init(&mutexMemtable,NULL)==0){
		log_info(logger, "MutexMemtable inicializado correctamente");
	} else {
		log_error(logger, "Fallo inicializacion de MutexMemtable");
	};
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
	//liberar memoria_f

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
			status = resolver_insert(logger, consulta_insert->nombre_tabla->palabra, consulta_insert->key, consulta_insert->valor->palabra, consulta_insert->timestamp);
			enviar_status_resultado(status, socket_memoria);
			eliminar_paquete_insert(consulta_insert);
			break;
		case CREATE:
			log_info(logger, "memoria solicitó CREATE");
			t_paquete_create* create = deserializar_create (socket_memoria);
			status = resolver_create(logger, create->nombre_tabla->palabra, create->consistencia, create->num_particiones, create->tiempo_compac);
			enviar_status_resultado(status, socket_memoria);
			//eliminar_paquete_status(status);
			eliminar_paquete_create(create);
			//aca debería enviarse el mensaje a LFS con CREATE
			break;
		case DESCRIBE:
			log_info(logger, "memoria solicitó DESCRIBE");
			t_paquete_drop_describe* consulta_describe = deserealizar_drop_describe(socket_memoria);
			if (validar_datos_describe(consulta_describe->nombre_tabla->palabra, socket_memoria)){
				resolver_describe(consulta_describe->nombre_tabla->palabra, socket_memoria);
			}
			eliminar_paquete_drop_describe(consulta_describe);
			//aca debería enviarse el mensaje a LFS con DESCRIBE
			break;
		case DROP:
			log_info(logger, "memoria solicitó DROP");
			t_paquete_drop_describe* consulta_drop = deserealizar_drop_describe(socket_memoria);
			status = resolver_drop(logger, consulta_drop->nombre_tabla->palabra);
			enviar_status_resultado(status, socket_memoria);
			eliminar_paquete_drop_describe(consulta_drop);
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

t_status_solicitud* resolver_drop(t_log* log_a_usar, char* nombre_tabla){

	t_status_solicitud* paquete_a_enviar;
	log_info(log_a_usar, "Se realiza DROP");
	log_info(log_a_usar, "Tabla: %s",nombre_tabla);
	char* dir_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
	if (existe_tabla_fisica(nombre_tabla)){
		eliminar_tabla_memtable(nombre_tabla);
		eliminar_tabla_logica(nombre_tabla);
		liberar_bloques_tabla(dir_tabla);
		eliminar_directorio(dir_tabla);
		paquete_a_enviar = crear_paquete_status(true, "OK");
	}else{
		char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
		log_error(log_a_usar, mje_error);
		paquete_a_enviar = crear_paquete_status(false, mje_error);
	}

	return paquete_a_enviar;
}
void liberar_bloques_tabla(char* path_tabla){

	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && ( archivo_es_del_tipo(entry->d_name,"temp") || archivo_es_del_tipo(entry->d_name,"tempc") || archivo_es_del_tipo(entry->d_name,"bin"))) {
			char* dir_archivo = string_from_format("%s/%s", path_tabla, entry->d_name);
			liberar_bloques_archivo(dir_archivo);
		}
		entry = readdir(dir);
	}
}

void liberar_bloques_archivo(char* path_archivo){
	t_config* archivo = config_create(path_archivo);
	int size_files = config_get_int_value(archivo, "SIZE");
	char **bloques = config_get_array_value(archivo, "BLOCKS");
	int ind_bloques=0;

	while(bloques[ind_bloques]!=NULL){
		liberar_bloque(atoi(bloques[ind_bloques]));
		ind_bloques = ind_bloques + 1;
	}

	config_destroy(archivo);

}

void eliminar_tabla_memtable(char* nombre_tabla){
	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}
	t_list* tabla_cache_eliminada = list_remove_by_condition(memtable, _es_tabla_con_nombre);
	if (tabla_cache_eliminada!=NULL){
		list_destroy(tabla_cache_eliminada);
	}
}

void eliminar_directorio(char* path_tabla){

	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) {
			char* dir_archivo = string_from_format("%s/%s", path_tabla, entry->d_name);
			if (unlink(dir_archivo) == 0)
				log_info(logger, "Eliminado archivo: %s\n", entry->d_name);
			else
				log_info(logger, "No se puede eliminar archivo: %s\n", entry->d_name);
		}
		entry = readdir(dir);
	}
	if (rmdir(path_tabla) == 0)
		log_info(logger, "Eliminado dir tabla: %s\n", path_tabla);
	else
		log_info(logger, "No se puede eliminar dir tabla: %s\n", path_tabla);

	closedir(dir);
}


bool validar_datos_describe(char* nombre_tabla, int socket_memoria){
	t_status_solicitud* status;
	bool es_valido = true;
	if(!string_is_empty(nombre_tabla)){
		if (existe_tabla_fisica(nombre_tabla)) {
			status = crear_paquete_status(true, "OK");
			enviar_status_resultado(status, socket_memoria);
		}else {
			char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
			log_error(logger, mje_error);
			status = crear_paquete_status(false, mje_error);
			enviar_status_resultado(status, socket_memoria);
			es_valido = false;
		}
	}
	return es_valido;
}

void levantar_lfs(char* montaje){

	log_info(logger, "Inicia filesystem");
	path_montaje = malloc(string_size(montaje));
	memcpy(path_montaje, montaje,string_size(montaje) );
	fin_de_programa=false;
	memtable = list_create();
	obtener_info_metadata();
	obtener_bitmap();
	obtener_tablas_en_lfs();

}
void obtener_tablas_en_lfs(){
	tablas_en_lfs = list_create();
	char* path_tablas = string_from_format("%s/Tables", path_montaje);
	DIR * dir = opendir(path_tablas);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (entry->d_type == DT_DIR &&  ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 )){
			char* nombre_tabla = entry->d_name;
			t_tabla_logica* tabla_fisica = crear_tabla_logica(nombre_tabla);
			list_add(tablas_en_lfs, tabla_fisica);
		}
		entry = readdir(dir);
	}

}

void agregar_tabla_logica(char* nombre_tabla){
	t_tabla_logica* tabla_fisica = crear_tabla_logica(nombre_tabla);
	list_add(tablas_en_lfs, tabla_fisica);
}

void eliminar_tabla_logica(char* nombre_tabla){
	bool _es_tabla_con_nombre(t_tabla_logica* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}

	t_tabla_logica* tabla_logica = list_remove_by_condition(tablas_en_lfs, _es_tabla_con_nombre);;
	if (tabla_logica!=NULL){
		log_info(logger_compactacion, "Se elimina la tabla %s. Se procede a frenar el hilo.", nombre_tabla);
//		if (pthread_detach(tabla_logica->id_hilo_compactacion) != 0){
//			log_error(logger_compactacion, "Error al frenar hilo de compactacion");
//		}
		free(tabla_logica->nombre);
		free(tabla_logica);
	}

}

t_tabla_logica* crear_tabla_logica(char* nombre_tabla){

	t_tabla_logica* tabla = malloc(sizeof(t_tabla_logica));
	tabla->esta_bloqueado=false;
	tabla->nombre=malloc(string_size(nombre_tabla));
	memcpy(tabla->nombre, nombre_tabla, string_size(nombre_tabla));

//	pthread_t hilo = crear_hilo_compactacion(nombre_tabla);
//	tabla->id_hilo_compactacion = hilo;
	return tabla;
}

void obtener_bitmap(){

	char* nombre_bitmap = string_from_format("%s/Metadata/Bitmap.bin", path_montaje);
	struct stat mystat;
	int fd = open(nombre_bitmap, O_RDWR);
    if (fstat(fd, &mystat) < 0) {
        printf("Error al establecer fstat\n");
        close(fd);
    }

    bmap = mmap(NULL, mystat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,	fd, 0);
	bitarray = bitarray_create_with_mode(bmap, blocks/8, MSB_FIRST);
}

t_metadata* obtener_info_metadata_tabla(char* dir_tabla, char* nombre_tabla){
	char* path_metadata_tabla = string_from_format("%s/Metadata", dir_tabla);

	t_config* metadata_tabla_config  = config_create(path_metadata_tabla);

	char* consistencia_string;
	long compactacion ;

	int num_particiones = config_get_int_value(metadata_tabla_config, "PARTITIONS");

	consistencia_string= config_get_string_value(metadata_tabla_config, "CONSISTENCY");

	compactacion =  config_get_long_value(metadata_tabla_config, "COMPACTION_TIME");

	t_metadata* metadata_tabla = crear_paquete_metadata(nombre_tabla, get_valor_consistencia(consistencia_string), num_particiones, compactacion);
	return metadata_tabla;

	// no hay que hacer un destroy de ese config ?
}

void obtener_info_metadata(){

	char* path_metadata = string_from_format("%s/Metadata/Metadata", path_montaje);
	t_config* metadata  = config_create(path_metadata);
	block_size = config_get_int_value(metadata, "BLOCK_SIZE");
	blocks = config_get_int_value(metadata, "BLOCKS");

	// no hay que hacer un destroy de ese config ?
}

t_status_solicitud* resolver_create (t_log* log_a_usar,char* nombre_tabla, t_consistencia consistencia, int num_particiones, long compactacion){

	/*
	 * Verificar que la tabla no exista en el file system. Por convención, una tabla existe si ya hay
		otra con el mismo nombre. Para dichos nombres de las tablas siempre tomaremos sus
		valores en UPPERCASE (mayúsculas). En caso que exista, se guardará el resultado en un
		archivo .log y se retorna un error indicando dicho resultado.
		2. Crear el directorio para dicha tabla.
		3. Crear el archivo Metadata asociado al mismo.
		4. Grabar en dicho archivo los parámetros pasados por el request.
		5. Crear los archivos binarios asociados a cada partición de la tabla y asignar a cada uno un
		bloque
	 * */
	t_status_solicitud* status;
	log_info(log_a_usar, "Se realiza CREATE");
	log_info(log_a_usar, "Tabla: %s",nombre_tabla);
	log_info(log_a_usar, "Num Particiones: %d",num_particiones);
	log_info(log_a_usar, "Tiempo compactacion: %d", compactacion);
	log_info(log_a_usar, "Consistencia: %d", consistencia);

	if(existe_tabla_fisica(nombre_tabla)){
		char * mje_error = string_from_format("La tabla %s ya existe", nombre_tabla);
		log_error(log_a_usar, mje_error);
		status = crear_paquete_status(false, mje_error);
	}else{
		char* dir_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
		if (crear_directorio_tabla(dir_tabla)){
			crear_archivo_metadata_tabla(dir_tabla, num_particiones, compactacion, consistencia);
			crear_particiones(dir_tabla, num_particiones);
			agregar_tabla_logica(nombre_tabla);
			status = crear_paquete_status(true, "OK");
		}else{
			char * mje_error = string_from_format("No pudo crearse la tabla %s", nombre_tabla);
			log_error(log_a_usar, mje_error);
			status = crear_paquete_status(false, mje_error);
		}

	}

	return status;

}

void crear_particiones(char* dir_tabla,int  num_particiones){

	int ind;
	for(ind =0;ind < num_particiones; ind++){
		char* dir_particion = string_from_format("%s/%i.bin", dir_tabla, ind);
		t_list* bloques = list_create();
		int num_bloque= obtener_bloque_disponible();
		list_add(bloques, num_bloque);   //paso la direccion de memoria de num_bloque, que adentro tiene el bloque disponible

		crear_archivo(dir_particion, 0, bloques);
		free(dir_particion);
		list_destroy(bloques);
	}
}

void crear_archivo(char* dir_archivo, int size, t_list* array_bloques){
	FILE* file = fopen(dir_archivo, "wb+");
	fclose(file);

	guardar_datos_particion_o_temp(dir_archivo, size, array_bloques);
}

void guardar_datos_particion_o_temp(char* dir_archivo, int size, t_list* array_bloques){
	char * array_bloques_string = array_int_to_array_char(array_bloques);
	char * char_size = string_itoa(size);

	t_config* particion_tabla = config_create(dir_archivo);
	dictionary_put(particion_tabla->properties,"SIZE", char_size);
	dictionary_put(particion_tabla->properties, "BLOCKS", array_bloques_string);

	config_save(particion_tabla);

	//free(array_bloques_string);
	//free(char_size);
	config_destroy(particion_tabla);
}

char* array_int_to_array_char(t_list* array_int){
	char * array_char = string_new();
	string_append(&array_char, "[");

	void _agregar_como_string(int* valor){
		string_append(&array_char, string_itoa((int)valor));
		string_append(&array_char, ",");
	}

	list_iterate(array_int, (void*)_agregar_como_string);

	char* array_char_sin_ultima_coma = string_substring_until(array_char, string_length(array_char) -1);
	string_append(&array_char_sin_ultima_coma,"]");
	return array_char_sin_ultima_coma;
}

void liberar_bloque(int num_bloque){

	char* dir_bloque = string_from_format("%s/Bloques/%d.bin", path_montaje, num_bloque);
	FILE* fpFile = fopen(dir_bloque,"wb");
	fclose(fpFile);
	truncate(dir_bloque, block_size);
	bitarray_clean_bit(bitarray, num_bloque);
	msync(bmap, sizeof(bitarray), MS_SYNC);
}

int obtener_bloque_disponible(){

	bool esta_ocupado=true;
	int nro_bloque=0;
	while(esta_ocupado == true){
		esta_ocupado = bitarray_test_bit(bitarray, nro_bloque);
		nro_bloque=nro_bloque+1;
	}
	nro_bloque=nro_bloque-1;
	bitarray_set_bit(bitarray,nro_bloque);
	msync(bmap, sizeof(bitarray), MS_SYNC);

	return nro_bloque;

}

void crear_archivo_metadata_tabla(char* dir_tabla, int num_particiones,long compactacion,t_consistencia consistencia){

	char* dir_metadata_tabla = string_from_format("%s/Metadata", dir_tabla);
	FILE* file = fopen(dir_metadata_tabla, "wb+");
	fclose(file);

	t_config* metadata_tabla = config_create(dir_metadata_tabla);
	dictionary_put(metadata_tabla->properties,"CONSISTENCY", consistencia_to_string(consistencia) );
	dictionary_put(metadata_tabla->properties, "PARTITIONS", string_itoa(num_particiones));
	dictionary_put(metadata_tabla->properties, "COMPACTION_TIME", string_itoa(compactacion));

	config_save(metadata_tabla);
}


int crear_directorio_tabla (char* dir_tabla){

	return !(mkdir(dir_tabla, 0777) != 0 && errno != EEXIST);
}

void resolver_describe(char* nombre_tabla, int socket_memoria){
	log_info(logger, "Se realiza DESCRIBE");
	if(string_is_empty(nombre_tabla)){
		int cant_tablas = obtener_cantidad_tablas_LFS();
		enviar_numero_de_tablas(socket_memoria, cant_tablas);
		log_info(logger, "Se trata de un describe global.");
		enviar_metadata_todas_tablas(socket_memoria);
	}else{
		enviar_tabla_para_describe(socket_memoria, nombre_tabla);
	}
}

int obtener_cantidad_tablas_LFS(){
	int cant_tablas = 0;
	char* path_tablas = string_from_format("%s/Tables", path_montaje);
	DIR * dir = opendir(path_tablas);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (entry->d_type == DT_DIR &&  ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 )) {
			log_info(logger,"Nombre: %s", entry->d_name);
			cant_tablas = cant_tablas + 1;
		}
		entry = readdir(dir);
	}
	closedir(dir);
	return cant_tablas;
}

void enviar_metadata_todas_tablas (int socket_memoria){
	char* path_tablas = string_from_format("%s/Tables", path_montaje);
	DIR * dir = opendir(path_tablas);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (entry->d_type == DT_DIR &&  ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 )){
			char* nombre_tabla = entry->d_name;
			enviar_tabla_para_describe(socket_memoria, nombre_tabla);
		}
		entry = readdir(dir);
	}
}

void enviar_tabla_para_describe(int socket_memoria, char* nombre_tabla){
	log_info(logger, "Tabla: %s", nombre_tabla);
	char* dir_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
	t_metadata* metadata_tabla = obtener_info_metadata_tabla(dir_tabla, nombre_tabla);
	enviar_paquete_metadata(socket_memoria, metadata_tabla);
	log_info(logger, "Metadata tabla: %s", metadata_tabla->nombre_tabla->palabra);
	log_info(logger, "Consistencia: %s", consistencia_to_string(metadata_tabla->consistencia));
	log_info(logger, "Numero de particiones: %d", metadata_tabla->n_particiones);
	log_info(logger, "Tiempo de compactacion: %d", metadata_tabla->tiempo_compactacion);

}

void resolver_describe_drop (int socket_memoria, char* operacion){
	t_paquete_drop_describe* consulta_describe_drop = deserealizar_drop_describe(socket_memoria);
	log_info(logger, "Se realiza %s", operacion);
	log_info(logger, "Tabla: %s", consulta_describe_drop->nombre_tabla->palabra);
	eliminar_paquete_drop_describe(consulta_describe_drop);
}

t_status_solicitud*  resolver_insert(t_log* log_a_usar, char* nombre_tabla, uint16_t key, char* value, long timestamp){

	t_status_solicitud* paquete_a_enviar;
	log_info(log_a_usar, "Se realiza INSERT");
	log_info(log_a_usar, "Consulta en la tabla: %s", nombre_tabla );
	log_info(log_a_usar, "Consulta por key: %d", key);
	log_info(log_a_usar, "Valor: %s", value);
	log_info(log_a_usar, "TIMESTAMP: %d",timestamp);
	if (existe_tabla_fisica(nombre_tabla)){
		//llenar los datos de consistencia, particion que estan en la metadata de la tabla (ingresar al directorio de la tabla) Metadata
		agregar_registro_memtable(crear_registro(value, key,  timestamp), nombre_tabla);
		paquete_a_enviar = crear_paquete_status(true, "OK");
	}else{
		char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
		log_error(log_a_usar, mje_error);
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
	bool _es_tabla_con_nombre(t_cache_tabla* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}

	return list_find(memtable, (void*) _es_tabla_con_nombre);
}

bool existe_tabla_fisica(char* nombre_tabla){
//	char* path_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
//
//	struct stat sb;
//
//	return (stat(path_tabla, &sb) == 0 && S_ISDIR(sb.st_mode));
	bool _es_tabla_con_nombre(t_tabla_logica* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}


	return list_any_satisfy(tablas_en_lfs, _es_tabla_con_nombre);
}


t_status_solicitud* resolver_select (char* nombre_tabla, uint16_t key){
	t_status_solicitud* status;
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", nombre_tabla);
	log_info(logger, "Consulta por key: %d", key);
	t_list* registros_encontrados = list_create();
	if (existe_tabla_fisica(nombre_tabla)){
		t_list* registros_memtable = buscar_registros_memtable(nombre_tabla, key);
		t_list* registros_temporales = buscar_registros_temporales(nombre_tabla, key);
		t_list* registros_particion = buscar_registros_en_particion(nombre_tabla, key);

		list_add_all(registros_encontrados, registros_memtable);
		list_add_all(registros_encontrados, registros_temporales);
		list_add_all(registros_encontrados, registros_particion);
		t_registro* registro_buscado = buscar_registro_actual(registros_encontrados);
		if(registro_buscado !=NULL){
			char* resultado = generar_registro_string(registro_buscado->timestamp, registro_buscado->key, registro_buscado->value);
			status = crear_paquete_status(true, resultado );

		}else{
			char * mje_error = string_from_format("No se encontró registro con key: %d en la tabla %s", key, nombre_tabla);
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
t_list* buscar_registros_en_particion(char* nombre_tabla,uint16_t key){

	char* path_metadata = string_from_format("%s/Tables/%s/Metadata", path_montaje, nombre_tabla);
	t_config* metadata = config_create(path_metadata);
	int num_particiones = config_get_int_value(metadata, "PARTITIONS");
	t_list* registros_encontrados = list_create();
	int particion = key % num_particiones;
	char* path_particion = string_from_format("%s/Tables/%s/%d.bin", path_montaje, nombre_tabla, particion);
	t_list* registros_en_particion = obtener_registros_de_archivo(path_particion);
	t_list* registros_con_key=	filtrar_registros_con_key(registros_en_particion, key);
	list_add_all(registros_encontrados, registros_con_key);
	config_destroy(metadata);
	return registros_encontrados;
}

t_list* buscar_registros_temporales(char* nombre_tabla, uint16_t key){

	char* path_tablas = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
	t_list* registros_encontrados = list_create();
	DIR * dir = opendir(path_tablas);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && (archivo_es_del_tipo(entry->d_name,"temp") || archivo_es_del_tipo(entry->d_name,"tempc"))){
			char* path_archivo_temporal = string_from_format("%s/%s", path_tablas, entry->d_name);
			t_list* registros_en_temporal = obtener_registros_de_archivo(path_archivo_temporal);
			t_list* registros_con_key=	filtrar_registros_con_key(registros_en_temporal, key);
			list_add_all(registros_encontrados, registros_con_key);
		}
		entry = readdir(dir);
	}
	return registros_encontrados;
}

t_list* filtrar_registros_con_key(t_list* registros, uint16_t key){

	bool _es_registro_con_key(t_registro* registro){
		return registro->key== key;
	}
	return  list_filter(registros, _es_registro_con_key);
}

t_list* obtener_registros_de_archivo(char* path_archivo_temporal){

	char* buffer_registros = leer_bloques_de_archivo(path_archivo_temporal);
	return obtener_registros_de_buffer(buffer_registros);
}

char* leer_bloques_de_archivo(char* path_archivo){
	t_config* archivo = config_create(path_archivo);
	int size_files = config_get_int_value(archivo, "SIZE");
	char **bloques = config_get_array_value(archivo, "BLOCKS");
	int resto_a_leer = size_files;
	int size_a_leer;
	int ind_bloques=0;
	char* buffer;
	char* buffer_bloques = string_new();

	if (size_files!=0){
		while(bloques[ind_bloques]!=NULL){
			if (resto_a_leer-block_size > 0){
				size_a_leer = block_size;
				resto_a_leer=resto_a_leer-block_size;
			}else{
				size_a_leer = resto_a_leer;
			}

			int num_bloque=atoi(bloques[ind_bloques]);

			char* dir_bloque = string_from_format("%s/Bloques/%i.bin", path_montaje, num_bloque);
			FILE* file = fopen(dir_bloque, "rb+");
//			fseek(file, 0, SEEK_END);
//			// int tamanioArchivo = sizeof(char) * ftell(fp);
//			long tamanioArchivo = ftell(file);
//			fseek(file, 0, SEEK_SET);
			buffer = malloc(size_a_leer);
			size_t read_count = fread(buffer, sizeof(char), size_a_leer, file);
			buffer[read_count]='\0';
			string_append(&buffer_bloques, buffer);
			fclose(file);
			free(buffer);
			free(dir_bloque);
			ind_bloques = ind_bloques+1;
		}
	}
	//buffer_bloques[size_files]="\0";
	config_destroy(archivo);
	return buffer_bloques;

}

t_list* obtener_registros_de_buffer(char* buffer){
	t_list* registros_de_bloques = list_create();
	char **array_buffer_registro = string_split(buffer, "\n");
	int ind_registros=0;
	while (array_buffer_registro[ind_registros]!=NULL){
		char** string_registro = string_split(array_buffer_registro[ind_registros], ";");

		uint16_t key = (uint16_t) atol(string_registro[1]);
		long timestamp = (long)atol(string_registro[0]);
		t_registro* registro= crear_registro(string_registro[2], key, timestamp);
		list_add(registros_de_bloques, registro);

		ind_registros = ind_registros+1;
	}

	return registros_de_bloques;

}

t_list* buscar_registros_memtable(char* nombre_tabla, uint16_t key){

	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
			return string_equals_ignore_case(tabla->nombre, nombre_tabla);
		}
	int _es_registro_con_key(t_registro* registro){
		return registro->key== key;
	}

	t_list* registros_encontrados = list_create();
	t_cache_tabla* tabla_cache= list_find(memtable, (void*) _es_tabla_con_nombre);
	if(tabla_cache!=NULL){
		list_filter(tabla_cache->registros,(void*) _es_registro_con_key);
	}

	return registros_encontrados;
}

t_registro* buscar_registro_actual(t_list* registros_encontrados){
	t_registro* registro_actual = NULL;
	int _registro_mayor_por_timestamp(t_registro* registroA, t_registro* registroB){
			return registroA->timestamp>registroB->timestamp;
	}
	if (!list_is_empty(registros_encontrados)){
		 list_sort(registros_encontrados, _registro_mayor_por_timestamp);
		 registro_actual= list_get(registros_encontrados,0);
	}
	return registro_actual;
}

 void iniciar_loggers() { 								// CREACION DE LOG
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/logs/lfs.log", "LFS", 0, LOG_LEVEL_INFO);
	logger_dump = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/logs/dump_lfs.log", "LFS", 0, LOG_LEVEL_INFO);
	logger_compactacion = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/logs/compactacion_lfs.log", "LFS", 0, LOG_LEVEL_INFO);
	logger_consola = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/logs/consola_lfs.log", "LFS", 1, LOG_LEVEL_INFO);
}

void leer_config() {				// APERTURA DE CONFIG
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/LFS/lfs.config");
}
void terminar_programa(){
	log_destroy(logger);
	log_destroy(logger_dump);
	log_destroy(logger_compactacion);
	log_destroy(logger_consola);
	config_destroy(archivoconfig);
}
/*void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
	destuir_hilo_dump;
	destruir_hilo_consola;
}
*/
