/*
 * memoria.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
#include "memoria.h"
int main(void)

{

	 FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	 FD_ZERO(&read_fds);
	/*
	 * INICIO LOGGER, CONFIG, LEVANTO DATOS E INICIO SERVER
	 */
	iniciar_logger();
	leer_config();

	/*
	 * HAY QUE CAMBIAR RUTA A UNA VARIABLE PARA PODER LEVANTAR MEMORIAS CON DIFERENTES CONFIGS
	 */
	levantar_datos_memoria();

	char* memoria_principal = (char*) malloc(tamanio_memoria*sizeof(char));
	log_info(logger,"Se reservaron %i bytes para la memoria", (tamanio_memoria*sizeof(char)));
	tablas = list_create();
	log_info(logger,"Tabla de segmentos creada, cantidad actual: %i ", tablas->elements_count);

	levantar_datos_lfs();

	/*
	 * obtener socket a la escucha
	 */
	server_memoria = iniciar_servidor(ip_memoria, puerto_memoria);
	seeds = levantarSeeds();
	puertosSeeds = levantarPuertosSeeds();
	seedsCargadas();

	/*
	 * INICIO LA TABLA DE GOSSIPING AGREGANDO LA MEMORIA ACTUAL <- falta
	 */
	//iniciarTablaDeGossiping();																					//
	//pthread_create(&thread_gossiping, NULL, &iniciarGossip, &tablaGossiping, &primeraVuelta);						// CREO THREAD DE GOSSIPING

	/*
	 * ME CONECTO CON LFS E INTENTO UN HANDSHAKE -----------------------INTENTA CONECTARSE, SI NO PUEDE CORTA LA EJECUCION
	 */
	socket_conexion_lfs = crear_conexion(ip__lfs,puerto__lfs); //
	if(socket_conexion_lfs != -1){
	log_info(logger,"Creada la conexion para LFS %i", socket_conexion_lfs);
	intentar_handshake_a_lfs(socket_conexion_lfs); 										// diferente al handshake normal SE SETEA TAMANIO_PAGINA y CANTIDAD_PAGINAS
	log_info(logger, "Conexion exitosa con con %i", socket_memoria);
	log_info(logger,"El tamaño de cada pagina sera: %i", tamanio_pagina);
	log_info(logger,"La cantidad de paginas en memoria principal será: %i", cantidad_paginas);
	//void* tabla_paginas =  se crean dinamicamentes, hay que reservar espacio para tabla de segmentos
	} else {
		log_info(logger,"No se pudo realizar la conexion con LFS. Abortando.");
		log_info(logger, "Se liberaran %i bytes de la memoria",(tamanio_memoria*sizeof(char)));
		free(memoria_principal);
		return -1;

	}
