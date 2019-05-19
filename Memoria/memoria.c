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
	list_add(tablas, crearSegmento("Nombre"));
	list_add(tablas, crearSegmento("Apellido"));
	list_add(tablas, crearSegmento("DNI"));


	paginaNueva(123,"Gon",125478,"Nombre",memoria_principal); // si no le paso la memoria principal por parametro me hace un segmentation fault.
	//paginaNueva(223,"GUn",15478,"Nombre",memoria_principal);
	pagina_concreta* paginaM;

	traerPaginaDeMemoria(0,paginaM,memoria_principal);

	segmento* unS = encontrarSegmento("BOLSA");
	//segmento* unS2 = encontrarSegmento("Apellido");
	log_info(logger,"SEGMENTO ENCONTRADO: %s", unS->nombreTabla);
	//log_info(logger,"SEGMENTO ENCONTRADO: %s", unS2->nombreTabla);
	//log_info(logger,"SEGMENTO ENCONTRADO: %s", unS2->nombreTabla);
	//log_info(logger,"PAGINA ENCONTRADA: %i", unaP->key);
	log_info(logger,"La key es: %i", paginaM->key);
	log_info(logger,"El ts es: %i", paginaM->timestamp);
	log_info(logger,"El value es: %s", paginaM->value);









//	iniciar_servidor_memoria_y_esperar_conexiones_kernel();
	select_esperar_conexiones_o_peticiones();
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
 	* @NAME: traerPaginaDeMemoria
 	* @DESC: pasando una posicion y la memoria devuelve el dato contenido en el mismo
 	*
 	*/
void traerPaginaDeMemoria(unsigned int posicion,pagina_concreta* pagina,char* memoria_principal){
	memcpy(&(pagina->key), &memoria_principal[posicion], sizeof(uint16_t));
	memcpy(&(pagina->timestamp), &memoria_principal[posicion+sizeof(uint16_t)], sizeof(long));
	strcpy(pagina->value, &memoria_principal[posicion+sizeof(uint16_t)+sizeof(long)]);

	}
/**
 	* @NAME: crearPagina
 	* @DESC: crea una pagina con una key y le asigna una posicion de memoria SE USA ADENTRO DE paginaNueva
 	*
 	*/
pagina* crearPagina(uint16_t key){
	pagina* paginaa = malloc(sizeof(pagina));
	paginaa->key=key;
	paginaa->modificado=0;
	paginaa->posicionEnMemoria=posicionProximaLibre;
	posicionProximaLibre+=1;
	return paginaa;
}

/**
 	* @NAME: paginaNueva
 	* @DESC: crea una pagina en la tabla de paginas(bit,key,posicion) y la vuelca en memoria(asumiendo que existe el segmento) PUES RECIBE LOS DATOS DEL LFS (select)
 	* NOTA: DATOS EN MEMORIA KEY-TS-VALUE
 	*/
void paginaNueva(uint16_t key, char* value, long ts, char* tabla, char* memoria){
	pagina* pagina = crearPagina(key);	// deberia ser con malloc?
	agregarPaginaASegmento(tabla,pagina);
	log_info(logger,"POSICION EN MMORIA: %i", pagina->posicionEnMemoria);
	memcpy(&memoria[pagina->posicionEnMemoria],&key,sizeof(uint16_t)); 					//deberia ser &key? POR ACA SEGMENTATION FAULT
	memcpy(&memoria[(pagina->posicionEnMemoria)+sizeof(uint16_t)],&ts,sizeof(long));		// mismo que arriba
	strcpy(&memoria[(pagina->posicionEnMemoria)+sizeof(uint16_t)+sizeof(long)], value);
	log_info(logger,"POSICION PROXIMA EN MMORIA DISPONIBLE: %i", posicionProximaLibre);
}
/**
 	* @NAME: agregarPaginaASegmento
 	* @DESC: agrega una pagina a un segmento
 	*
 	*/
void agregarPaginaASegmento(char* tabla, pagina* pagina){
	segmento* segmentoBuscado = encontrarSegmento("Nombre");
	list_add(segmentoBuscado->paginas, pagina);
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
	segmento1->nombreTabla=nombreTabla;
	log_info(logger, "Se creo el segmento %s", segmento1->nombreTabla);
	return segmento1;
	}
