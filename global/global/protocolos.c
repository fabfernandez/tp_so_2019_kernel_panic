/*
 * protocolos.c
 *
 *  Created on: 13 abr. 2019
 *      Author: utnso
 */


#include "protocolos.h"

int get_tamanio_paquete_select(t_paquete_select* paquete_select){
	return paquete_select->nombre_tabla->size + sizeof(uint16_t)+ sizeof(t_operacion)+sizeof(int);
}


//Creacion de paquetes

t_paquete_select* crear_paquete_select(t_instruccion_lql instruccion){
	t_paquete_select* paquete_select = malloc(sizeof(t_paquete_select));
	paquete_select->codigo_operacion = instruccion.operacion;
	paquete_select->key = instruccion.parametros.SELECT.key;
	char* nombre_tabla = instruccion.parametros.SELECT.tabla;
	paquete_select->nombre_tabla = malloc(sizeof(t_buffer));
	paquete_select->nombre_tabla->size = strlen(nombre_tabla)+1;
	paquete_select->nombre_tabla->palabra = malloc(paquete_select->nombre_tabla->size);
	memcpy(paquete_select->nombre_tabla->palabra, nombre_tabla, paquete_select->nombre_tabla->size);
	return paquete_select;
}

t_paquete_create* create_paquete_create(t_instruccion_lql instruccion) {
	t_paquete_create* paquete_create = malloc(sizeof(t_paquete_create));
	paquete_create->codigo_operacion = instruccion.operacion;
	paquete_create->consistencia = instruccion.parametros.CREATE.consistencia;
	paquete_create->num_particiones = instruccion.parametros.CREATE.num_particiones;
	paquete_create->tiempo_compac = instruccion.parametros.CREATE.compactacion_time;
	char* nombre_tabla = instruccion.parametros.CREATE.tabla;
	paquete_create->nombre_tabla = malloc(sizeof(t_buffer));
	paquete_create->nombre_tabla->size = strlen(nombre_tabla)+1;
	paquete_create->nombre_tabla->palabra = malloc(paquete_create->nombre_tabla->size);
	memcpy(paquete_create->nombre_tabla->palabra, nombre_tabla, paquete_create->nombre_tabla->size);
	return paquete_create;
}

char* serializar_paquete_select(t_paquete_select* paquete_select, int bytes)
{
	char * serializado = malloc(bytes);
	int desplazamiento = 0;

	memcpy(serializado + desplazamiento, &(paquete_select->codigo_operacion), sizeof(t_operacion));
	desplazamiento+= sizeof(t_operacion);
	memcpy(serializado + desplazamiento, &(paquete_select->nombre_tabla->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(serializado + desplazamiento, paquete_select->nombre_tabla->palabra, paquete_select->nombre_tabla->size);
	desplazamiento+= paquete_select->nombre_tabla->size;
	memcpy(serializado + desplazamiento, &(paquete_select->key), sizeof(uint16_t));
		desplazamiento+= sizeof(uint16_t);
	return serializado;
}


t_paquete_select* deserializar_select (int socket_cliente){
	t_paquete_select* consulta_select = malloc(sizeof(t_paquete_select));
	consulta_select->nombre_tabla = malloc(sizeof(t_buffer));
	recv(socket_cliente,  &(consulta_select->nombre_tabla->size), sizeof(int), MSG_WAITALL);
	int size_nombre_tabla = consulta_select->nombre_tabla->size;
	char * nombre_tabla = malloc(size_nombre_tabla);
	//consulta_select->nombre_tabla->palabra = malloc(consulta_select->nombre_tabla->size);
	recv(socket_cliente, nombre_tabla, size_nombre_tabla, MSG_WAITALL);
	memcpy(consulta_select->nombre_tabla->palabra ,nombre_tabla, size_nombre_tabla );
	recv(socket_cliente,  &(consulta_select->key), sizeof(uint16_t), MSG_WAITALL);

	return consulta_select;//Acordarse de hacer un free despues de usarse
}

