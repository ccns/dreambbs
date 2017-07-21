#!/bin/sh
if [ $OSTYPE = "FreeBSD" ]; then
ostype="freebsd"
else if [ $OSTYPE = "linux-gnu" -o $OSTYPE = "Linux" ]; then
ostype="linux"
  fi
fi
echo $ostype

make $ostype install
