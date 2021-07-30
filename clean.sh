PUNTO_MONTAJE=~
TP=tp-2021-1c-holy-C
cd ${PUNTO_MONTAJE}/${TP}/bibliotecas/Debug
echo "Desinstalando Bibliotecas"
make clean
cd ${PUNTO_MONTAJE}/${TP}/bibliotecas/Debug/Mi-RAM_HQ/Debug
echo "Desinstalando Mi-RAM"
make clean
cd ${PUNTO_MONTAJE}/${TP}/Discordiador/Debug
echo "Desinstalando Discordiador"
make clean
cd ${PUNTO_MONTAJE}/${TP}/i-Mongo-Store/Debug
echo "Desinstalando i-Mongo"
make clean