/*
 * PRUEBA PARA SELECT
 */
	list_add(tablas, crearSegmento("Apellido"));
	list_add(tablas, crearSegmento("Nombre"));
	list_add(tablas, crearSegmento("DNI"));
	list_add(tablas, crearSegmento("Genero"));
	list_add(tablas, crearSegmento("Algo"));
	list_add(tablas, crearSegmento("Algo2"));

	paginaNueva(123,"Gon",125478,"DNI",memoria_principal,tablas); 					// si no le paso la memoria principal por parametro me hace un segmentation fault.
	sleep(1);
	paginaNueva(273,"Faba",15478,"Nombre",memoria_principal,tablas);
	sleep(1);
	paginaNueva(113,"Juan",15478,"Apellido",memoria_principal,tablas);
	sleep(1);
	paginaNueva(213,"Flor",15478,"Apellido",memoria_principal,tablas);
	sleep(1);
	paginaNueva(113,"Delfi",15478,"Genero",memoria_principal,tablas);
	sleep(1);
	paginaNueva(213,"Juana",15478,"Genero",memoria_principal,tablas);
	sleep(1);
	paginaNueva(113,"Vito",15478,"DNI",memoria_principal,tablas);
	sleep(1);
	paginaNueva(213,"Pess",15478,"Nombre",memoria_principal,tablas);

	segmento* unS = encontrarSegmento("Apellido",tablas);
	segmento* unS2 = encontrarSegmento("Nombre",tablas);
	segmento* unS3 = encontrarSegmento("DNI",tablas);
	segmento* unS4 = encontrarSegmento("Genero",tablas);
	segmento* unS5 = encontrarSegmento("Algo",tablas);
	segmento* unS6 = encontrarSegmento("Algo2",tablas);

	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS->nombreTabla, unS->paginas->elements_count);
	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS2->nombreTabla, unS2->paginas->elements_count);
	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS3->nombreTabla, unS3->paginas->elements_count);
	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS4->nombreTabla, unS4->paginas->elements_count);
	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS5->nombreTabla, unS5->paginas->elements_count);
	log_info(logger,"SEGMENTO ENCONTRADO: %s , con %i elementos.", unS6->nombreTabla, unS6->paginas->elements_count);

	/*
	pagina_concreta* paginaM1= traerPaginaDeMemoria(0,memoria_principal);
	pagina_concreta* paginaM2= traerPaginaDeMemoria(1,memoria_principal);
	pagina_concreta* paginaM3= traerPaginaDeMemoria(2,memoria_principal);
	pagina_concreta* paginaM4= traerPaginaDeMemoria(3,memoria_principal);
	pagina_concreta* paginaM5= traerPaginaDeMemoria(4,memoria_principal);
	pagina_concreta* paginaM6= traerPaginaDeMemoria(5,memoria_principal);
	pagina_concreta* paginaM7= traerPaginaDeMemoria(6,memoria_principal);
	pagina_concreta* paginaM8= traerPaginaDeMemoria(7,memoria_principal);

	log_info(logger,"La key es: %i", paginaM1->key);
	log_info(logger,"El ts es: %i", paginaM1->timestamp);
	log_info(logger,"El value es: %s", paginaM1->value);

	log_info(logger,"La key es: %i", paginaM2->key);
	log_info(logger,"El ts es: %i", paginaM2->timestamp);
	log_info(logger,"El value es: %s", paginaM2->value);

	log_info(logger,"La key es: %i", paginaM3->key);
	log_info(logger,"El ts es: %i", paginaM3->timestamp);
	log_info(logger,"El value es: %s", paginaM3->value);

	log_info(logger,"La key es: %i", paginaM4->key);
	log_info(logger,"El ts es: %i", paginaM4->timestamp);
	log_info(logger,"El value es: %s", paginaM4->value);

	log_info(logger,"La key es: %i", paginaM5->key);
	log_info(logger,"El ts es: %i", paginaM5->timestamp);
	log_info(logger,"El value es: %s", paginaM5->value);

	log_info(logger,"La key es: %i", paginaM6->key);
	log_info(logger,"El ts es: %i", paginaM6->timestamp);
	log_info(logger,"El value es: %s", paginaM6->value);

	log_info(logger,"La key es: %i", paginaM7->key);
	log_info(logger,"El ts es: %i", paginaM7->timestamp);
	log_info(logger,"El value es: %s", paginaM7->value);

	log_info(logger,"La key es: %i", paginaM8->key);
	log_info(logger,"El ts es: %i", paginaM8->timestamp);
	log_info(logger,"El value es: %s", paginaM8->value);

	free(paginaM1->value);
	free(paginaM1);
	free(paginaM2->value);
	free(paginaM2);
	free(paginaM3->value);
	free(paginaM3);
	free(paginaM4->value);
	free(paginaM4);
	free(paginaM5->value);
	free(paginaM5);		// siempre que uso traerPaginaDeMemoria tengo que liberar la memoria
	free(paginaM6->value);
	free(paginaM6);
	free(paginaM7->value);
	free(paginaM7);
	free(paginaM8);
	*/
	/*
	int eg = buscarRegistroEnTabla("Nombre",273,memoria_principal, tablas);
	log_info(logger," **** POSICION %i ****", eg);
	/*pagina_concreta* paginacc = traerPaginaDeMemoria(eg,memoria_principal);
	log_info(logger,"La key es: %i", paginacc->key);
	log_info(logger,"El ts es: %i", paginacc->timestamp);
	log_info(logger,"El value es: %s", paginacc->value);
	free(paginacc);*/
	log_info(logger,"**** CANTIDAD DE SEGMENTOS: %i ****", tablas->elements_count);
//	int leo=buscarRegistroEnTabla("DNI",123,memoria_principal,tablas);
	int ultimaLeida = lru(memoria_principal,tablas); // tiene que ser el primer registro agregado
	log_info(logger,"**** REGISTRO MENOS LEIDO, EL QUE HACE MAS TIEMPO NO SE LEE: %i ****", ultimaLeida);
	pagina_concreta* paginacc2 = traerPaginaDeMemoria(ultimaLeida,memoria_principal);
	log_info(logger,"La key es: %i", paginacc2->key);
	log_info(logger,"El ts es: %i", paginacc2->timestamp);
	log_info(logger,"El value es: %s", paginacc2->value);
	free(paginacc2);
	sleep(2);













//	iniciar_servidor_memoria_y_esperar_conexiones_kernel();
	select_esperar_conexiones_o_peticiones(memoria_principal,tablas);
//	esperar_operaciones(socket_kernel_conexion_entrante);
//	terminar_programa(socket_kernel_fd, conexionALFS); // termina conexion, destroy log y destroy config.
	return EXIT_SUCCESS;
}

/*
			######## #### ##    ##    ########  ######## ##          ##     ##    ###    #### ##    ##
			##        ##  ###   ##    ##     ## ##       ##          ###   ###   ## ##    ##  ###   ##
			##        ##  ####  ##    ##     ## ##       ##          #### ####  ##   ##   ##  ####  ##
			######    ##  ## ## ##    ##     ## ######   ##          ## ### ## ##     ##  ##  ## ## ##
			##        ##  ##  ####    ##     ## ##       ##          ##     ## #########  ##  ##  ####
			##        ##  ##   ###    ##     ## ##       ##          ##     ## ##     ##  ##  ##   ###
			##       #### ##    ##    ########  ######## ########    ##     ## ##     ## #### ##    ##
*/
/**
 	* @NAME: buscarSegmento
 	* @DESC: Busca un segmento y lo logea
 	*
 	*/
