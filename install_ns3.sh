# Prerequisites
sudo apt-get -y update
sudo apt-get -y upgrade
sudo apt-get -y install gcc g++ python python-dev
sudo apt-get -y install qt4-dev-tools libqt4-dev
sudo apt-get -y install mercurial
sudo apt-get -y install bzr
sudo apt-get -y install cmake libc6-dev libc6-dev-i386 g++-multilib
sudo apt-get -y install gdb valgrind 
sudo apt-get -y install gsl-bin libgsl0-dev libgsl0ldbl
sudo apt-get -y install flex bison libfl-dev
sudo apt-get -y install tcpdump
sudo apt-get -y install sqlite sqlite3 libsqlite3-dev
sudo apt-get -y install libxml2 libxml2-dev
sudo apt-get -y install libgtk2.0-0 libgtk2.0-dev
sudo apt-get -y install uncrustify
sudo apt-get -y install doxygen graphviz imagemagick
sudo apt-get -y install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng
sudo apt-get -y install python-sphinx dia 
sudo apt-get -y install python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev
sudo apt-get -y install libboost-signals-dev libboost-filesystem-dev
sudo apt-get -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev

# Installation
cd
hg clone http://code.nsnam.org/bake
export BAKE_HOME=`pwd`/bake 
export PATH=$PATH:$BAKE_HOME
export PYTHONPATH=$PYTHONPATH:$BAKE_HOME
bake.py configure -e ns-3.17
bake.py deploy
