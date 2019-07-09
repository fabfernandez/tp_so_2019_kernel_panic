# Lissandra - Los-Dinosaurios-Del-Libro - 2019 1C

A continuación todo lo necesario para la instalación y ejecución del trabajo práctico.
## Instalación
**Ubicacion:** `cd /home/utnso`

**Clonar repositorio:** 

`git clone https://github.com/sisoputnfrba/tp-2019-1c-Los-Dinosaurios-Del-Libro.git`

**Compilar proyecto:** 

    cd tp-2019-1c-Los-Dinosarios-Del-Libro
    ./setup.sh

**Creación LFS** 

    cd tp-2019-1c-Los-Dinosarios-Del-Libro/creacion_LFS/Debug
    ./creacion_LFS [cant_bloques] [size_bloques] [magic_number]   

## Ejecución de los  módulos
 LFS:

    cd LFS/Debug
    ./LFS

KERNEL:

	cd Kernel/Debug
	./Kernel

Memorias:

	cd Memoria/Debug
	./Memoria /home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoria.config
	Para la memoria principal

	./Memoria /home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/Memoria/memoriaN.config
	Siendo N el numero de memoria que queramos levantar.