void buscarSegmento(char* segment,t_list* tablas){
		segmento* unS = encontrarSegmento(segment, tablas);
		log_info(logger,"SEGMENTO ENCONTRADO: %s", unS->nombreTabla);
	}
/**
 	* @NAME: traerPaginaDeMemoria
 	* @DESC: pasando una posicion y la memoria devuelve el dato contenido en el mismo
 	*
 	*/
pagina_concreta* traerPaginaDeMemoria(unsigned int posicion,char* memoria_principal){
	pagina_concreta* pagina= malloc(sizeof(pagina_concreta));
	memcpy(&(pagina->key), &memoria_principal[posicion*tamanio_pagina], sizeof(uint16_t));
	memcpy(&(pagina->timestamp), &memoria_principal[posicion*tamanio_pagina+sizeof(uint16_t)], sizeof(long));
	pagina->value = malloc(20);
	strcpy(pagina->value, &memoria_principal[posicion*tamanio_pagina+sizeof(uint16_t)+sizeof(long)]);
	return pagina;
	}
void traerPaginaDeMemoria2(unsigned int posicion,char* memoriappal,pagina_concreta* pagina){
	//pagina_concreta* pagina= malloc(sizeof(pagina_concreta));
	memcpy(&(pagina->key), &memoriappal[posicion*tamanio_pagina], sizeof(uint16_t));
	memcpy(&(pagina->timestamp), &memoriappal[posicion*tamanio_pagina+sizeof(uint16_t)], sizeof(long));
	pagina->value = malloc(20);
	strcpy(pagina->value, &memoriappal[posicion*tamanio_pagina+sizeof(uint16_t)+sizeof(long)]);
	//return pagina;
	}

/**
 	* @NAME: crearPagina
 	* @DESC: crea una pagina con una key y le asigna una posicion de memoria SE USA ADENTRO DE paginaNueva
 	*
 	*/
pagina* crearPagina(){
	pagina* paginaa = malloc(sizeof(pagina));
	paginaa->modificado=0;
	paginaa->posicionEnMemoria=posicionProximaLibre;
	paginaa->ultimaLectura=(unsigned)time(NULL);;
	posicionProximaLibre+=1;
	return paginaa;
}
pagina* crearPaginaInsert(){
	pagina* paginaa = malloc(sizeof(pagina));
	paginaa->modificado=1;
	paginaa->posicionEnMemoria=posicionProximaLibre;
	paginaa->ultimaLectura=(unsigned)time(NULL);;
	log_info(logger,"Se creo una pagina con insert, bit MOD=%i", paginaa->modificado);
	posicionProximaLibre+=1;
	return paginaa;
}


/**
 	* @NAME: paginaNueva
 	* @DESC: crea una pagina en la tabla de paginas(bit,key,posicion) y la vuelca en memoria(asumiendo que existe el segmento) PUES RECIBE LOS DATOS DEL LFS (select)
 	* NOTA: DATOS EN MEMORIA KEY-TS-VALUE
 	*/
void paginaNueva(uint16_t key, char* value, long ts, char* tabla, char* memoria,t_list* tablas){
	pagina* pagina = crearPagina();	// deberia ser con malloc?
	agregarPaginaASegmento(tabla,pagina,tablas);
	//log_info(logger,"POSICION EN MMORIA: %i", pagina->posicionEnMemoria);
	memcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina],&key,sizeof(uint16_t)); 					//deberia ser &key? POR ACA SEGMENTATION FAULT
	memcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina+sizeof(uint16_t)],&ts,sizeof(long));			// mismo que arriba
	strcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina+sizeof(uint16_t)+sizeof(long)], value);
	//log_info(logger,"POSICION PROXIMA EN MMORIA DISPONIBLE: %i", posicionProximaLibre);
	}
void paginaNuevaInsert(uint16_t key, char* value, long ts, char* tabla, char* memoria,t_list* tablas){
	pagina* pagina = crearPaginaInsert();	// deberia ser con malloc?
	agregarPaginaASegmento(tabla,pagina,tablas);
	//log_info(logger,"POSICION EN MMORIA: %i", pagina->posicionEnMemoria);
	memcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina],&key,sizeof(uint16_t)); 					//deberia ser &key? POR ACA SEGMENTATION FAULT
	memcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina+sizeof(uint16_t)],&ts,sizeof(long));			// mismo que arriba
	strcpy(&memoria[(pagina->posicionEnMemoria)*tamanio_pagina+sizeof(uint16_t)+sizeof(long)], value);
	//log_info(logger,"POSICION PROXIMA EN MMORIA DISPONIBLE: %i", posicionProximaLibre);
	}

