/*
 * LFS_Compactacion.c
 *
 *  Created on: 27 jun. 2019
 *      Author: utnso
 */
#include "LFS_Compactacion.h"

void *compactar(char* nombre_tabla, int tiempo_compactacion){ //ESTO ES LEGAL ?
	char* path_tabla; //genero el path
	log_info(logger, "Tiempo retardo compactacion: [%d]", tiempo_compactacion);
	while(1){
		log_info(logger, "%d", tiempo_compactacion);
		sleep(tiempo_compactacion/1000);

		if (hay_temporales(path_tabla)) {
			log_info(logger, "Compactacion- No hay datos para compactar en: %s.", path_tabla);
		}else{
			log_info(logger, "Se realiza compactacion en: %s.", path_tabla);
			renombrar_temp_a_tempc(path_tabla);

			char* registros = leer_registros_temporales(path_tabla);
			char* registros_filtrados= filtrar_registros_duplicados_segun_particiones(path_tabla, registros);

			bloquear_tabla();
			int comienzo = timestamp();

			realizar_compactacion(path_tabla, registros_filtrados);
			int tiempo_operatoria = comienzo - timestamp();

			desbloquear_tabla();
			log_info(logger, "La tabla: %s estuvo bloqueada %d milisegundos.", nombre_tabla, tiempo_operatoria);

		}
	}
}

//podria recibir path de tabla, o nombre por parametro, y tiempo de retardo entre cada compact. para esa tabla
void crear_hilo_compactacion(){
	pthread_t hilo_compactacion;
	if (pthread_create(&hilo_compactacion, 0, compactar, NULL) !=0){
		log_error(logger, "Error al crear el hilo para proceso de compactacion");
	}
	if (pthread_detach(hilo_compactacion) != 0){
		log_error(logger, "Error al frenar hilo de compactacion");
	}
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

void realizar_compactacion(path_tabla, registros_filtrados){
	//escribir archivos, liberar .bin, escribir nuevos .bin
}
