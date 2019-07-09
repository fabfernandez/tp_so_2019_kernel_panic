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
			char* registros_filtrados= filtrar_registros_duplicados_segun_particiones(path_tabla, registros); //esto podria devolver una matris filtrando los datos respecto de la particion a la que corresponde, devolveria una lista de lista donde cada posicin de la lista es el index de la particion, y la lista en esa posicion contiene los registros filtrados en base a ese archivo

			bloquear_tabla(nombre_tabla); //lo hace gaby
			int comienzo = time(NULL);

			realizar_compactacion(path_tabla, registros_filtrados);

			desbloquear_tabla(nombre_tabla); // lo hace gaby
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

/*
 * t_list* chars_to_ints(char* bloques){
	t_list* bloques_int = list_create();

	for(int i=1 ; bloques[i] != ']'; i++){
		list_add(bloques_int, atoi(bloques[i]));
	}

	return bloques_int;
}*/

t_list* leer_registros_bloques(char* bloques, int size_total){
	char* registros = string_new();

	for(int i=1 ; bloques[i] != ']'; i++){
		int bloque = atoi(bloques[i]);

		string_append(&registros, leer_registros_de_bloque(bloque));
	}
	string_append(&registros, "\0");

	t_list* registros_finales = transformar_registros(registros);
	free(registros);

	return registros_finales;
}

char* leer_registros_de_bloque(int bloque){
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
		while(registros[i] != '\n'){
			timestamp_char[l] = registros[i];
			i++;
			l++;
		}
		timestamp_char[l]= '\0';
		long timestamp = atol(timestamp_char);
		free(timestamp_char);
		l=0;


		char* key_char = (char*) malloc(sizeof(char));
		while(registros[i] != '\n'){
			key_char[l] = registros[i];
			key_char = (char*) realloc(key_char,sizeof(char));
			i++;
			l++;
		}
		key_char[l]='\0';
		uint16_t key = (uint16_t) atol(key_char);
		free(key_char);
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
}

void filtrar_registros_duplicados_segun_particiones(char* path_tabla, char* registros_nuevos){
	//raro porque deberia tener leidos todos los datos de cada particion.
	//generar estructuras para la tabla,
	//validar que mis registros esten tocando todas las particiones (para no tocar particiones al pedo
}

void realizar_compactacion(char* path_tabla, char* registros_filtrados){
	//escribir archivos, liberar .bin, escribir nuevos .bin
}