/**
 	* @NAME: agregarPaginaASegmento
 	* @DESC: agrega una pagina a un segmento
 	*
 	*/
void agregarPaginaASegmento(char* tabla, pagina* pagina, t_list* tablas){
	segmento* segmentoBuscado = encontrarSegmento(tabla, tablas);
	list_add(segmentoBuscado->paginas, pagina);
	log_info(logger,"Se agrego la pagina con posicion en memoria: %i , al segmento: %s , con ultimo tiempo de lectura = %i", pagina->posicionEnMemoria, segmentoBuscado->nombreTabla, pagina->ultimaLectura);
}
/**
 	* @NAME: crearSegmento
 	* @DESC: crea un segmento y lo agrega a la lista de segmentos
 	*
 	*/
segmento* crearSegmento(char* nombreTabla)
	{
	segmento* segmento1 = malloc(sizeof(segmento));
	segmento1->paginas = list_create();
	segmento1->nombreTabla=strdup(nombreTabla);
	log_info(logger, "Se creo el segmento %s", segmento1->nombreTabla);
	return segmento1;
	}
/**
	 	* @NAME: buscarRegistroEnTabla
	 	* @DESC:  dada una solicitud select, con el nombre de la tabla y la key busca en memoria el dato.
	 	* Si el dato no está en memoria, o la tabla no está en memoria retorna -1, caso contrario retorna la posicion.
	 	*
	 	*
	 	*/

int buscarRegistroEnTabla(char* tabla, uint16_t key, char* memoria_principal,t_list* tablas){
		segmento* segment = encontrarSegmento(tabla,tablas);
		if(segment==NULL){return -1;}
		for(int i=0 ; i < segment->paginas->elements_count ; i++)
		{
			pagina* pagin = list_get(segment->paginas,i);
			uint16_t pos = pagin->posicionEnMemoria;
			log_info(logger,"Posicion en memoria: %i",pagin->posicionEnMemoria);
			pagina_concreta * pagc = malloc(sizeof(pagina_concreta));
			pagc = traerPaginaDeMemoria(pos,memoria_principal);
			log_info(logger,"Pagina Key: %i",pagc->key);
			int posicion=pagin->posicionEnMemoria;
			if(pagc->key == key){
				log_info(logger,"Ultima lectura anterior: %i",pagin->ultimaLectura);
				sleep(1); // si no hago sleep el ts es el mismo pq la ejecucion es super veloz jaja
				pagin->ultimaLectura=(unsigned)time(NULL);;
				log_info(logger,"TS: %i",(unsigned)time(NULL));
				log_info(logger,"Ultima lectura(actual): %i",pagin->ultimaLectura);
				free(pagc);
				return posicion;
			}
		}
		log_error(logger,"El registro no se encuentra en memoria");
		return -1;
}





/**
 	* @NAME: resolver_select
 	* @DESC: resuelve el select
 	*
 	*/
	void resolver_insert(int socket_kernel_fd, int socket_conexion_lfs,char* memoria_principal, t_list* tablas){
	t_paquete_insert* consulta_insert= deserealizar_insert(socket_kernel_fd);
	log_info(logger, "Se realiza INSERT");
	log_info(logger, "Se busca insertar en la tabla: %s", consulta_insert->nombre_tabla->palabra);
	log_info(logger, "en la key: %i", consulta_insert->key);
	log_info(logger, "el dato: %s", consulta_insert->valor->palabra);
	log_info(logger, "El ts es: %i", consulta_insert->ts);
	long tsss =consulta_insert->ts;
	char* tabla = consulta_insert->nombre_tabla->palabra;
	uint16_t key = consulta_insert->key;
	char* dato = consulta_insert->nombre_tabla->palabra;
	segmento* segmentoBuscado = encontrarSegmento(tabla,tablas);
	if(posicionProximaLibre+1>cantidad_paginas){
		/*
		 * PROCESO LRU
		 * proceso de creacion/modificacion de registro
		 */

	} else {
		if(segmentoBuscado==NULL)
		{
			log_info(logger, "No existe el segmento: $s", consulta_insert->nombre_tabla->palabra);
			list_add(tablas, crearSegmento(consulta_insert->nombre_tabla->palabra));
			paginaNuevaInsert(key,dato,tsss,tabla,memoria_principal,tablas);
		} else
			{
			int reg2 = buscarRegistroEnTabla(tabla, key,memoria_principal,tablas);
			if(reg2==-1)
				{
					log_info(logger, "El segmento existe y se encuentra en memoria, pero no la pagina");
					paginaNuevaInsert(key,dato,tsss,tabla,memoria_principal,tablas);
				} else
					{
					log_info(logger, "El registro se encuentra en la posicion de memoria: %i , se procede a modificarlo");
					//modificarRegistro(key,dato,(unsigned)time(NULL),reg2,memoria_principal,tablas);
					modificarRegistro(key,dato,tsss,reg2,memoria_principal,tablas);
					int resultado = cambiarBitModificado(tabla,key,tablas);
					}
			}
	}
	}

	int cambiarBitModificado(char* tabla, uint16_t key,t_list* tablas){
			segmento* segment = encontrarSegmento(tabla,tablas);
			if(segment==NULL){return -1;}
			for(int i=0 ; i < segment->paginas->elements_count ; i++)
			{
				pagina* pagin = list_get(segment->paginas,i);
				uint16_t pos = pagin->posicionEnMemoria;
				pagina_concreta * pagc = malloc(sizeof(pagina_concreta));
				pagc = traerPaginaDeMemoria(pos,memoria_principal);
				if(pagc->key == key){
					pagin->ultimaLectura=(unsigned)time(NULL);;
					pagin->modificado=(unsigned)time(NULL);;
					log_info(logger,"Pagina con posicion en memoria %i modificada, ultimo acceso: %i , Bit de MOD = %i",pagin->posicionEnMemoria,pagin->ultimaLectura, pagin->modificado);
					free(pagc);
					return 2;
				}
			}
			log_error(logger,"El registro no se encuentra en memoria");
			return -1;
	}
	void modificarRegistro(uint16_t key,char* dato,long tss,int posicion, char* memoria_principal, t_list* tablas){
		memcpy(&memoria_principal[posicion*tamanio_pagina],&key,sizeof(uint16_t)); 					//deberia ser &key? POR ACA SEGMENTATION FAULT
		memcpy(&memoria_principal[posicion*tamanio_pagina+sizeof(uint16_t)],&tss,sizeof(long));			// mismo que arriba
		strcpy(&memoria_principal[posicion*tamanio_pagina+sizeof(uint16_t)+sizeof(long)], dato);
	}
