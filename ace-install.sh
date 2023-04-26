#!/bin/bash

#install FEMU by using github
#git clone https://github.com/vtess/femu.git
mkdir logging
cd femu
# Switch to the FEMU building directory
cd build-femu
# only Debian/Ubuntu based distributions supported
sudo ./pkgdep.sh

#install ACE
rm -rf ../hw/femu
cp -rf ../../ace ../hw/
mv ../hw/ace ../hw/femu
./femu-compile.sh
cd ../../

