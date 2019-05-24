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
			resolver_select(instruccion.parametros.SELECT.tabla, instruccion.parametros.SELECT.key);
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
			//resolver_create(instruccion);
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



