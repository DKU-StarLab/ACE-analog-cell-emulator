#!/bin/bash

#install FEMU by using github
#git clone https://github.com/vtess/femu.git
mkdir logging
cd femu
# Switch to the FEMU building directory
mkdir build-femu
cd build-femu
# Copy femu script
 cp ../femu-scripts/femu-copy-scripts.sh .
./femu-copy-scripts.sh .
# only Debian/Ubuntu based distributions supported
sudo ./pkgdep.sh

#install ACE
rm -rf ../hw/femu
cp -rf ../../ace ../hw/
mv ../hw/ace ../hw/femu
./femu-compile.sh
cd ../../

