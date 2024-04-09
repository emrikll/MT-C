#!/bin/bash

killall cat

FILENAME="matrix-pico.txt"

cmake --build /home/simon/MT-C/rp2040/build --config Debug --target all

stty -F /dev/ttyACM0 115200 cooked -echo -parenb cs8 -cstopb
cat /dev/ttyACM0 > $FILENAME &
cd ../rp2040

NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')

for i in {1..50}
do
   sudo sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ./build/App-Matrix/MATRIX.elf verify reset exit" &
   while [ $(du -sb "../scripts/$FILENAME"  | awk '{print $1}') == $NLINES ]; do sleep 0.5; done
   NLINES=$(du -sb "../scripts/$FILENAME"  | awk '{print $1}')
done

sed -i 's/[\n]//g' ../scripts/$FILENAME

