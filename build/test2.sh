#!/bin/bash

echo "TEST 2"; 

./server config.txt &
pid=$!
sleep 2s

./client -f ../thesocket  -t 200 -p -w ../data_test/filetest1 -D ../espulsi &

#sleep 1s

./client -f ../thesocket  -t 200 -p -w ../data_test/filetest2 -D ../espulsi &

#sleep 1s

./client -f ../thesocket  -t 200 -p -w ../data_test/filetest3 -D ../espulsi &


sleep 1s

kill -s SIGHUP $pid
wait $pid
