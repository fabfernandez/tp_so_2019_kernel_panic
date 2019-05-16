/*
 * memoria.h
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <stdint.h>
#include <time.h>
#include <global/utils.h>
#include <global/protocolos.h>


							// ******* TIPOS NECESARIOS ******* //
// prueba select //
	 fd_set master;   // conjunto maestro de descriptores de fichero
	 fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	 int fdmax,fdmin;        // número máximo de descriptores de fichero
	 //server_memoria  // descriptor de socket a la escucha
	 int memoriaNuevaAceptada;        // descriptor de socket de nueva conexión aceptada
//
t_list* tablas;
void agregar_pagina_a_tabla(pagina* pagina,char* nombreTabla);
pagina* crearPagina(uint16_t key, char* value, char* tabla, long timestamp);
void agregarEnMemoriaElRegistro(char* key,char* value,long timestamp);
void* memoria_principal;
int nbytes;
int primeraVuelta = 0;
pthread_t thread_gossiping;
t_list tablaGossiping;
t_log* logger;
t_config* archivoconfig;
int socket_conexion_lfs;
int cantidad_paginas;
int tamanio_pagina;
int posicionProximaLibre=0;
int max_value;
char* ip_memoria;
char* puerto_memoria;
char* nombre_memoria;
int tamanio_memoria;
char* ip__lfs;
char* puerto__lfs;
char** puertosSeeds;
char** seeds;
int server_memoria;
int socket_kernel_conexion_entrante;
typedef char* t_valor;	//valor que devuelve el select
char** levantarSeeds();
char** levantarPuertosSeeds();
void select_esperar_conexiones_o_peticiones();
void seedsCargadas();
void logearSeeds();
void levantar_datos_memoria();
void levantar_datos_lfs();
int esperar_operaciones();
void iniciarTablaDeGossiping();
void resolver_operacion(int socket_memoria, t_operacion cod_op);
void leer_config();
void iniciar_logger();
void iniciar_servidor_memoria_y_esperar_conexiones_kernel();
void intentar_handshake_a_lfs(int alguien);
int insert(char* tabla, uint16_t key, long timestamp);
void enviar_paquete_select(int socket_envio, t_paquete_select* consulta_select);
void recibir_datos(t_log* logger,int socket_fd);
void recibir_max_value(t_log* logger, int socket_cliente);
segmento* crearSegmento(char* nombreTabla);
t_valor select_(char* tabla, uint16_t key);

int drop(char* tabla);

int create(char* tabla, t_consistencia consistencia, int maximo_particiones, long tiempo_compactacion);

t_metadata describe(char* tabla);

int journal(void);

#endif /* MEMORIA_H_ */
