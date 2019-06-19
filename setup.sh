#!/bin/bash

#echo "Se instala Commons"
#cd ..
#git clone https://github.com/sisoputnfrba/so-commons-library
#cd so-commons-library
#sudo make install
#cd ..
echo "Compilacion de proyecto"
#cd tp-2019-1c-Los-Dinosaurios-Del-Libro
cd global/Debug/
make clean
make all
cd ../../Kernel/Debug/
make clean
make all
cd ../../Memoria/Debug/
make clean
make all
cd ../../LFS/Debug/
make clean
make all
cd ../../creacion_LFS/Debug/
make clean
make all
export LD_LIBRARY_PATH=/home/utnso/tp-2019-1c-Los-Dinosaurios-Del-Libro/global/Debug