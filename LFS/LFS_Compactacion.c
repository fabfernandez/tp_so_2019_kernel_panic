/*
 * LFS_Compactacion.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "LFS_Compactacion.h"

void *compactar(void* nombre_tabla){
	//char* tabla = (char*) nombre_tabla; Agregar linea comentada si descubro que void* rompe en ejecucion y necesito castera char*
	char* path_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
	long tiempo_compactacion = obtener_tiempo_compactacion(path_tabla);
	t_tabla_logica* tabla_logica = buscar_tabla_logica_con_nombre(nombre_tabla);
	log_info(logger_compactacion, "Tiempo retardo compactacion: [%i]", tiempo_compactacion);
	while(1){
		log_info(logger_compactacion, "%i", tiempo_compactacion);
		sleep(tiempo_compactacion/1000);

		pthread_mutex_lock(&(tabla_logica->mutex_compactacion));
		if (!hay_temporales(path_tabla)) {
			log_info(logger_compactacion, "Compactacion- No hay datos para compactar en: %s.", path_tabla);
		}else{
			log_info(logger_compactacion, "Se realiza compactacion en: %s.", path_tabla);
			int cantidad_archivos_renombrados = renombrar_archivos_para_compactar(path_tabla);

			t_list* registros = leer_registros_temporales(path_tabla, cantidad_archivos_renombrados);
			t_list* registros_filtrados= filtrar_registros_duplicados_segun_particiones(path_tabla, registros); //esto podria devolver una matris filtrando los datos respecto de la particion a la que corresponde, devolveria una lista de lista donde cada posicin de la lista es el index de la particion, y la lista en esa posicion contiene los registros filtrados en base a ese archivo

			list_destroy(registros);

			bloquear_tabla(nombre_tabla);
			int comienzo = time(NULL);

			realizar_compactacion(path_tabla, registros_filtrados);

			desbloquear_tabla(nombre_tabla);
			int tiempo_operatoria = comienzo - time(NULL);
			log_info(logger_compactacion, "La tabla: %s estuvo bloqueada %d milisegundos por compactacion.", nombre_tabla, tiempo_operatoria);

		}
		pthread_mutex_unlock(&(tabla_logica->mutex_compactacion));
	}
	//free path_tabla
}

void bloquear_tabla(char* nombre_tabla){
	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}

	t_tabla_logica* tabla_a_bloquear = list_find(tablas_en_lfs, _es_tabla_con_nombre);
	tabla_a_bloquear->esta_bloqueado=true;
}

void desbloquear_tabla(char* nombre_tabla){
	int _es_tabla_con_nombre(t_cache_tabla* tabla) {
		return string_equals_ignore_case(tabla->nombre, nombre_tabla);
	}

	t_tabla_logica* tabla_a_bloquear = list_find(tablas_en_lfs, _es_tabla_con_nombre);
	tabla_a_bloquear->esta_bloqueado=false;

	if (dictionary_has_key(instrucciones_bloqueadas_por_tabla, nombre_tabla)){
		t_queue* instrucciones_bloqueadas = dictionary_get(instrucciones_bloqueadas_por_tabla, nombre_tabla);
		for(int i=0; i<queue_size(instrucciones_bloqueadas); i++){
			t_instruccion_bloqueada* instruccion_bloqueada = queue_pop(instrucciones_bloqueadas);
			t_status_solicitud* status;
			if (instruccion_bloqueada->instruccion.operacion == SELECT){
				if (instruccion_bloqueada->socket_memoria==NULL){
					resolver_select_consola(nombre_tabla, instruccion_bloqueada->instruccion.parametros.SELECT.key);
				}else{
					status= resolver_select(nombre_tabla, instruccion_bloqueada->instruccion.parametros.SELECT.key);
					enviar_status_resultado(status, instruccion_bloqueada->socket_memoria);
				}

			}else{
				status = resolver_insert(logger, nombre_tabla, instruccion_bloqueada->instruccion.parametros.INSERT.key, instruccion_bloqueada->instruccion.parametros.INSERT.value, instruccion_bloqueada->instruccion.parametros.INSERT.timestamp);
				if (instruccion_bloqueada->socket_memoria!=NULL){
					enviar_status_resultado(status, instruccion_bloqueada->socket_memoria);
				}
			}
			//eliminar_paquete_status(status);
			free(instruccion_bloqueada);
		}
		free(dictionary_remove(instrucciones_bloqueadas_por_tabla, nombre_tabla));

	}

}

pthread_t crear_hilo_compactacion(char* nombre_tabla){
	pthread_t hilo_compactacion;
	if (pthread_create(&hilo_compactacion, 0, compactar, nombre_tabla) !=0){
		log_error(logger_compactacion, "Error al crear el hilo para proceso de compactacion");
	}
	if (pthread_detach(hilo_compactacion) != 0){
		log_error(logger_consola, "Error al frenar hilo");
	}
	return hilo_compactacion;
}

long obtener_tiempo_compactacion(char* path_tabla){
	char* path_metadata = string_from_format("%s/Metadata", path_tabla);

	t_config* metadata_tabla = config_create(path_metadata);
	long tiempo_compactacion = config_get_long_value(metadata_tabla,"COMPACTION_TIME");
	config_destroy(metadata_tabla);

	return tiempo_compactacion;
}

bool hay_temporales(char* path_tabla){
	return cantidad_archivos_actuales(path_tabla, "temp")>0;

}

int renombrar_archivos_para_compactar(char* path_tabla){
	int cantidad_temporales = 0;
	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);

	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && archivo_es_del_tipo(entry->d_name, "temp")) {
			cantidad_temporales ++;
			char* temp = string_from_format("%s/%s", path_tabla, entry->d_name);

			char ** nombre_y_extension = string_split(entry->d_name, ".");
			char* tempc = string_from_format("%s/%s.tempc", path_tabla, nombre_y_extension[0]);
			rename(temp, tempc);

			//free a todos esos char?
		}
		entry = readdir(dir);
	}
	closedir(dir);

	log_info(logger_compactacion, "Renombrando %d archivos temp a tempc en: %s.", cantidad_temporales ,path_tabla);
	return cantidad_temporales;
}

t_list* leer_registros_temporales(char* path_tabla, int cantidad_temporales){
	t_list* registros = list_create();

	for(int i=1 ; i<=cantidad_temporales; i++){
		char* path_archivo = string_from_format("%s/%d.tempc", path_tabla, i);
		list_add_all(registros, obtener_registros_de_archivo(path_archivo));
//		t_config* tempc = config_create(path_archivo);
//		char* bloques = config_get_string_value(tempc,"BLOCKS");
//		int size = config_get_int_value(tempc,"SIZE");
//
//		//t_list* bloques_ints = chars_to_ints(bloques);
//		list_add_all(registros, leer_registros_bloques(bloques,size));
//
//		//free(bloques)
//		//free(path_archivo)
//		config_destroy(tempc);
	}
	log_info(logger_compactacion, "Se leen registros de %d archivos temporales en: %s.", cantidad_temporales ,path_tabla);
	return registros;
}

t_list* leer_registros_particiones(char* path_tabla){
	char* path_metadata = string_from_format("%s/Metadata", path_tabla);
	t_config* metadata = config_create(path_metadata);
	int num_particiones = config_get_int_value(metadata, "PARTITIONS");

	t_list* registros = list_create();

	for(int i=0 ; i<num_particiones; i++){
		char* path_archivo = string_from_format("%s/%d.bin", path_tabla, i);

		list_add(registros, obtener_registros_de_archivo(path_archivo));
	}

	config_destroy(metadata);
	return registros;
}

t_registro* crear_t_registros_por_particion(char* value, uint16_t key, long timestamp){

	t_registro* nuevo_registro = malloc(sizeof(t_registro));
	nuevo_registro->key=key;
	nuevo_registro->timestamp = timestamp;
	nuevo_registro->value=malloc(string_size(value));
	memcpy(nuevo_registro->value, value, string_size(value));

	return nuevo_registro;
}
/*
 * t_list* chars_to_ints(char* bloques){
	t_list* bloques_int = list_create();

	for(int i=1 ; bloques[i] != ']'; i++){
		list_add(bloques_int, atoi(bloques[i]));
	}

	return bloques_int;
}*/

