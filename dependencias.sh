echo "clonando so-commons"
cd ~
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd ~/so-commons-library
make
sudo make install

echo "clonando a-mongos-pruebas"
cd ~
git clone https://github.com/sisoputnfrba/a-mongos-pruebas.git