/**
 	* @NAME: resolver_select
 	* @DESC: resuelve el select
 	*
 	*/
	void resolver_select (int socket_kernel_fd, int socket_conexion_lfs){
	t_paquete_select* consulta_select = deserializar_select(socket_kernel_fd);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);
	char* tabla = consulta_select->nombre_tabla->palabra;
	uint16_t key = consulta_select->key;
	if(encontrarSegmento(tabla)!=NULL) { // es diferente de null(existe en memoria el segmento)
		if(encontrarPagina(encontrarSegmento(tabla),key)!= NULL) { // es diferente de null (existe en memoria la pagina)
			pagina* pag = encontrarPagina(encontrarSegmento(tabla),key); // bajo la pagina desde memoria
			int posicionEnMemoria = pag->posicionEnMemoria; // paso la posicion a una variable
			pagina_concreta* datos = traerRegistroDeMemoria(posicionEnMemoria); // bajo la pagina a su estructura
			char* value = datos->value;
			long timestamp = datos->timestamp;
			log_info(logger, "El registro con key '%d' se encuentra en memoria y tiene el value '$s'", key, value, timestamp);
		} 	else
			log_info(logger, "El registro con key '%d' NO se encuentra en memoria, pero si su tabla y procede a realizar la peticion a LFS", key);
			enviar_paquete_select(socket_conexion_lfs, consulta_select);
			eliminar_paquete_select(consulta_select);
			/**
				 *  aca hay que esperar a que el lfs nos envie los datos, hay que ver como resolver esto
				 *  esperarRegistroYPocesarlo(); deberia recibir los datos, crear la tabla de pagina en la tabla de paginas
				 *  y volcar el dato a memoria y despues retornar la pagina concreta para que se envien en otra funcion los datos
				 *  a quien os haya solicitado
				 */
			pagina_concreta* registroNuevo = esperarRegistroYPocesarlo();
	} 	else
		log_info(logger, "El registro con key '%d' NO se encuentra en memoria y procede a realizar la peticion a LFS", key);
		enviar_paquete_select(socket_conexion_lfs, consulta_select);
		eliminar_paquete_select(consulta_select);
		/**
		 *
		 *
		 *  aca es igual que arriba a diferencia de que hay que crear el segmento antes por que no existe.
		 *
		 */
		crearSegmento(tabla); // pues no existe.
		pagina_concreta* registroNuevo = esperarRegistroYPocesarlo();
		// 	loQueSeaParaDevolverRegistroAQuienLoPida();
	}
/**
	 	* @NAME: esperarRegistroYPocesarlo
	 	* @DESC: luego de una peticion al lfs espera los datos y los baja a una estrcutura de pagina
	 	*
	 	*/

	pagina_concreta* esperarRegistroYPocesarlo(){
		pagina_concreta* pagina;
		return pagina;
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
	void resolver_operacion(int socket_memoria, t_operacion cod_op){
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
					resolver_select(socket_memoria, socket_conexion_lfs);
					//aca debería enviarse el mensaje a LFS con SELECT
					break;
				case INSERT:
					log_info(logger, "%i solicitó INSERT", socket_memoria);
					//aca debería enviarse el mensaje a LFS con INSERT
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
	void select_esperar_conexiones_o_peticiones(){
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
	                    				resolver_operacion(i,cod_op);
	                    			    log_info(logger, "Se recibio una operacion de %i", i);

	                    			}}
	                	}
	            }
	}
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
	segmento*  *encontrarSegmento(char *nombredTabla) {
            		int _es_el_Segmento(segmento* segmento) {
            			return string_equals_ignore_case(segmento->nombreTabla, nombredTabla);
            		}

            		return list_find(tablas, (void*) _es_el_Segmento);
    }
/**
	* @NAME: encontrarPagina
	* @DESC: Retorna la pagina buscada en la lista si esta tiene el mismo key que el que buscamos.
	*
	*/
	pagina* *encontrarPagina(segmento* unSegmento, uint16_t key){
					int _es_la_Pagina(pagina *pagina) {
	            			return (pagina->key==key);
	            		}
					return list_find(unSegmento->paginas, (void*) _es_la_Pagina);
	}
