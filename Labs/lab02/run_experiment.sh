#!/bin/bash

SPEED=1
DELAY=1
LOSS=0
CORRUPT=0

{
    killall link
    killall recv
    killall send
} &> /dev/null

./link_emulator/link speed=$SPEED delay=$DELAY loss=$LOSS corrupt=$CORRUPT &> /dev/null &
sleep 1
./recv fisier.bin fisier_2.bin test test.txt &
sleep 1

./send fisier.bin fisier_2.bin test test.txt

sleep 2
echo "Finished transfer, checking files"
diff fisier.bin fisier.bin-received
diff fisier_2.bin fisier_2.bin-received
diff test test-received
diff test.txt test.txt-received
