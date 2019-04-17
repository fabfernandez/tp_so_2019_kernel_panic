/*
 * protocolos.h
 *
 *  Created on: 13 abr. 2019
 *      Author: utnso
 */

#ifndef GLOBAL_PROTOCOLOS_H_
#define GLOBAL_PROTOCOLOS_H_

#include<stdint.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include <stdbool.h>


typedef enum consistencias{
	STRONG, STRONG_HASH, EVENTUAL
}t_consistencia;

struct memoriaGossip {
	int descriptorMemoria;
	char* IP;
	char* PUERTO;
	t_consistencia consistencia;
	t_estado estado;
};
typedef enum estado {
	CONECTADA, DESCONECTADA
}t_estado;

struct tablaMemoriaGossip {
	struct memoriaGossip memoria;
	struct tablaMemoriaGossip* siguiente;
};
struct tablaMemoriaGossip crearTablaGossip(){
	struct tablaMemoriaGossip elementoCreado = malloc(sizeof(struct tablaMemoriaGossip));
	//-- CREAR PRIMER ELEMENTO(MEMORIA A, LA PRIEMRA QUE SE LEVANTA, HAY QUE PASAR POR PARAMETROS LOS DATOS DE LA MISMA--//
	return &elementoCreado;
}

struct tablaMemoriaGossip crearElementoParaTablaDeConsistencias(struct tablaMemoriaGossip tabla){ // hay que pasar por parametro un struct con los datos de la nueva memoria
	struct memoriaGossip memogossip = malloc(sizeof(struct memoriaGossip));
	struct tablaMemoriaGossip elementoCreado = malloc(sizeof(struct tablaMemoriaGossip));
	// memogossip.descriptorMemoria = (seteo descriptor de la nueva memoria)
	// memogossip.IP = (seteo ip de la nueva memoria)
	// memogossip.PUERTO = (seteo puerto de la nueva memoria)
	// memogossip.consistencia = (seteo consistencia)
	// memogossip.estado= (seteo estado)
	// memogossip.siguiente=null;
	elementoCreado.memoria = &memogossip;
	elementoCreado.siguiente= NULL;
	return &elementoCreado;
}

void agregarMemoriaALaTablaGossip(struct tablaMemoriaGossip tabla, struct tablaMemoriaGossip elementoAAgregar){
	if(tabla->siguiente == NULL){
		tabla->siguiente=&elementoAAgregar;
	} else { agregarMemoriaALaTablaGossip(tabla->siguiente,elementoAAgregar); };
}

typedef enum operaciones {
	INSERT,SELECT,CREATE,DESCRIBE,DROP, JOURNAL,ADD,METRICS,RUN, HANDSHAKE
}t_operacion;

typedef struct
{
	int size;
	char* palabra;
} t_buffer;



typedef struct
{
	t_operacion codigo_operacion;
	t_buffer* nombre_tabla;
	uint16_t key;
	t_buffer* valor;

} t_paquete_insert;

typedef struct
{
	t_operacion codigo_operacion;
	t_buffer* nombre_tabla;
	t_consistencia consistencia;
	uint16_t num_particiones;
	long tiempo_compac;

} t_paquete_create;

typedef struct
{
	t_operacion codigo_operacion;
	uint16_t num_memoria;
	t_consistencia consistencia;

} t_paquete_add;

typedef struct
{
	t_operacion codigo_operacion;
	t_buffer* nombre_tabla;
	uint16_t key;

} t_paquete_select;

typedef struct
{
	t_operacion codigo_operacion;
	t_buffer* nombre_tabla;

} t_paquete_drop_describe;


typedef struct {
	bool valido;
	t_operacion operacion;
	union {
		struct {
			char* tabla;
			long timestamp;
			uint16_t key;
			char* value;
		} INSERT;
		struct {
			char* tabla;
			uint16_t key;
		} SELECT;
		struct {
			char* tabla;
			t_consistencia consistencia;
			uint16_t num_particiones;
			uint16_t compactacion_time;
		} CREATE;
		struct {
			char* tabla;
		} DESCRIBE;
		struct {
			char* tabla;
		} DROP;
		struct {
			int numero_memoria;
			t_consistencia consistencia;
		} ADD;
		struct {
			char * path_script;
		} RUN;

	} parametros;
	char** _raw; //Para uso de la liberación
} t_instruccion_lql;


#endif /* GLOBAL_PROTOCOLOS_H_ */
