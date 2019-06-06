#include "Kernel_Plani.h"



//Creación de Hilos

void iniciar_hilo_planificacion(){
	pthread_t hilo_planificacion;

	if(pthread_create(&hilo_planificacion, 0, planificador, NULL) !=0){
		log_error(logger, "Error al crear el hilo");
	}
	if(pthread_detach(hilo_planificacion) != 0){
		log_error(logger, "Error al crear el hilo");
	}
}

void* planificador(){
	while(1){
		revisa_new_queue();
		revisa_ready_queue();
		revisa_exec_queue();
	}
}

void revisa_new_queue(){
	while(queue_size(new_queue)>0){
		char* new_path=queue_pop(new_queue);

		t_script* nuevo_script = malloc(sizeof(t_script));
		nuevo_script->id=generarID();
		nuevo_script->path=new_path;
		nuevo_script->offset=0;

		queue_push(ready_queue, nuevo_script);
	}
}

void revisa_ready_queue(){
	while(puedo_ejecutar() && queue_size(ready_queue)>0) {
		t_script* script=queue_pop(ready_queue);
		queue_push(exec_queue, script);
	}
}


int puedo_ejecutar(){
	return queue_size(exec_queue)<1; //el 1 va a cambiar por la cantidad maxima de procesamiento por archivo de config
}

void revisa_exec_queue(){
	while(queue_size(exec_queue)>0){
		t_script* script_a_ejecutar = queue_pop(exec_queue);

		ejecutar_script(script_a_ejecutar);
		sleep(SLEEP_EJECUCION);

		if (script_a_ejecutar->offset==NULL) {
			queue_push(exit_queue, script_a_ejecutar);
		} else {
			queue_push(ready_queue, script_a_ejecutar);
		}
	}
}


