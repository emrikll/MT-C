
#!/bin/bash

killall cat

FILENAME="matrix-discovery.txt"

cmake --build /home/simon/MT-C/rp2040/build --config Debug --target all

stty -F /dev/ttyACM0 115200 cooked -echo -parenb cs8 -cstopb
cat /dev/ttyACM0 > $FILENAME &
cd ../discovery-template/

NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')

for i in {1..5}
do
   make matrix &
   process_pid=$!
   while [ $(du -sb "../scripts/$FILENAME"  | awk '{print $1}') == $NLINES ]; do sleep 0.5; done
   NLINES=$(du -sb "../scripts/$FILENAME"  | awk '{print $1}')
   kill $process_pid
done

sed -i 's/[\n]//g' ../scripts/$FILENAME