/**
 	* @NAME: resolver_select
 	* @DESC: resuelve el select
 	*
 	*/
	void resolver_select (int socket_kernel_fd, int socket_conexion_lfs,char* memoria_principal, t_list* tablas){
	t_paquete_select* consulta_select = deserializar_select(socket_kernel_fd);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);
	char* tabla = consulta_select->nombre_tabla->palabra;
	uint16_t key = consulta_select->key;
	int reg = 0;
	reg = buscarRegistroEnTabla(tabla, key,memoria_principal,tablas);
	log_info(logger, "registro numero: %i", reg);
	if(reg==-1){
		log_info(logger, "El registro con key '%d' NO se encuentra en memoria y procede a realizar la peticion a LFS", key);
		enviar_paquete_select(socket_conexion_lfs, consulta_select);
		eliminar_paquete_select(consulta_select);
	} else {
		log_info(logger, "El registro con key '%d' se encuentra en memoria en la posicion $i", key,reg);
		pagina_concreta* paginalala= traerPaginaDeMemoria(reg,memoria_principal);
		log_info(logger, "Se bajo de la memoria el registro: (%i,%s,%i)", paginalala->key, paginalala->value,paginalala->timestamp);
		log_info(logger, "Se procede a enviar el dato a kernel");
		free(paginalala->value);
		free(paginalala);
	}
	}

/**
 	* @NAME: iniciar_logger
 	* @DESC: Crea(lee,abre) el archivo de log
 	*
 	*/
	void iniciar_logger() {
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
	}
 /**
 	* @NAME: leer_config
 	* @DESC: lee(abre)el archivo config
 	*
 	*/
 	void leer_config() {
 	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config");
 	}
/**
	* @NAME: terminar_programa
	* @DESC: termina el programa
	*
	*/
 	void terminar_programa(int conexionKernel, int conexionALFS)
	{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
	}


/**
	* @NAME: resolver_operacion
	* @DESC: resuelve la operacion recibida
	*
	*/
	void resolver_operacion(int socket_memoria, t_operacion cod_op,char* memoria_principal, t_list* tablas){
	switch(cod_op)
				{
				case HANDSHAKE:
					log_info(logger, "Inicia handshake con %i", socket_memoria);
					recibir_mensaje(logger, socket_memoria);
					enviar_handshake(socket_memoria, "OK");
					log_info(logger, "Conexion exitosa con con %i", socket_memoria);


					break;
				case SELECT:
					log_info(logger, "%i solicitó SELECT", socket_memoria);
					resolver_select(socket_memoria, socket_conexion_lfs,memoria_principal,tablas);
					//aca debería enviarse el mensaje a LFS con SELECT
					break;
				case INSERT:
					log_info(logger, "%i solicitó INSERT", socket_memoria);
					resolver_select(socket_memoria, socket_conexion_lfs,memoria_principal,tablas);
					break;
				case CREATE:
					log_info(logger, "%i solicitó CREATE", socket_memoria);
					//aca debería enviarse el mensaje a LFS con CREATE
					break;
				case DESCRIBE:
					log_info(logger, "%i solicitó DESCRIBE", socket_memoria);
					//aca debería enviarse el mensaje a LFS con DESCRIBE
					break;
				case DROP:
					log_info(logger, "%i solicitó DROP", socket_memoria);
					//aca debería enviarse el mensaje a LFS con DROP
					break;
				case GOSSPING:
					log_info(logger, "La memoria %i solicitó GOSSIPING", socket_memoria);
						// chequearTablaYAgregarSiNoEsta(int de_quien);  // si me solicito gossiping ME ENVIO SU TABLA TB, asi que la chequeo toda y agrego las memorias que no tenga en mi tabla.
																		// le envio mi tabla de gossiping para que haga lo mismo.
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando conexion con %i", socket_memoria);
					break;
				default:
					log_warning(logger, "Operacion desconocida.");
					break;
				}
	}
