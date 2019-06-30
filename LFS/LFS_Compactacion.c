/*
 * LFS_Compactacion.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "LFS_Compactacion.h"

void *compactar(void* nombre_tabla){
	//char* tabla = (char*) nombre_tabla;
	char* path_tabla = string_from_format("%s/Tables/%s", path_montaje, nombre_tabla);
	long tiempo_compactacion = obtener_tiempo_compactacion(path_tabla);

	log_info(logger, "Tiempo retardo compactacion: [%i]", tiempo_compactacion);
	while(1){
		log_info(logger, "%i", tiempo_compactacion);
		sleep(tiempo_compactacion/1000);

		if (hay_temporales(path_tabla)) {
			log_info(logger, "Compactacion- No hay datos para compactar en: %s.", path_tabla);
		}else{
			log_info(logger, "Se realiza compactacion en: %s.", path_tabla);
			renombrar_temp_a_tempc(path_tabla);

			char* registros = leer_registros_temporales(path_tabla);
			char* registros_filtrados= filtrar_registros_duplicados_segun_particiones(path_tabla, registros); //esto podria devolver una matris filtrando los datos respecto de la particion a la que corresponde, devolveria una lista de lista donde cada posicin de la lista es el index de la particion, y la lista en esa posicion contiene los registros filtrados en base a ese archivo

			bloquear_tabla();
			int comienzo = timestamp();

			realizar_compactacion(path_tabla, registros_filtrados);
			int tiempo_operatoria = comienzo - timestamp();

			desbloquear_tabla();
			log_info(logger, "La tabla: %s estuvo bloqueada %d milisegundos.", nombre_tabla, tiempo_operatoria);

		}
	}
}

void crear_hilo_compactacion(char* nombre_tabla){
	pthread_t hilo_compactacion;
	if (pthread_create(&hilo_compactacion, 0, compactar, nombre_tabla) !=0){
		log_error(logger, "Error al crear el hilo para proceso de compactacion");
	}
	if (pthread_detach(hilo_compactacion) != 0){
		log_error(logger, "Error al frenar hilo de compactacion");
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

void renombrar_archivos_para_compactar(char* path_tabla){
	DIR * dir = opendir(path_tabla);
	struct dirent * entry = readdir(dir);

	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && archivo_es_del_tipo(entry->d_name, "temp")) {
			//renombrar
		}
		entry = readdir(dir);
	}
}

void realizar_compactacion(char* path_tabla, char* registros_filtrados){
	//escribir archivos, liberar .bin, escribir nuevos .bin
}
