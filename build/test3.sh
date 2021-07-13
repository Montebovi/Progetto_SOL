#!/bin/bash

echo "TEST 3"; 

./server config.txt &
pid=$!
sleep 2s

./client -f ../thesocket  -t 0 -p -b ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -l ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -W ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -u ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -e ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -D ../espulsi -d ../reader1 

./client -f ../thesocket  -t 0 -p -b ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -l ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -W ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -u ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -e ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -D ../espulsi -d ../reader2 &


./client -f ../thesocket  -t 0 -p -b ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -l ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -W ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -u ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -e ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -D ../espulsi -d ../reader2 

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -l ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -r ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -u ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -e ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -D ../espulsi -d ../reader2 &

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -l ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -r ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -u ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -e ../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -D ../espulsi -d ../reader1 &

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -l ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -r ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -u ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -e ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -D ../espulsi -d ../reader2 &

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -l ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -r ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -u ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -e ../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf -D ../espulsi -d ../reader1 &

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -l ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -r ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -u ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -e ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -D ../espulsi -d ../reader2 &

./client -f ../thesocket  -t 0 -p -o ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -l ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -r ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -u ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -e ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf -D ../espulsi -d ../reader1 

./client -f ../thesocket  -t 0 -p -d ../reader3 -R 0 -o ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf,../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf,../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -l ../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf,../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf,../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf -c../data_test/filetest1/testfile1.pdf,../data_test/filetest1/testfile1a.pdf,../data_test/filetest1/fat_file.docx,../data_test/filetest1/testfile1b.pdf,../data_test/filetest2/testfile1d.pdf,../data_test/filetest2/testfile1c.pdf &

sleep 2s 

#kill -s SIGHUP $pid
kill -s SIGINT $pid
wait $pid