/**
	* @NAME: levantar_datos_memoria
	* @DESC: levanto datos desde el archivo de configuracion(mi puerto, mi ip, mi nombre, mi tamaño)
	*
	*/
	void levantar_datos_memoria(){
	ip_memoria = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_memoria );
	puerto_memoria = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_memoria);
	nombre_memoria = config_get_string_value(archivoconfig, "NOMBRE_MEMORIA"); // asignamos NOMBRE desde CONFIG
	log_info(logger, "El nombre de la memoria es %s",nombre_memoria);
	tamanio_memoria = config_get_int_value(archivoconfig, "TAM_MEM");
	log_info(logger, "El tamaño de la memoria es: %i",tamanio_memoria);
	}

/**
	* @NAME: levantar_datos_lfs
	* @DESC: levanto datos desde el archivo de configuracion(puerto del lfs, ip del lfs)
	*
	*/
	void levantar_datos_lfs(){
	ip__lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP del LFS es %s", ip__lfs);
	puerto__lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto del LFS es %s", puerto__lfs);
	}
/**
	* @NAME: intentar_handshake_a_lfs
	* @DESC: se intenta hacer handshake con lfs
	*
	*/
	void intentar_handshake_a_lfs(int alguien){
	char *mensaje = "Hola me conecto soy la memoria: ";
		log_info(logger, "Trato de realizar un hasdshake");
		if (enviar_handshake(alguien,mensaje)){
			log_info(logger, "Se envió el mensaje %s", mensaje);

			recibir_datos(logger, alguien);
			log_info(logger,"Conexion exitosa con LFS");
		}
	}

/**
	* @NAME: recibir_datos
	* @DESC: recibe datos en el handshake
	*
	*/
	void recibir_datos(t_log* logger,int socket_fd){
	int cod_op = recibir_operacion(socket_fd);
	if (cod_op == HANDSHAKE){
		recibir_max_value(logger, socket_fd);
	}else{
		log_info(logger, "ERROR. No se realizó correctamente el HANDSHAKE");
	}
	}
/**
	* @NAME: recibir_max_value
	* @DESC: usada en el handsake para recibir el max_value que es el valor maximo de value(determina tamanio_pagina)
	*
	*/
	void recibir_max_value(t_log* logger, int socket_cliente)
	{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "El tamaño maximo de value es %s", buffer);
	max_value = atoi(buffer);
	tamanio_pagina=sizeof(uint16_t)+(sizeof(char)*max_value)+sizeof(long);
	cantidad_paginas = tamanio_memoria/tamanio_pagina;
	log_info(logger, "Luego el tamaño de la pagina será %i", tamanio_pagina);
	log_info(logger, "La memoria de %i b, tendrá %i paginas", tamanio_memoria, tamanio_memoria/tamanio_pagina);
	free(buffer);
	}

/**
	* @NAME: iniciar_servidor_memoria_y_esperar_conexiones_kernel
	* @DESC:
	*
	*/
	void iniciar_servidor_memoria_y_esperar_conexiones_kernel(){
	log_info(logger, "Memoria lista para recibir a peticiones de Kernel");
	log_info(logger, "Memoria espera peticiones");
	socket_kernel_conexion_entrante = esperar_cliente(server_memoria);
	log_info(logger, "Memoria se conectó con Kernel");
	}

/**
	* @NAME: iniciarTablaDeGossiping
	* @DESC: inicia la tabla de gossiping
	*
	*/
	void iniciarTablaDeGossiping(){
	}
/**
	* @NAME: levantarPuertosSeeds
	* @DESC: levanta las seeds
	*
	*/
	char** levantarSeeds(){
	return config_get_array_value(archivoconfig, "IP_SEEDS");
	}
/**
	* @NAME: levantarPuertosSeeds
	* @DESC: levanta los puertos de las seeds
	*
	*/
	char** levantarPuertosSeeds(){
	return config_get_array_value(archivoconfig, "PUERTO_SEEDS");
	}
