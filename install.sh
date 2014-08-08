#!/bin/bash

# Make sure only root can run our script
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

rm -rf libs
rm -rf templibs
mkdir templibs
mkdir libs
cd templibs
echo "Downloading RF24"
git clone https://github.com/TMRh20/RF24.git
echo "Downloading RF24Network"
git clone https://github.com/TMRh20/RF24Network.git
cp -R RF24/RPi/RF24 ../libs 
cp -R RF24Network/RPi/RF24Network ../libs 
cd ..
rm -rf templibs
cd libs/RF24
echo "Compiling RF24"
make 
make install
cd ../RF24Network
echo "Compiling RF24Network"
make
make install
echo "Now build NrfPiNode"
