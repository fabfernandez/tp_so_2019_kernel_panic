/*
 * LFS_Consola.c
 *
 *  Created on: 23 may. 2019
 *      Author: utnso
 */
#include "LFS_Consola.h"

void crear_hilo_consola(){
	pthread_t hilo_consola;
	if (pthread_create(&hilo_consola, 0, levantar_consola, NULL) !=0){
		log_error(logger, "Error al crear el hilo");
	}
	if (pthread_detach(hilo_consola) != 0){
		log_error(logger, "Error al frenar hilo");
	}
}

void *levantar_consola(){
	while(1){
		char* linea = readline("Consola LFS >");
		t_instruccion_lql instruccion = parsear_linea(linea);
		if (instruccion.valido) {
			resolver_operacion_por_consola(instruccion);
		}else{
			printf("Reingrese la instruccion");
		}
		free(linea);
	}
}

int resolver_operacion_por_consola(t_instruccion_lql instruccion){
	switch(instruccion.operacion)
		{
		case SELECT:
			log_info(logger, "Se solicito SELECT por consola");
			resolver_select_consola(instruccion.parametros.SELECT.tabla, instruccion.parametros.SELECT.key);
			break;
		case INSERT:
			log_info(logger, "Se solicitó INSERT por consola");
			t_status_solicitud* status = resolver_insert(instruccion.parametros.INSERT.tabla,
													instruccion.parametros.INSERT.key,
													instruccion.parametros.INSERT.value,
													instruccion.parametros.INSERT.timestamp);
			eliminar_paquete_status(status);
			break;
		case CREATE:
			log_info(logger, "Se solicitó CREATE por consola");
			status = resolver_create(instruccion.parametros.CREATE.tabla,
					instruccion.parametros.CREATE.consistencia,
					instruccion.parametros.CREATE.num_particiones,
					instruccion.parametros.CREATE.compactacion_time);
			//enviar_status_resultado(status, socket_memoria);
			log_info(logger, status->mensaje->palabra);
			eliminar_paquete_status(status);
			//aca debería enviarse el mensaje a LFS con CREATE
			break;
		case DESCRIBE:
			log_info(logger, "Se solicitó DESCRIBE por consola");
			//resolver_describe_drop(instruccion);
			//aca debería enviarse el mensaje a LFS con DESCRIBE
			break;
		case DROP:
			log_info(logger, "Se solicitó DROP por consola");
			//resolver_describe_drop(instruccion);
			//aca debería enviarse el mensaje a LFS con DROP
			break;

		default:
			log_warning(logger, "Operacion desconocida.");
			return EXIT_FAILURE;
		}
	return EXIT_SUCCESS;
}

void resolver_select_consola (char* nombre_tabla, uint16_t key){

	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", nombre_tabla);
	log_info(logger, "Consulta por key: %d", key);
	if (existe_tabla(nombre_tabla)){
		t_registro* registro_buscado = buscar_registro_memtable(nombre_tabla, key);
		//TODO: buscar en archivos temporales y en bloques
		if(registro_buscado !=NULL){
			char* resultado = generar_registro_string(registro_buscado->timestamp, registro_buscado->key, registro_buscado->value);
			log_info(logger, "Resultado: %s", resultado);

		}else{
			char * mje_error = string_from_format("No se encontró registro con key: %d en la tabla %s", key, nombre_tabla);
			log_error(logger, mje_error);
		}

	}else{
		char * mje_error = string_from_format("La tabla %s no existe", nombre_tabla);
		log_error(logger, mje_error);
	}
}


