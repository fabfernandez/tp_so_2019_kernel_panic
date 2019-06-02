/*
 * parser.c
 *
 *  Created on: 11 abr. 2019
 *      Author: utnso
 */
#include "parser.h"

#define INSERT_KEY "INSERT"
#define SELECT_KEY "SELECT"
#define CREATE_KEY "CREATE"
#define DESCRIBE_KEY "DESCRIBE"
#define DROP_KEY "DROP"
#define ADD_KEY "ADD"
#define RUN_KEY "RUN"
#define JOURNAL_KEY "JOURNAL"

t_instruccion_lql parsear_linea(char* line){

	if(line == NULL || string_equals_ignore_case(line, "")){
		return lanzar_error("No se pudo interpretar una linea vacia\n");
	}

	t_instruccion_lql ret = {
		.valido = true
	};

	char* auxLine = string_duplicate(line);
	string_trim(&auxLine);
	char** split = string_split(auxLine," ");

	char* keyword = split[0];

	ret._raw = split;

	if(string_equals_ignore_case(keyword, INSERT_KEY)){

		ret.operacion = INSERT;
		if (split[1] == NULL || split[2]== NULL || split[3] == NULL ){
			return lanzar_error("Formato incorrecto. INSERT TABLA KEY VALUE TIMESTAMP - El ultimo valor es opcional\n");
		}
		string_to_upper(split[1]);
		ret.parametros.INSERT.tabla = split[1];
		ret.parametros.INSERT.key= (uint16_t)atoi(split[2]);
		ret.parametros.INSERT.value = (char*)split[3];
		if(split[4] == NULL){
			ret.parametros.INSERT.timestamp= (unsigned long)time(NULL);
		} else{
			ret.parametros.INSERT.timestamp=(long)atoi(split[4]);
		}

	} else if(string_equals_ignore_case(keyword, SELECT_KEY)){
		ret.operacion=SELECT;
		if (split[1] == NULL || split[2]== NULL){
			return lanzar_error("Formato incorrecto. SELECT TABLA KEY\n");
		}
		string_to_upper(split[1]);
		ret.parametros.SELECT.tabla = split[1];
		ret.parametros.SELECT.key= (uint16_t)atoi(split[2]);
	} else if(string_equals_ignore_case(keyword, CREATE_KEY)){
		ret.operacion=CREATE;
		if (split[1] == NULL || split[2]== NULL || split[3] == NULL || split[4] == NULL){
			return lanzar_error("Formato incorrecto. CREATE TABLE SC NumPart COMPACTACION\n");
		}
		string_to_upper(split[1]);
		ret.parametros.CREATE.tabla = split[1];
		string_to_upper(split[2]);
		if (!check_consistencia(split[2])){
			return lanzar_error("Valor consistencia inválido. Debe ser STRONG, STRONG_HASH, EVENTUAL\n");
		}
		ret.parametros.CREATE.consistencia = get_valor_consistencia(split[2]);
		ret.parametros.CREATE.num_particiones=atoi(split[3]);
		ret.parametros.CREATE.compactacion_time=(long)atoi(split[4]);
	} else if(string_equals_ignore_case(keyword, DESCRIBE_KEY)){
		ret.operacion=DESCRIBE;
		if  (split[2]!= NULL){
			return lanzar_error("Formato incorrecto. DESCRIBE TABLE o DESCRIBE\n");
		}
		if(split[1] !=NULL){
			string_to_upper(split[1]);
			ret.parametros.DESCRIBE.tabla = split[1];
		}else{
			ret.parametros.DESCRIBE.tabla=NULL;
		}
	} else if(string_equals_ignore_case(keyword, DROP_KEY)){
		ret.operacion=DROP;
		if (split[1] == NULL || split[2]!= NULL){
			return lanzar_error("Formato incorrecto. DROPE TABLE\n");
		}
		string_to_upper(split[1]);
		ret.parametros.DROP.tabla = split[1];
	} else if(string_equals_ignore_case(keyword, RUN_KEY)){
		ret.operacion=RUN;
		if (split[1] == NULL || split[2]!= NULL){
			return lanzar_error("Formato incorrecto. RUN path\n");
		}
		ret.parametros.RUN.path_script = split[1];
	} else if(string_equals_ignore_case(keyword, JOURNAL_KEY)){
		ret.operacion=JOURNAL;
		if (split[1] != NULL){
			return lanzar_error("Formato incorrecto. JOURNAL\n");
		}
	} else if(string_equals_ignore_case(keyword, ADD_KEY)){
		ret.operacion=ADD;
		if (!string_equals_ignore_case(split[1],"MEMORY") || split[2]== NULL || !string_equals_ignore_case(split[3],"TO") || split[4]==NULL){
			return lanzar_error("Formato incorrecto. ADD MEMORY TO CONSISTENCY\n");
		}
		ret.parametros.ADD.numero_memoria=atoi(split[2]);
		ret.parametros.ADD.consistencia= get_valor_consistencia(split[4]);

	} else {

		return lanzar_error("Operacion no contemplada.\n");
	}

	free(auxLine);
	return ret;
}

t_consistencia get_valor_consistencia(char* consistencia_ingresada){

	t_consistencia result;
	if(string_equals_ignore_case(consistencia_ingresada,STRONG_HASH_TEXT)){
		result= STRONG_HASH;
	}
	if (string_equals_ignore_case(consistencia_ingresada,STRONG_TEXT)){
		result = STRONG;
	}
	if (string_equals_ignore_case(consistencia_ingresada,EVENTUAL_TEXT)){
		result= EVENTUAL;
	}
	return result;
}

int check_consistencia(char* consistencia_ingresada){
	if ( !string_equals_ignore_case(STRONG_TEXT,consistencia_ingresada) && !string_equals_ignore_case(STRONG_HASH_TEXT,consistencia_ingresada) && !string_equals_ignore_case(EVENTUAL_TEXT,consistencia_ingresada)){
		return false;
	}
	return true;
}

t_instruccion_lql lanzar_error(char* mensaje){
	fprintf(stderr,mensaje );
	t_instruccion_lql error;
	error.valido=false;
	return error;
}