/*t_list* leer_registros_bloques(char* bloques, int size_total){
	char* registros = string_new();

	for(int i=1 ; bloques[i] != ']'; i++){
		int bloque = atoi(&bloques[i]);

		string_append(&registros, leer_registros_char_de_bloque(bloque));
	}
	string_append(&registros, "\0");

	t_list* registros_finales = transformar_registros(registros);
	free(registros);

	return registros_finales;
}

char* leer_registros_char_de_bloque(int bloque){
	char* registros = string_new();

	char* dir_bloque = string_from_format("%s/Bloques/%i.bin", path_montaje, bloque);
	FILE* file = fopen(dir_bloque, "rb+");

	char* buffer = (char*) malloc(sizeof(char));

	while(!feof(file)){
		fread(buffer, sizeof(char), 1, file);
		string_append(&registros, buffer);
	}
	fclose(file);
	free(buffer);
	free(dir_bloque);

	return registros;
}

t_list* transformar_registros(char* registros){
	t_list* registros_finales = list_create();

	for(int i=0 ; registros[i] != '\0'; i++){
		int l=0;


		char* timestamp_char = (char*) malloc(sizeof(char)*11);
		while(registros[i] != ';'){
			timestamp_char[l] = registros[i];
			i++;
			l++;
		}
		timestamp_char[l]= '\0';
		long timestamp = atol(timestamp_char);
		free(timestamp_char);
		i++;
		l=0;


		char* key_char = (char*) malloc(sizeof(char));
		while(registros[i] != ';'){
			key_char[l] = registros[i];
			key_char = (char*) realloc(key_char,sizeof(char));
			i++;
			l++;
		}
		key_char[l]='\0';
		uint16_t key = (uint16_t) atol(key_char);
		free(key_char);
		i++;
		l=0;


		char* value= (char*) malloc(sizeof(char));
		while(registros[i] != '\n'){
			value[l] = registros[i];
			value = (char*) realloc(value,sizeof(char));
			i++;
			l++;
		}
		value[l]='\0';

		t_registro* registro_nuevo = crear_registro(value, key, timestamp);
		list_add(registros_finales, registro_nuevo);
	}

	return registros_finales;
}*/

