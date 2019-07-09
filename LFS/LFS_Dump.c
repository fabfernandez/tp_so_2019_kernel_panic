/*
 * LFS_Dump.c
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */
#include "LFS_Dump.h"

void *dump(){
	log_info(logger_dump, "Tiempo retardo dump: [%d]", tiempo_dump);
	while(1){
		sleep(tiempo_dump/1000);
		if (list_is_empty(memtable)) {
			log_info(logger_dump, "DUMP- No hay datos para dumpear.");
		}else{
			log_info(logger_dump, "Se realiza dump");
			t_list* datos = copiar_y_limpiar_memtable();

			while(!list_is_empty(datos)){
				t_cache_tabla* tabla = list_remove(datos, 0);
				log_info(logger_dump, "Se realiza dump para tabla: [%s]",tabla->nombre);
				dump_por_tabla(tabla);
				eliminar_tabla(tabla);
			}

		}
	}
}

void crear_hilo_dump(){
	if (pthread_create(&hilo_dump, 0, dump, NULL) !=0){
		log_error(logger_dump, "Error al crear el hilo para proceso de dump");
	}
	if (pthread_detach(hilo_dump) != 0){
		log_error(logger_dump, "Error al frenar hilo de dump");
	}
}

void dump_por_tabla(t_cache_tabla* tabla){
	char * array_registros = string_new();

	while(!list_is_empty(tabla->registros)){
		t_registro* registro = list_remove(tabla->registros, 0);
		string_append(&array_registros, string_from_format("%ld;", registro->timestamp));
		string_append(&array_registros, string_from_format("%i;", registro->key));
		string_append(&array_registros, registro->value);
		string_append(&array_registros,"\n");
		eliminar_registro(registro);
	}

	int size_registros = string_length(array_registros);
	t_list* bloques_ocupados = bajo_registros_a_blocks(size_registros,array_registros);
	free(array_registros);

	char* path_tabla = string_from_format("%s/Tables/%s", path_montaje, tabla->nombre);

	int num = proximo_archivo_temporal_para(path_tabla);
	string_append(&path_tabla, string_from_format("/%i.temp", num));

	log_info(logger_dump, "Se crea el temporal: [%d], en el path: [%s]", num, path_tabla);
	crear_archivo(path_tabla, size_registros, bloques_ocupados);
	free(path_tabla);
	list_destroy(bloques_ocupados);
}


/**
 * * @NAME: bajo_registros_a_blocks
 	* @DESC: Escribe registrs de una tabla en bloques
 	*          Retorna lista de numeros identificadores de los bloques escritos
 	*
 	*/
t_list* bajo_registros_a_blocks(int size_registros, char* registros){

	int cantidad_bloques = div_redondeada_a_mayor( size_registros,block_size );
	log_info(logger_dump, "Cantidad de blocks a ocupar: [%d]",cantidad_bloques);
	t_list* bloques = list_create();

	for(int i=0; i < cantidad_bloques; i++){

		int byte_inicial = i*block_size;
		int tamanio_registros = tamanio_bloque(i+1,cantidad_bloques, size_registros);
		int byte_final = byte_inicial + tamanio_registros;
		char* datos = string_substring(registros, byte_inicial, byte_final);
		int bloque = obtener_bloque_disponible();

		log_info(logger_dump, "Se escriben [%d] bytes de registros, en el bloque: [%d]",tamanio_registros,bloque);
		escribir_bloque(bloque, datos);
		log_info(logger_dump, "Bloque escrito satisfactoriamente");
		list_add(bloques, (int) bloque);
		free(datos);
	}

	return bloques;
}

/**
 * * @NAME: tamanio_bloque
 	* @DESC: Define cantidad de bytes de datos que seran escritos en el proximo bloque
 	*
 	*/

int tamanio_bloque(int bloque_por_escribir, int bloques_totales, int size_datos){
	int tamanio = block_size;

	if(bloque_por_escribir == bloques_totales){
		tamanio = size_datos - ((bloques_totales-1) * block_size);
	}
	return tamanio;
}

void escribir_bloque(int bloque, char* datos){

	char* dir_bloque = string_from_format("%s/Bloques/%i.bin", path_montaje, bloque);
	FILE* file = fopen(dir_bloque, "wb+");

	fwrite(datos, sizeof(datos[0]), string_length(datos), file);

	fclose(file);
	free(dir_bloque);
}

/**
 * * @NAME: proximo_archivo_temporal_para
 	* @DESC: Define index del proximo archivo temporal para una tabla
 	*
 	*/
int proximo_archivo_temporal_para(char* path_tabla){
	int proximo_temporal = cantidad_archivos_actuales(path_tabla,"temp" );
	proximo_temporal++;
	return proximo_temporal;
}

int cantidad_archivos_actuales(char* path_dir, char* extension_archivo){
	int temporales_que_hay = 0;
	DIR * dir = opendir(path_dir);
	struct dirent * entry = readdir(dir);

	while(entry != NULL){
		if (( strcmp(entry->d_name, ".")!=0 && strcmp(entry->d_name, "..")!=0 ) && archivo_es_del_tipo(entry->d_name, extension_archivo)) {
			temporales_que_hay ++;
		}
		entry = readdir(dir);
	}
	return temporales_que_hay;
}

bool archivo_es_del_tipo(char* archivo, char* extension_archivo){
	char ** nombre_y_extension = string_split(archivo, ".");
	if (nombre_y_extension[1]== NULL){
		return false;
	}
	return strcmp(nombre_y_extension[1], extension_archivo) == 0;
}


/**
 * * @NAME: copiar_y_limpiar_memtable
 	* @DESC: Duplica memtable para realizar el dump y no generar valores inconsistentes, ni
 	*          bloquear la utilizacion de la memtable por otros request mientras se realiza el dump.
 	*
 	*/
t_list* copiar_y_limpiar_memtable(){

	log_info(logger, "Duplico y limpio memtable para bloquear su uso el mejor tiempo posible");

	pthread_mutex_lock(&mutexMemtable);

	t_list* copia = list_duplicate(memtable);
	memtable = list_create();
	log_info(logger_dump, "Memtable lista para recibir datos nuevos");

	pthread_mutex_unlock(&mutexMemtable);
	return copia;
}

int div_redondeada_a_mayor(int a, int b){
	int resto = a % b;
	int retorno = a/b;

	return resto==0 ? retorno : (retorno+1);
}

void eliminar_registro(t_registro* registro){
	free(registro->value);
	free(registro);
}

void eliminar_tabla(t_cache_tabla* tabla_cache){
	free(tabla_cache->nombre);
	list_destroy_and_destroy_elements(tabla_cache->registros, (void*)eliminar_registro);
	free(tabla_cache);
}

