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
	// ------------------------------------------ INICIO LOGGER, CONFIG, LEVANTO DATOS E INICIO SERVER ------------	//
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 //
	iniciar_logger();																								// */
	leer_config();																									//
	levantar_datos_memoria();																						// HAY QUE CAMBIAR RUTA A UNA VARIABLE PARA PODER LEVANTAR MEMORIAS CON DIFERENTES CONFIGS
	levantar_datos_lfs();																							//
	server_memoria = iniciar_servidor(ip_memoria, puerto_memoria); 													// obtener socket a la escucha
																													//*/
	// ------------------------------------------ INICIO LOGGER, CONFIG, LEVANTO DATOS E INICIO SERVER ------------	//


	// ------------------------------------------ INICIO LA TABLA DE GOSSIPING AGREGANDO LA MEMORIA ACTUAL			// GOSSIPING PARA ULTIMA ENTREGA
																													//
	seeds = levantarSeeds();
	puertosSeeds = levantarPuertosSeeds();																							//
	seedsCargadas();																								//
	//iniciarTablaDeGossiping();																					//
	//pthread_create(&thread_gossiping, NULL, &iniciarGossip, &tablaGossiping, &primeraVuelta);						// CREO THREAD DE GOSSIPING
																													//
	// ------------------------------------------ INICIO LA TABLA DE GOSSIPING AGREGANDO LA MEMORIA ACTUAL			//



	// ------------------------------------------ ME CONECTO CON LFS E INTENTO UN HANDSHAKE -----------------------	// INTENTA CONECTARSE, SI NO PUEDE CORTA LA EJECUCION
	socket_conexion_lfs = crear_conexion(ip__lfs,puerto__lfs); //
	if(socket_conexion_lfs != -1){														//
	log_info(logger,"Creada la conexion para LFS %i", socket_conexion_lfs);																	//
	intentar_handshake_a_lfs(socket_conexion_lfs); // no intento por que no anda
	} else {
		log_info(logger,"No se pudo realizar la conexion con LFS. Abortando.");
		return -1;
		//
	}
																													//
	// ------------------------------------------ ME CONECTO CON LFS E INTENTO UN HANDSHAKE -----------------------	//


	// ------------------------------------------ INICIO SERVIDOR Y ESPERO CONEXION	-------------------------------	//
//	iniciar_servidor_memoria_y_esperar_conexiones_kernel();															// puedo esperar de kernel o memoria(tanto conexiones como solicitudes)
	select_esperar_conexiones_o_peticiones();
	// ------------------------------------------ INICIO SERVIDOR Y ESPERO CONEXION	-------------------------------	//


	// ------------------------------------------ A ESPERA DE OPERACIONES -----------------------------------------	//

//	esperar_operaciones(socket_kernel_conexion_entrante);															// -> ESPERO OPERACIONES DE KERNEL(ACA DEBERIA ESPERAR TAMBIEN MEMORIAS Y LFS)

	// ------------------------------------------ A ESPERA DE OPERACIONES -----------------------------------------	//
//	terminar_programa(socket_kernel_fd, conexionALFS); // termina conexion, destroy log y destroy config.
	return EXIT_SUCCESS;
}

