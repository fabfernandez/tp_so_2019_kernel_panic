/*
 * protocolos.c
 *
 *  Created on: 13 abr. 2019
 *      Author: utnso
 */


#include "protocolos.h"

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

void* serializar_paquete_select(t_paquete_select* paquete_select, int bytes)
{
	void * serializado = malloc(bytes);
	int desplazamiento = 0;

	memcpy(serializado + desplazamiento, &(paquete_select->codigo_operacion), sizeof(t_operacion));
	desplazamiento+= sizeof(int);
	memcpy(serializado + desplazamiento, &(paquete_select->nombre_tabla->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(serializado + desplazamiento, paquete_select->nombre_tabla->palabra, paquete_select->nombre_tabla->size);
	desplazamiento+= paquete_select->nombre_tabla->size;
	memcpy(serializado + desplazamiento, &(paquete_select->key), sizeof(uint16_t));
		desplazamiento+= sizeof(uint16_t);
	return serializado;
}
