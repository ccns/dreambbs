#!/bin/sh
if [ $OSTYPE = "FreeBSD" ]; then
ostype="freebsd"
else if [ $OSTYPE = "linux-gnu" -o $OSTYPE = "Linux" ]; then
ostype="linux"
  fi
fi
echo $ostype

cd /home/bbs/src/lib
make clean all
cd ../maple
make clean $ostype install
cd ../so
make clean $ostype install
cd ../bpop3d
make clean $ostype install
cd ../bmtad
make clean $ostype install
cd ../util
make clean $ostype install
cd ../bgopherd
make clean $ostype install
cd ../innbbsd
make clean link $ostype install
