[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

# ACE: Analog Cell Emulator 
# ACE: Analog Cell Emulator 

ACE is a FEMU-based reliability measurement tool. Analog signals can be output using ACE. ACE is an emulator designed to generate errors similar to real SSDs. Gaussian distribution graph values ​​are assigned to each cell to simulate analog signal distribution in actual cells. It is necessary to model changes in the distribution graph to trigger errors. We modeled the parameters using polynomial regression, one of the machine learning techniques. The modeled parameters change the distribution graph according to the P/E cycle, write time, and read count, resulting in durability, retention, and disturbance error. These errors are similar to those that occur on real hardware. ACE functions are implemented in ./ace/error.

## How to install ACE?

1. Install VM image
2. Run install-script
3. Run black-box script

### 1. Install VM image
You must use <span style="color:red"> GUI </span> environment. This method was referred to FEMU github. <br>

```
git clone https://github.com/DKU-StarLab/ACE-analog-cell-emulator

cd ACE-Analog-Cell-Emulator

mkdir ./images

cd ./ACE-Analog-Cell-Emulator/images

wget https://releases.ubuntu.com/focal/ubuntu-20.04.6-live-server-amd64.iso

sudo apt-get install qemu-system-x86

qemu-img create -f qcow2 femu.qcow2 80G

qemu-system-x86_64 -cdrom ubuntu-20.04.6-live-server-amd64.iso -hda femu.qcow2 -boot d -net nic -net user -m 8192 -rtc base=localtime -smp 8 -cpu host -enable-kvm

```

Run QEMU using VM image.
```
qemu-system-x86_64 -hda femu.qcow2 -net nic -net user -m 8192 -rtc base=localtime -smp 8 -cpu host -enable-kvm
```

Edit **/etc/default/grub**
```
GRUB_CMDLINE_LINUX="ip=dhcp console=ttyS0,115200 console=tty console=ttyS0"
GRUB_TERMINAL=serial
GRUB_SERIAL_COMMAND="serial --unit=0 --speed=115200 --word=8 --parity=no --stop=1"
```
Update grub information.
```
 sudo update-grub
 sudo shutdown -h now
 ```
If you have any problem, Please visit FEMU github. <br>
https://github.com/vtess/FEMU
 ### 2. Run install script
 ```
 ./ace-install.sh
 ```
 ### 3. Run black-box mode
 ```
 ./run-ace-blackbox-mode.sh
 ```

 ## You can compile ACE by using this script. 
 If you want to modify the analog signal and the inside of the SSD, you can modify the ACE folder.
 ```
 ./ace-compile.sh
 ```

 ## You can clean file using this script.
 ```
 ./clean.sh
 ```

 ## Either TLC mode or MLC mode can be used in ./ace/nvme.h.
```
in ./ACE/nvme.h 
line 48,49

#define MLC_ERROR 0 // 1: on 2: off
#define TLC_ERROR 1 // Make sure to use either MLC or TLC
```

 ## You can check log to ./logging directory
If you run ACE, you can see log data in the ./logging directory. Log data includes RBER, UBER, and voltage. You can output the desired log by adjusting the UBER_CHK and VOL_CHK options in ./ace/nvme.h.
```
in ./ace/nvme.h 
line 51,52

#define UBER_CHK 1 // 1: on 0: off
#define VOL_CHK 1
```
## How to evaluate ACE?
You can use whatever workload you want. We use fio here. After running the workload, check the error log in the ./logging directory.
```
# Just mount it to the file system you want.
sudo mkfs -t ext4 /dev/nvmeon1
# install fio
sudo apt-get install fio
```
This is simple script for fio(example script file name: fio.sh)
```
[global]
filename=/dev/nvme0n1
numjobs=1
ioengine=sync
size=8m

norandommap
group_reporting

[wrute]
stonewall
rw=randwrite

[read]
stonewall
rw=randread
```
Run fio script (example script file name: fio.sh)
```
sudo fio fio.sh
```

## Reference
FEMU source: https://github.com/vtess/FEMU <br>
ECC source: https://github.com/ak-hard/brcm-nand-bch

 
