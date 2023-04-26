#!/bin/bash
#This script compiles ACE by using FEMU

rm -rf ./femu/hw/femu
cp -rf ./ace ./femu/hw/
mv ./femu/hw/ace ./femu/hw/femu
cd ./femu/build-femu/
./femu-compile.sh
cd ../../
