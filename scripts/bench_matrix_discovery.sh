
#!/bin/bash
killall cat
cmake --build /home/simon/MT-C/rp2040/build --config Debug --target all

stty -F /dev/ttyACM0 115200 cooked -echo -parenb cs8 -cstopb

FILENAME="results.txt"
cat /dev/ttyACM0 > $FILENAME &
cd ../discovery-template/

killall openocd

echo "$(pwd)"
echo "../scripts/$FILENAME"

: "" > "../scripts/$FILENAME"

NLINES=$(du -sb "../scripts/$FILENAME" | awk '{print $1}')

for i in {1..51}
do
    make matrix &
    process_pid=$!
    while [ $(du -sb "../scripts/$FILENAME"  | awk '{print $1}') == $NLINES ]; do sleep 0.5; done
    NLINES=$(du -sb "../scripts/$FILENAME"  | awk '{print $1}')
    kill $process_pid
done

killall cat
killall openocd

sed -i 's/[\n]//g' ../scripts/$FILENAME
