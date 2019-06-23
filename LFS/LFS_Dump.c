/*
 * LFS_Dump.c
 *
 *  Created on: 23 jun. 2019
 *      Author: utnso
 */
#include "LFS_Dump.h"

void eliminar_registro(t_registro* registro){
	free(registro->value);
	free(registro);
}

void eliminar_tabla(t_cache_tabla* tabla_cache){
	free(tabla_cache->nombre);
	list_destroy_and_destroy_elements(tabla_cache->registros, (void*)eliminar_registro);
	free(tabla_cache);
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

	int num = proximo_archivo_temporal_para(tabla->nombre);
	char* dir_temporal = string_from_format("%s/Tables/%s/%i.temp", path_montaje, tabla->nombre, num);

	crear_archivo(dir_temporal, size_registros, bloques_ocupados);
	free(dir_temporal);
	list_destroy(bloques_ocupados);
}

void *dump(){
	while(1){
		sleep(tiempo_dump);
		log_info(logger, "Se realiza dump");
		t_list* datos = copiar_y_limpiar_memtable();

		while(!list_is_empty(datos)){
			t_cache_tabla* tabla = list_remove(memtable, 0);
			dump_por_tabla(tabla);
			eliminar_tabla(tabla);
		}
	}
}

void crear_hilo_dump(){
	pthread_t hilo_dump;
	if (pthread_create(&hilo_dump, 0, dump, NULL) !=0){
		log_error(logger, "Error al crear el hilo para proceso de dump");
	}
	if (pthread_detach(hilo_dump) != 0){
		log_error(logger, "Error al frenar hilo de dump");
	}
}


/**
 * * @NAME: bajo_registros_a_blocks
 	* @DESC: Escribe registrs de una tabla en bloques
 	*          Retorna lista de numeros identificadores de los bloques escritos
 	*
 	*/
t_list* bajo_registros_a_blocks(int size_registros, char* registros){

	int cantidad_bloques = div_redondeada_a_mayor( size_registros,block_size );
	t_list* bloques = list_create();

	for(int i=0; i < cantidad_bloques; i++){

		int byte_inicial = i*block_size;
		int byte_final = byte_inicial + tamanio_bloque(i+1,cantidad_bloques, size_registros);
		char* datos = string_substring(registros, byte_inicial, byte_final);
		int bloque = obtener_bloque_disponible();

		escribir_bloque(bloque, datos);
		list_add(bloques, bloque);
		free(datos);
	}

	return bloques;
}

int tamanio_bloque(int bloque_por_escribir, int bloques_totales, int size_datos){
	int tamanio = block_size;

	if(bloque_por_escribir == bloques_totales){
		tamanio = (bloques_totales * block_size) - size_datos;
	}
	return tamanio;
}

void escribir_bloque(int bloque, char* datos){

	char* dir_bloque = string_from_format("%s/Bloques/%i.bin", path_montaje, bloque);
	FILE* file = fopen(dir_bloque, "wb+");

	fwrite(datos, sizeof(datos[0]), sizeof(datos), file);

	fclose(file);
	free(dir_bloque);
}

int proximo_archivo_temporal_para(char* tabla){
	int temporales = dictionary_get(temporales_por_tabla, tabla);
	if(temporales == NULL ){
		temporales = 1;
	}else{
	temporales++;
	}
	dictionary_put(temporales_por_tabla, tabla, temporales);
	return temporales;
}

t_list* copiar_y_limpiar_memtable(){

	//desconfio que esto este haciendo lo que yo quiero
	t_list* copia = list_create();
	void _agregar_a_copia(t_cache_tabla* tabla){
		list_add(copia, tabla);
	}
	list_iterate(memtable, (void*)_agregar_a_copia);

	pthread_mutex_lock(&mutexMemtable);

	list_clean(memtable);

	pthread_mutex_unlock(&mutexMemtable);
	return copia;
}

int div_redondeada_a_mayor(int a, int b){
	int resto = a % b;
	int retorno = a/b;

	return resto==0 ? retorno : (retorno+1);
}
