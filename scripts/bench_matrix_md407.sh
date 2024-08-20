
#!/bin/bash

killall cat
killall openocd

FILENAME="results.txt"
TTY="ttyUSB1"

cd ../MD407-concurrency/

cmake -S . -B build
cmake --build build

stty -F /dev/ttyACM0 115200 cooked -echo -parenb cs8 -cstopb
stty -F "/dev/$TTY" 115200 cooked -echo -parenb cs8 -cstopb

: "" > "../scripts/$FILENAME"

NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')

for i in {0..50}
do
    echo "Loading..."
    echo 'load' > "/dev/$TTY" &&
    sleep 0.5 &&
    cat ./Debug/MD407-MATRIX.s19 > "/dev/$TTY" &&
    sleep 0.5
    cat "/dev/$TTY" >> "../scripts/$FILENAME" &
    read_pid=$!
    NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')
    echo 'go' > "/dev/$TTY"
    sleep 0.2
    
    echo "Loaded!"
    while [ $(du -sb "../scripts/$FILENAME"  | awk '{print $1}') == $NLINES ]; do sleep 0.1; done
    NLINES=$(du -sb "../scripts/$FILENAME"  | awk '{print $1}')
    kill $read_pid
    sleep 1
done

sed -i 's/[\n]//g' ../scripts/$FILENAME

