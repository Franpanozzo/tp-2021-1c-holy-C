PUNTO_MONTAJE=~
TP=tp-2021-1c-holy-C
cd ${PUNTO_MONTAJE}/${TP}/bibliotecas/Debug
echo "Compilando Bibliotecas"
make
cd ${PUNTO_MONTAJE}/${TP}/bibliotecas/Debug/Mi-RAM_HQ/Debug
echo "Compilando Mi-RAM"
make
cd ${PUNTO_MONTAJE}/${TP}/Discordiador/Debug
echo "Compilando Discordiador"
make
cd ${PUNTO_MONTAJE}/${TP}/i-Mongo-Store/Debug
echo "Compilando i-Mongo"
make

cd ~
echo "agregando LD_LIBRARY_PATH"
echo "export LD_LIBRARY_PATH=${PUNTO_MONTAJE}/${TP}/bibliotecas/Debug/" >> ~/.bashrc

cd ~
source .bashrc
