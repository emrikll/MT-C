# STM32f051 Template
This is an example eclipse based blinky project for STM32F051 (ARM Cortex M0 controller). Below are the steps to get it working in Linux (Ubuntu 14.04 and above)

### Installing Eclipse and GNU ARM Toolchain
* Download Eclipse for C/C++ from here: https://eclipse.org/downloads/
* Install the GNU ARM plugins as per the steps mentioned here [Note: Although the link does not mention, you need to install the Packs and Qemu plugins as well]: http://gnuarmeclipse.livius.net/blog/plugins-install/
* Also install and configure the packs plugin as mentioned here: http://gnuarmeclipse.livius.net/blog/packs-manager/#The_Packs_perspective
* Download QEMU binaries from here: http://sourceforge.net/projects/gnuarmeclipse/files/QEMU/GNU%20Linux/

### ARM GCC Toolchain install
* Download the tar file form here: https://launchpad.net/gcc-arm-embedded/+download
* Open the terminal and enter the following command:
``` sh1
$ sudo apt-get install lib32z1 lib32ncurses5 lib32bz2-1.0
$ mkdir ~/ARMTools
$ cd ~/ARMTools
```
* Extract the download file inside this directory:
``` sh2
$ tar -xvf gcc-arm-none-eabi-j_n-yyyyqe-yyyymmdd-linux.tar.bz2
```
* Create symlinks to the binaries. This is required so that we don't have to add the folder path in $PATH variable. This also prevents modifying the eclipse PATH variable.
``` sh3
$ cd /usr/local/bin
$ sudo ln -s /home/${USER}/ARMTools/gcc-arm-none-eabi-j_n-yyyyge-yyyymmdd-linux.tar.bz2/bin/* .
```
### Openocd Install
* Download the openocd src code from here: http://sourceforge.net/projects/openocd/
* Extract the folder into /home/${USER}/ARMTools/
* Build and install from src
``` sh4
$ sudo apt-get install libusb-1.0-0-dev
$ cd openocd-x.y.z
$ ./configure --enable-maintainer-mode --enable-stlink --verbose
$ make
$ sudo make install
```
* Execute the following commands:
``` sh5
$ cd /home/${USER}/ARMTools/
$ tar -xvf openocd-x.y.z.tar.bz2
$ mkdir -p openocd-bin/scripts openocd-bin/bin
$ cp -R openocd-x.y.z/tcl/* openocd-bin/scripts/
$ cd openocd-bin/bin
$ ln -s /usr/local/bin/openocd openocd
```
* Add openocd_path variable to env.
``` sh6
$ echo "export OPENOCD_PATH=/home/bhavin/ARMUtils/openocd-bin/bin" >> ~/.bashrc
```
(For windows, download and install from here: http://sourceforge.net/projects/gnuarmeclipse/files/OpenOCD/Windows/)

### Using QEMU to emulate code
* The QEMU installed above is specially built for certain ARM boards. However, if you directly try to debug a normal app built for stm on qemu, you will get an error as below:
``` sh7
qemu: fatal: Trying to execute code outside RAM or ROM at 0x08000390
```
This is because for STM32F051, the qemu somehow thinks that the FLASH starts from 0x0 and RAM starts from 0x20000000. The RAM address is correct but the flash actually starts from 0x08000000. To solve this, you need to change the Flash address to 0x0 temporarily in the linker script and re-compile the code.
* Note: Remember to change the address back to original before loading the code into actual chip.
