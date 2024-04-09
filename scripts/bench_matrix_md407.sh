
#!/bin/bash

killall cat

FILENAME="matrix-md407.txt"

cmake --build /home/simon/MT-C/rp2040/build --config Debug --target all

cd ../MD407-matrix/

NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')

cmake -S . -B build
cmake --build build

stty -F /dev/ttyACM0 115200 cooked -echo -parenb cs8 -cstopb
stty -F /dev/ttyUSB0 115200 cooked -echo -parenb cs8 -cstopb

: "" > "../scripts/$FILENAME"  

for i in {1..5}
do
    echo "Loading..."
    echo 'load' > /dev/ttyUSB0 &&
    sleep 0.5 &&
    cat ./Debug/MD407-MATRIX.s19 > /dev/ttyUSB0 &&
    sleep 0.5 &&
    echo 'go' > /dev/ttyUSB0

    echo "Loaded!"
    cat /dev/ttyUSB0 >> "../scripts/$FILENAME" &
    read_pid=$!
    while [ $(du -sb "../scripts/$FILENAME"  | awk '{print $1}') == $NLINES ]; do sleep 0.1; done
    NLINES=$(du -sb "../scripts/$FILENAME"  | awk '{print $1}')
    kill $read_pid
    sleep 1
done

sed -i 's/[\n]//g' ../scripts/$FILENAME

