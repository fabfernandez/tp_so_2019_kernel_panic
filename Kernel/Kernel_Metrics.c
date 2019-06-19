/*
 * Kernel_Metrics.c


 */

#include "Kernel_Metrics.h"


void iniciar_hilo_metrics(){
	pthread_t hilo_metrics;

	if(pthread_create(&hilo_metrics, 0, loguear_y_borrar, NULL) !=0){
		log_error(logger, "Error al crear el hilo");
	}
	if(pthread_detach(hilo_metrics) != 0){
		log_error(logger, "Error al crear el hilo");
	}
}



void loguear_y_borrar(){
	log_metrics = crear_log("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Kernel/Kernel_Metrics.log");

	while(1){
		sleep(30);

		loguear();
		borrar();
	}
}

void loguear(){
	t_metrics* metrica_uno;

	for(int i = 0; i < list_size(metricas); i++){
		metrica_uno = list_get(metricas, i);
		log_info(log_metrics, "En la memoria %d el tiempo promedio de un SELECT fue de %d en los ultimos 30 segundos", metrica_uno->nombre_memoria, metrica_uno->read_latency);
		log_info(log_metrics, "En la memoria %d el tiempo promedio de un INSERT fue de %d en los ultimos 30 segundos", metrica_uno->nombre_memoria, metrica_uno->write_latency);
		log_info(log_metrics, "En la memoria %d la cantidad de SELECT fue de %d en los ultimos 30 segundos", metrica_uno->nombre_memoria, metrica_uno->cant_reads);
		log_info(log_metrics, "En la memoria %d la cantidad de INSERT fue de %d en los ultimos 30 segundos", metrica_uno->nombre_memoria, metrica_uno->cant_writes);
		log_info(log_metrics, "En la memoria %d el MEMORY LOAD fue de %d", metrica_uno->nombre_memoria, metrica_uno->memory_load);
	}
}

void borrar(){
	for(int i= 0; i < list_size(metricas); i++){
		free(list_remove(metricas, i));
	}
}

t_metrics* obtener_nodo_metricas(int nombre_memoria){
	int existe_nodo(t_metrics* nodo_metrica){
		return nodo_metrica->nombre_memoria == nombre_memoria;
	}

	t_metrics* nodo_metrica = list_find(metricas, (void*) existe_nodo);
	if(nodo_metrica == NULL){
		nodo_metrica = malloc(sizeof(t_metrics));
		nodo_metrica->cant_operaciones_totales = 0;
		nodo_metrica->read_latency = 0;
		nodo_metrica->write_latency = 0;
		nodo_metrica->cant_reads = 0;
		nodo_metrica->cant_writes = 0;
		nodo_metrica->memory_load = 0;
		nodo_metrica->nombre_memoria = nombre_memoria;
		list_add(metricas, nodo_metrica);
	};
	return nodo_metrica;
}

void registrar_metricas(int nombre_memoria, int diferencia_timestamp){
	t_metrics* metrica = obtener_nodo_metricas(nombre_memoria);

	metrica->cant_operaciones_totales++;
	metrica->cant_reads++;
	metrica->read_latency += diferencia_timestamp;
}

void registrar_metricas_insert(int nombre_memoria, int diferencia_timestamp){
	t_metrics* metrica = obtener_nodo_metricas(nombre_memoria);

	metrica->cant_operaciones_totales++;
	metrica->cant_writes++;
	metrica->write_latency += diferencia_timestamp;
}

