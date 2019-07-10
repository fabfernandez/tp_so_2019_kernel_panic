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

	log_info(logger_compactacion, "Tiempo retardo compactacion: [%i]", tiempo_compactacion);
	while(1){
		log_info(logger_compactacion, "%i", tiempo_compactacion);
		sleep(tiempo_compactacion/1000);

		if (hay_temporales(path_tabla)) {
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
	}
	//free path_tabla
}

void bloquear_tabla(char* nombre_tabla){

}

void desbloquear_tabla(char* nombre_tabla){

}

void crear_hilo_compactacion(char* nombre_tabla){
	pthread_t hilo_compactacion;
	if (pthread_create(&hilo_compactacion, 0, compactar, nombre_tabla) !=0){
		log_error(logger_compactacion, "Error al crear el hilo para proceso de compactacion");
	}
	if (pthread_detach(hilo_compactacion) != 0){
		log_error(logger_compactacion, "Error al frenar hilo de compactacion");
	}
}

long obtener_tiempo_compactacion(char* path_tabla){
	char* path_metadata = string_from_format("%s/Metadata", path_tabla);

	t_config* metadata_tabla = config_create(path_metadata);
	long tiempo_compactacion = config_get_long_value(metadata_tabla,"COMPACTION_TIME");
	config_destroy(metadata_tabla);

	return tiempo_compactacion;
}

bool hay_temporales(char* path_tabla){
	int cant_temporales = cantidad_archivos_actuales(path_tabla, "temp");
	return cant_temporales == 0 ? false : true;
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
			char* tempc = string_from_format("%s/%s.temp", path_tabla, nombre_y_extension[0]);
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
	return registros;
}

t_list* leer_registros_particiones(char* path_tabla){
	char* path_metadata = string_from_format("%s/Metadata", path_tabla);
	t_config* metadata = config_create(path_metadata);
	int num_particiones = config_get_int_value(metadata, "PARTITIONS");

	t_list* registros = list_create();

	for(int i=0 ; i<=num_particiones; i++){
		char* path_archivo = string_from_format("%s/%d.bin", path_tabla, i);
		list_add(registros, obtener_registros_de_archivo(path_archivo));
	}

	config_destroy(metadata);
	return registros;
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

	while(!list_is_empty(registros_nuevos)){
		t_registro* un_registro = list_remove(registros_nuevos, 0);

		int particion = un_registro->key % num_particiones;
		actualizar_registro(list_get(registros_particiones,particion), un_registro); //PRECAUCION: Capaz list_get no funciona, necesitaria que trabaje con el puntero de la lista
	}

	log_info(logger_compactacion, "Se obtienen registros filtrados para: %s.", path_tabla);
	return registros_particiones;
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

void realizar_compactacion(char* path_tabla, t_list* registros_filtrados){
	liberar_bloques_tabla(path_tabla);

	//falta borrar todos tempc y .bin

	DIR * dir = opendir(path_tabla);
		struct dirent * entry = readdir(dir);
		while(entry != NULL){
			if ( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 && strcmp(entry->d_name, "Metadata")!=0  && !archivo_es_del_tipo(entry->d_name, "temp")) {
				char* dir_archivo = string_from_format("%s/%s", path_tabla, entry->d_name);
				if (unlink(dir_archivo) == 0)
					log_info(logger, "Eliminado archivo: %s\n", entry->d_name);
				else
					log_info(logger, "No se puede eliminar archivo: %s\n", entry->d_name);
			}
			entry = readdir(dir);
		}


	for(int i=0 ; !list_is_empty(registros_filtrados); i++){
		t_list* registros_de_particion = list_remove(registros_filtrados, 0);
		char* path_archivo = string_from_format("%s/%d.bin", path_tabla, i);

		escribir_registros_y_crear_archivo(registros_de_particion, path_archivo);
		free(path_archivo);
		list_destroy(registros_de_particion);
	}
}
