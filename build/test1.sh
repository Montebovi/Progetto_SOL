#!/bin/bash

echo "TEST 1"; 

valgrind --leak-check=full --show-leak-kinds=all ./server config.txt &
pid=$!
sleep 2s

./client -f ../thesocket  -t 200 -p -b ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -l ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -W ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -u ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -e ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -D ../espulsi -d ../reader1


./client -f ../thesocket  -t 200 -p -o ../data_test/filetest1/testfile1.pdf -r ../data_test/filetest1/testfile1.pdf -e ../data_test/filetest1/testfile1.pdf -d ../reader2 -R 1 -D ../espulsi


./client -f ../thesocket  -t 200 -p -w ../data_test/filetest2 -d ../reader2 -R 1 -D ../espulsi


./client -f ../thesocket  -t 200 -p -o ../data_test/filetest1/testfile1.pdf -l ../data_test/filetest1/testfile1.pdf -c ../data_test/filetest1/testfile1.pdf -D ../espulsi 


sleep 1s

kill -s SIGHUP $pid
wait $pid