t_list* filtrar_registros_duplicados_segun_particiones(char* path_tabla, t_list* registros_nuevos){
	t_list* registros_particiones = leer_registros_particiones(path_tabla);
	int num_particiones = list_size(registros_particiones);

	t_dictionary* particiones_tocadas = dictionary_create();

	while(!list_is_empty(registros_nuevos)){
		t_registro* un_registro = list_remove(registros_nuevos, 0);

		int particion = un_registro->key % num_particiones;

		dictionary_put(particiones_tocadas, (char*)string_itoa(particion), (bool*) true);
		actualizar_registro(list_get(registros_particiones,particion), un_registro); //PRECAUCION: Capaz list_get no funciona, necesitaria que trabaje con el puntero de la lista
	}

	vaciar_datos_de_listas_no_tocadas(registros_particiones, particiones_tocadas);

	log_info(logger_compactacion, "Se obtienen registros filtrados para: %s.", path_tabla);
	return registros_particiones;
}

void vaciar_datos_de_listas_no_tocadas(t_list* registros_particiones, t_dictionary* particiones_tocadas){
	for(int i= 0; list_get(registros_particiones, i)!= NULL; i++ ){
		if(dictionary_get(particiones_tocadas, string_itoa(i))){
		}else{
			t_list* registros_no_tocados = list_replace(registros_particiones, i, list_create());
			list_destroy_and_destroy_elements(registros_no_tocados,(void*) eliminar_registro);
		}
	}
}