/**
	* @NAME: iniciarGossip
	* @DESC: inicia proceso de gossiping
	*
	*/
	void iniciarGossip(struct tablaMemoriaGossip* tabla, int contador){ 	// VOY A HAER OSSIPING CON LAS MEMORIAS(PREGUNTARLES QUE MEMORIAS CONECTADAS CONOCE)
	int pasada = contador;
	if(pasada==0){ 														// PRIMER PASADA TIENE UN SLEEP DE 4 UNIDADES DE TIEMPO, SE VA A REPETIR EL GOSSIPING HASTA QUE TERMINE EL PROCESO ED LA MEMORIA
		while(1){
			sleep(4);													// ESPERO 4 UNIDADES
			if(tabla->siguiente!=NULL) 									// HAY MEMORIAS POR RECORRER EN LA TABLA APARTE DE LA QUE YA ESTOY LEYENDO?
				{ 														// SI HAY UNA MEMORIA PENDIENTE, LEO
				//realizarGossipingConUnaMemoria(MEMORIA); 				// no deberia hacer gossiping con ella misma(primer elemento de la tabla es la memoria creadora de la tabla)
				struct tablaMemoriaGossip* proximo = tabla->siguiente;
				iniciarGossip(proximo,1); 								// LLAMO A GOSSIP PARA QUE HAGA GOSSIPING CON LA MEMORIA QUE VIENE, PERO CON 1, ES DECIR, SIN DORMIR EL PROCESO PUES ES PARTE DE ESTE CICLO ACTUAL DE GOSSIPING.
				}
			else{ 														// SI NO HAY UNA MEMORIA PENDIENTE, HAGO ULTIMO GOSSIPING DEL CICLO
				//realizarGossipingConUnaMemoria(tabla->memoria.descriptorMemoria);
				}
		}
	}
	else

	{
		if(tabla->siguiente!=NULL) // ENTRE POR ACÁ POR QUE ESTOY LEYENDO MINIMAMENTE UNA SEGUNDA MEMORIA EN UN MISMO CICLO DE GOSSIP(SIN DORMIR)
						{ 									// SI HAY UNA MEMORIA PENDIENTE, LEO SIGUIENTE Y HAGO GOSSIP CON LA ACTUAL
						//realizarGossipingConUnaMemoria(tabla->memoria);
						struct tablaMemoriaGossip* proximo = tabla->siguiente;
						iniciarGossip(proximo,1);
						}
					else{ 									// SI NO HAY UNA MEMORIA PENDIENTE, HAGO ULTIMO GOSSIPING DEL CICLO
						//realizarGossipingConUnaMemoria(tabla->memoria);
						}
		}
	}
/**
	* @NAME: seedsCargadas
	* @DESC: logea las seeds cargadas
	*
	*/
	void seedsCargadas(){
	int i=0;
	while(seeds[i]!= NULL){
		log_info(logger, "Se obtuvo la seed a memoria con ip: %s , y puerto: %s",seeds[i],puertosSeeds[i]);
		i++;
	}
	log_info(logger, "Se obtuvieron %i seeds correctamente.",i);
	}

