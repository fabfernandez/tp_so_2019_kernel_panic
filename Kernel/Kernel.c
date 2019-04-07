/*
 * Kernel.c
 *
 *  Created on: 7 abr. 2019
 *      Author: utnso
 */
	#include <Kernel.h>

int main(void) {
	// PARTE 2
	t_log* logger = iniciar_logger();

	t_config* config = leer_config();

	int conexion = crear_conexion(
		config_get_string_value(config, "IP_MEMORIA"),
		config_get_string_value(config, "PUERTO_MEMORIA")
	);

	enviar_mensaje("TESTINGGGGG", conexion);



	// Viejo terminar programa

	log_destroy(logger);
	config_destroy(config);
	close(conexion);
}

t_log* iniciar_logger() {
	return log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
}

t_config* leer_config() {
	return config_create("kernel.config");
}

/*
 * Hasta esta funcion, es parecida a la resolucion numero 2. Pero esta
 * ultima tenia un problema grave: la logica de leer strings de consola
 * se encontraba duplicada.
 *
 * Para solucionar eso vamos a introducir los dos conceptos nuevos, que
 * si bien no forman parte del standard de ANSI C, si forman parte de la
 * implementacion de GNU C, que es la que se encuentra en el compilador que
 * usamos por la catedra: GCC. Estos son:
 *
 * Funciones anidadas (o Nested Functions):
 * ---------------------------------------
 * El compilador nos permite crear funciones locales al contexto de otra funcion.
 * De esta manera, pueden evitar crear muchas funciones auxiliares que se van a usar
 * en un unico lugar, y tener que pensar miles de nombres diferentes para cada una.
 * Algo importante a tener en cuenta, es que las funciones anidadas, mantienen el
 * contexto de la funcion exterior, es decir, que las variables definidas en la funcion
 * exterior son accesibles desde le interior (masomenos, como funcionan las variables globales).
 *
 *
 * Punteros a Funcion:
 * ------------------
 * Las funciones no escapan  del concepto de puntero, por como funciona el compilador.
 * Esto implica que llamar a una funcion por su nombre, ejemplo:
 * ```
 * void _soy_funcion() { };
 *
 * variable = _soy_funcion;
 * ```
 * Es como tener un puntero a "alguna cosa", y podemos definir la variable como:
 * ```
 * void* variable = (void*) _soy_funcion;
 * ```
 * El casteo a "void*" es necesario para que el compilador entienda que es un puntero
 * a "alguna cosa".
 *
 * Ahora, si podemos asignar a una variable, porque no: pasar una funcion por parametro.
 * Esto nos habilita el orden superior en nuestras funciones.
 */


/*
 * Este comportamiento es el comun a ambas funciones,
 * cambiando solamente que por un lado vamos a loggear y
 * por el otro, queremos agregar las lineas a un paquete.
 *
 * Por lo tanto, necesita que se le pase una funcion que reciba
 * un string por parametro y no haga nada.
 *
 * Noten que la firma de la funcion pide void(*accion)(char*)
 * que implica:
 * - Un puntero a la funcion accion: (*accion)
 * - Devuelve un void
 * - Recibe un solo argumento (char*)
 */


int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}
