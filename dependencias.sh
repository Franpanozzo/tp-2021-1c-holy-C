echo "clonando so-commons"
cd ~
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd ~/so-commons-library
make
sudo make install

echo "clonando a-mongos-pruebas"
cd ~
git clone https://github.com/sisoputnfrba/a-mongos-pruebas.git

echo "instalando ncurses"
cd ~
sudo apt-get install libncurses5-dev

echo "clonando nivel-gui"
cd ~
git clone https://github.com/sisoputnfrba/so-nivel-gui-library.git
cd ~/so-nivel-gui-library
sudo make install