void actualizar_registro(t_list* registros, t_registro* un_registro){

	int _es_registro_con_key(t_registro* registro){
		return registro->key== un_registro->key;
	}

	t_registro* registro_viejo = list_find(registros, (void*) _es_registro_con_key);

	if(registro_viejo == NULL){
		list_add(registros, un_registro);
	}else{
		if(registro_viejo->timestamp <= un_registro->timestamp){
			list_remove_by_condition(registros, (void*) _es_registro_con_key);
			eliminar_registro(registro_viejo);
			list_add(registros, un_registro);
		}else{
			eliminar_registro(un_registro);
		}
	}
}
void liberar_bloques_compactacion(char* path_tabla, t_list* particiones_a_liberar){

	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && ( archivo_es_del_tipo(entry->d_name,"tempc") || (archivo_es_del_tipo(entry->d_name,"bin")&& pertenece_a_lista_particiones(particiones_a_liberar,entry->d_name) )) ) {
			char* dir_archivo = string_from_format("%s/%s", path_tabla, entry->d_name);
			liberar_bloques_archivo(dir_archivo);
		}
		entry = readdir(dir);
	}
}

bool pertenece_a_lista_particiones(t_list* particiones_a_liberar,char* nombre_archivo){
	char ** nombre_y_extension = string_split(nombre_archivo, ".");
	if (nombre_y_extension[0]== NULL){
		return false;
	}else{
		int particion = atoi(nombre_y_extension[0]);

		bool _es_igual(int elemento_lista){
			return particion == elemento_lista;
		}

		return list_any_satisfy(particiones_a_liberar, _es_igual);
	}
}

void realizar_compactacion(char* path_tabla, t_list* registros_filtrados){
	t_list* particiones_a_liberar = encontrar_particiones_tocadas(registros_filtrados);
	liberar_bloques_compactacion(path_tabla, particiones_a_liberar);

	//borra todos tempc y .bin para una tabla
	eliminar_temp_y_bin_tabla(path_tabla, particiones_a_liberar);

	for(int i=0 ; !list_is_empty(registros_filtrados); i++){
		t_list* registros_de_particion = list_remove(registros_filtrados, 0);

		if(!list_is_empty(registros_de_particion)){
		char* path_archivo = string_from_format("%s/%d.bin", path_tabla, i);

		escribir_registros_y_crear_archivo(registros_de_particion, path_archivo);
		free(path_archivo);
		}

		list_destroy(registros_de_particion);
	}
}

t_list* encontrar_particiones_tocadas(t_list* registros_filtrados){
	t_list* listas_tocadas =  list_create();
	int index= 0;

	void no_es_vacia(t_list* elemento){
		if(!list_is_empty(elemento)){
			list_add(listas_tocadas, (int*) index);
		}
		index++;
	}
	list_iterate(registros_filtrados, (void*) no_es_vacia);

	return listas_tocadas;
}

void eliminar_temp_y_bin_tabla(char* path_tabla, t_list* particiones_a_liberar){
	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);
	while(entry != NULL){
		if ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 && strcmp(entry->d_name, "Metadata")!=0  && !archivo_es_del_tipo(entry->d_name, "temp")) {
			if(pertenece_a_lista_particiones(particiones_a_liberar,entry->d_name) ){
				char* dir_archivo = string_from_format("%s/%s", path_tabla, entry->d_name);
				if (unlink(dir_archivo) == 0)
					log_info(logger, "Eliminado archivo: %s\n", entry->d_name);
				else
					log_info(logger, "No se puede eliminar archivo: %s\n", entry->d_name);
			}
		}
		entry = readdir(dir);
	}
}