void resolver_select (int socket_kernel_fd, int socket_conexion_lfs){
	t_paquete_select* consulta_select = deserializar_select(socket_kernel_fd);
	log_info(logger, "Se realiza SELECT");
	log_info(logger, "Consulta en la tabla: %s", consulta_select->nombre_tabla->palabra);
	log_info(logger, "Consulta por key: %d", consulta_select->key);

	enviar_paquete_select(socket_conexion_lfs, consulta_select);
	eliminar_paquete_select(consulta_select);
}
 void iniciar_logger() {
	logger = log_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.log", "Memoria", 1, LOG_LEVEL_INFO);
}
void leer_config() {
	archivoconfig = config_create("/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config");
}
void terminar_programa(int conexionKernel, int conexionALFS)
{
	liberar_conexion(conexionKernel);
	liberar_conexion(conexionALFS);
	log_destroy(logger);
	config_destroy(archivoconfig);
}
int esperar_operaciones(int de_quien){ // MODIFICAR
	while(1){
			int cod_op = recibir_operacion(de_quien);
			switch(cod_op)
			{
			case HANDSHAKE:
				log_info(logger, "Inicia handshake con %i", de_quien);
				recibir_mensaje(logger, de_quien);
				enviar_handshake(de_quien, "OK");
				log_info(logger, "Conexion exitosa con con %i", de_quien);
				break;
			case SELECT:
				log_info(logger, "%i solicitó SELECT", de_quien);
				resolver_select(de_quien, socket_conexion_lfs);
				//aca debería enviarse el mensaje a LFS con SELECT
				break;
			case INSERT:
				log_info(logger, "%i solicitó INSERT", de_quien);
				//aca debería enviarse el mensaje a LFS con INSERT
				break;
			case CREATE:
				log_info(logger, "%i solicitó CREATE", de_quien);
				//aca debería enviarse el mensaje a LFS con CREATE
				break;
			case DESCRIBE:
				log_info(logger, "%i solicitó DESCRIBE", de_quien);
				//aca debería enviarse el mensaje a LFS con DESCRIBE
				break;
			case DROP:
				log_info(logger, "%i solicitó DROP", de_quien);
				//aca debería enviarse el mensaje a LFS con DROP
				break;
			case GOSSPING:
				log_info(logger, "La memoria %i solicitó GOSSIPING", de_quien);
					// chequearTablaYAgregarSiNoEsta(int de_quien);  // si me solicito gossiping ME ENVIO SU TABLA TB, asi que la chequeo toda y agrego las memorias que no tenga en mi tabla.
																	// le envio mi tabla de gossiping para que haga lo mismo.
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando conexion con %i", de_quien);
				return EXIT_FAILURE;
			default:
				log_warning(logger, "Operacion desconocida.");
				return EXIT_FAILURE;
				break;
			}
		}
}

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
void levantar_datos_memoria(){
	ip_memoria = config_get_string_value(archivoconfig, "IP_MEMORIA"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP de la memoria es %s",ip_memoria );
	puerto_memoria = config_get_string_value(archivoconfig, "PUERTO_MEMORIA"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto de la memoria es %s",puerto_memoria);
	nombre_memoria = config_get_string_value(archivoconfig, "NOMBRE_MEMORIA"); // asignamos NOMBRE desde CONFIG
	log_info(logger, "El nombre de la memoria es %s",puerto_memoria);
}
void levantar_datos_lfs(){
	ip__lfs = config_get_string_value(archivoconfig, "IP_LFS"); // asignamos IP de memoria a conectar desde CONFIG
	log_info(logger, "La IP del LFS es %s", ip__lfs);
	puerto__lfs = config_get_string_value(archivoconfig, "PUERTO_LFS"); // asignamos puerto desde CONFIG
	log_info(logger, "El puerto del LFS es %s", puerto__lfs);
}
void intentar_handshake_a_lfs(int alguien){
	char *mensaje = "Hola, me conecto, soy la memoria";
		log_info(logger, "Trato de realizar un hasdshake");
		if (enviar_handshake(alguien,mensaje)){
			log_info(logger, "Se envió el mensaje %s", mensaje);

			recibir_handshake(logger, alguien);
			log_info(logger,"Conexion exitosa con LFS");
		}
}
void iniciar_servidor_memoria_y_esperar_conexiones_kernel(){
	log_info(logger, "Memoria lista para recibir a peticiones de Kernel");
	log_info(logger, "Memoria espera peticiones");
	socket_kernel_conexion_entrante = esperar_cliente(server_memoria);
	log_info(logger, "Memoria se conectó con Kernel");
}
void iniciarTablaDeGossiping(){ /*
	tablaGossiping = malloc(sizeof(struct tablaMemoriaGossip));
	tablaGossiping->memoria.IP= ip_memoria;
	tablaGossiping->memoria.PUERTO= puerto_memoria;
	tablaGossiping->memoria.estado= CONECTADA;
	tablaGossiping->memoria.descriptorMemoria=server_memoria;
	tablaGossiping->siguiente=NULL;							//
	log_info(logger, "Se inicializo la tabla de Gossiping");
	log_info(logger, "Se agrego a la tabla de Gossiping la memoria: %i con estado %i de ip: %s y puerto: %s", tablaGossiping->memoria.descriptorMemoria,tablaGossiping->memoria.estado, tablaGossiping->memoria.IP,tablaGossiping->memoria.PUERTO );
 	 ------------> ver implementacion mas adelante <--------- ENTREGA FINAL */ }
char** levantarSeeds(){
	return config_get_array_value(archivoconfig, "IP_SEEDS");
}
char** levantarPuertosSeeds(){
	return config_get_array_value(archivoconfig, "PUERTO_SEEDS");
}
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

void seedsCargadas(){
	int i=0;
	while(seeds[i]!= NULL){
		log_info(logger, "Se obtuvo la seed a memoria con ip: %s , y puerto: %s",seeds[i],puertosSeeds[i]);
		i++;
	}
	log_info(logger, "Se obtuvieron %i seeds correctamente.",i);
}

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
	                if (FD_ISSET(i, &read_fds)) //  pregunta si "i" está en el conjunto ¡¡tenemos datos!!
	                	{
	                    	if (i == server_memoria)
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


	//log_info(logger, "Memoria lista para recibir peticiones o conexiones de otras memorias");
	//log_info(logger, "Memoria espera peticiones");
	//socket_kernel_conexion_entrante = esperar_cliente(server_memoria);
	//log_info(logger, "Memoria se conectó con Kernel");



// ANEXO //
/* CODIGO COMENTADO ENTRE iniciar_servidor_memoria_y_esperar_conexiones_kernel(); Y esperar_operaciones(socket_kernel_conexion_entrante);

	// conexion a lfs
		close(socket_conexion_lfs);
		terminar_programa(socket_conexion_lfs); // termina conexion, destroy log y destroy config.
	    conexionALFS = crear_conexion(ip_lfs, puerto_lfs); // --> VER <--

 FIN */