/**
	* @NAME: select_esperar_conexiones_o_peticiones
	* @DESC:
	*
	*/
	void select_esperar_conexiones_o_peticiones(char* memoria_principal,t_list* tablas){
	FD_SET(server_memoria, &master); // agrego el socket de esta memoria(listener, está a la escucha)al conjunto maestro
	fdmin = server_memoria;
	fdmax = server_memoria; // por ahora el socket de mayor valor es este, pues es el unico ;)
	log_info(logger,"A la espera de nuevas solicitudes");
	for(;;) 	// bucle principal
	{
		read_fds = master; // cópialo
	    	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
	    		{
	    			log_error(logger, "Fallo accion select.");;
	                exit(1);
	            }
	            // explorar conexiones existentes en busca de datos que leer
	            for(int i = 0; i <= fdmax; i++) {
	                if (FD_ISSET(i, &read_fds)) //  pregunta si "i" está en el conjunto ¡¡tenemos datos!! read_fds es como una lista de sockets con conexiones entrantes
	                	{
	                    	if (i == server_memoria) // si estoy parado en el socket que espera conexiones nuevas (listen)
	                    		{
	                    		memoriaNuevaAceptada = esperar_cliente(server_memoria);
	                    		FD_SET(memoriaNuevaAceptada, &master); // añadir al conjunto maestro
	                    		if (memoriaNuevaAceptada > fdmax)
	                    			{    // actualizar el máximo
	                    			fdmax = memoriaNuevaAceptada;
	                    			}
	                            log_info(logger,"Nueva conexion desde %i",memoriaNuevaAceptada);
	                    		} else 				// // gestionar datos de un cliente
	                        	{
	                    			int cod_op;
	                    			nbytes = recv(i, &cod_op, sizeof(int), 0);
	                    			if (nbytes <= 0) {
	                    			                            // error o conexión cerrada por el cliente
	                    			                            if (nbytes == 0) {
	                    			                            // conexión cerrada
	                    			                            	log_error(logger, "el cliente se desconecto. Terminando conexion con %i", i);
	                    			                            } else {
	                    			                                perror("recv");
	                    			                            }
	                    			                            close(i); // bye!
	                    			                            FD_CLR(i, &master); // eliminar del conjunto maestro
	                    			                        }
	                    			else {
	                    			                            // tenemos datos de algún cliente
	                    				resolver_operacion(i,cod_op,memoria_principal,tablas);
	                    			    log_info(logger, "Se recibio una operacion de %i", i);

	                    			}}
	                	}
	            }
	}
	}


	/**
		* @NAME: lru
		* @DESC: Se ejecuta el algoritmo de sustitucion, recorre todas las paginas, si encuentra
		*  paginas sin modificar las compara y devuelve la que tenga el ts mas antiguo, si no
		*  devuelve -1, lo que significa que estan todas las paginas modificadas y hay que hacer un journal.
		*
		*/
		int lru(char* memoria_principal, t_list* tablas){
				unsigned int posicionMemo=0;
				long ultimaLectura=-1;
				int segment=0;
				int pagg=0;
				log_info(logger, "[TABLAS= %i ]",tablas->elements_count);
				for(int i=0; i<(tablas->elements_count); i++){	// recorro segmentos
						segmento* segmente = list_get(tablas, i);
						log_info(logger, "[Ciclo segmento %i , cant pags= %i ]",i, segmente->paginas->elements_count);
						for(int j=0; j<segmente->paginas->elements_count; j++) {// en un segmento i, recorro sus paginas
							pagina* pag = list_get(segmente->paginas, j);		// traigo pagina
							if(ultimaLectura==-1 && pag->modificado==0)
							{			// si es la primer pagina que leo sin modificar
								posicionMemo=pag->posicionEnMemoria;			// asigno la posicion de memoria para retornarla
								ultimaLectura=pag->ultimaLectura;				// guardo su ultima lectura para seguir comparando.
								log_info(logger, "[PM=-1 // PRIMER VUELTA J=%i]Ultima lectura %i",j,ultimaLectura);
								log_info(logger, "[PM=-1 // PRIMER VUELTA J=%i]Posicion memo %i",j,posicionMemo);
								segment=i;										// determino el segmento y
								pagg=j;											// la pagina para despues poder eliminarla de mis tablas

							} else log_info(logger, "NO CUMPLE");
							if(pag->modificado==0)
							{// si no es la primer pagina que leo sin modificar y aparte hace mas tiempo que no se lee
								if(pag->ultimaLectura<ultimaLectura){
								posicionMemo=pag->posicionEnMemoria;							// asigo la posicion de memoria para retornarla
								ultimaLectura=pag->ultimaLectura;								// guardo su ultima lectura para seguir comparando
								log_info(logger, "[CICLO PAG J=%i]Ultima lectura %i",j,ultimaLectura);
								log_info(logger, "[CICLO PAG J=%i]Posicion memo %i",j,posicionMemo);
								segment=i;														// determino el segmento y
								pagg=j;
								}// la pagina para despues poder eliminarla de mis tablas
							} else log_info(logger, "NO CUMPLE");
						}
					}
															// termino de recorrer las tablas
			if(ultimaLectura==-1) { return ultimaLectura; }	// si la posicion es -1 significa que no encontro ni una pagina que no estuviera modificada, devuevlo -1, hay que hacer journal
			//segmento* segm = list_get(tablas,segment);		// si sigue la ejecucion, es que encontro paginas, procedo a eliminar esa pagina que va a ser sustituda, traigo el segmento
			//list_remove(segm->paginas, pagg);				// elimino de la lista de paginas del segmento a la pagina indicada
			log_info(logger, "TERMINE CICLOS? %i",posicionMemo);
			return posicionMemo;							// retorno la direccion en memoria sobre el que voy a escribir mi nuevo registro dato.
		}





	/**
	* @NAME: traerRegistroDeMemoria
	* @DESC: Retorna una pagina desde memoria conociendo su posicion.
	*
	*/
	pagina_concreta* traerRegistroDeMemoria(int posicion){
				pagina_concreta* pagina = malloc(sizeof(uint16_t)+(sizeof(char)*max_value)+sizeof(long));
				unsigned int desplazamiento=0;
				memcpy(pagina->key,&memoria_principal[posicion*tamanio_pagina],sizeof(uint16_t));					// agrego primero KEY
				desplazamiento+=sizeof(uint16_t);
				memcpy(pagina->timestamp,&memoria_principal[posicion*tamanio_pagina+desplazamiento],sizeof(long));	// agrego luego TIMESTAMP
				desplazamiento+=sizeof(long);
				strcpy(pagina->value,&memoria_principal[posicion*tamanio_pagina+desplazamiento]);					// agrego por ultimo VALUE(tamaño variable)
				return pagina;
	}

/**
	* @NAME: encontrarSegmento
	* @DESC: Retorna el segmento buscado en la lista si este tiene el mismo nombre que el que buscamos.
	*
	*/
	segmento* encontrarSegmento(char* nombredTabla, t_list* tablas) {
            		int _es_el_Segmento(segmento* segmento) {
            			return string_equals_ignore_case(segmento->nombreTabla, nombredTabla);
            		}

            		return list_find(tablas, (void*) _es_el_Segmento);
    }
