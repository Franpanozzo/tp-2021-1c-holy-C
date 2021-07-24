
cd ./bibliotecas/Debug
echo "Compilando Bibliotecas"
make
cd ../../Mi-RAM_HQ/Debug
echo "Compilando Mi-RAM"
make
cd ../../Discordiador/Debug
echo "Compilando Discordiador"
make
cd ../../i-Mongo-Store/Debug
echo "Compilando i-Mongo"
make

cd ~
echo "agregando LD_LIBRARY_PATH"
echo "export LD_LIBRARY_PATH=~/tp-2021-1c-holy-C/bibliotecas/Debug/" >> ~/.bashrc

