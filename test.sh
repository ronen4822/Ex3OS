#!/bin/sh


#Test 3x4

running_pids=`ps -ef | egrep "server.out|client.out" | grep -v grep | awk '{print $2}'`
if [ "$running_pids" !=  "" ] ; then
  echo killing $running_pids
  kill $running_pids
fi

rm -f fifo_clientTOserver
ipcrm -a

{ sleep 12; echo "3 4" ; sleep 2; } | ./server.out > server_output &

sleep 2
echo starting first client

{ sleep 4; echo "0 0" ; sleep 4; } | ./client.out > client_output1 &

sleep 2 
echo starting second client

{ sleep 7; echo "2 3" ; sleep 10; } | ./client.out > client_output2 &

sleep  30
grep "You won" client_output1
if [ "$?" = "0" ] ; then
    echo "Test 3x4 OK"
else
    echo "Test 3x4 NOT OK!"
fi 



#Test 10x10
sleep 10
running_pids=`ps -ef | egrep "server.out|client.out" | grep -v grep | awk '{print $2}'`
if [ "$running_pids" !=  "" ] ; then
  echo killing $running_pids
  kill $running_pids
fi

rm -f fifo_clientTOserver
ipcrm -a

{ sleep 12; echo "10 10" ; sleep 2; } | ./server.out > server_output &
sleep 2
echo starting first client

{ sleep 4; echo "0 0" ; sleep 4; echo "1 1" ; sleep 35; echo "9 9"; }| ./client.out > client_output1 &
sleep 2
echo starting second client

{ sleep 7; echo "2 3" ; sleep 10; echo "2 3"; sleep 25; echo "4 4"; sleep 5; } | ./client.out > client_output2 &


sleep 60
grep "You won" client_output2
if [ "$?" = "0" ] ; then
    echo "Test 10x10 OK"
else
    echo "Test 10x10 NOT OK!"
fi

