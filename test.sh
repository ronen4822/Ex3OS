#!/bin/sh

ipcrm -a

{ sleep 3; echo "3 4" ; sleep 2; } | ./server.out > server_output &


{ sleep 4; echo "0 0" ; sleep 4; } | ./client.out > client_output1 &

{ sleep 10; echo "2 3" ; sleep 10; } | ./client.out > client_output2 &

sleep 15 
grep "You won" client_output1
if [ "$?" = "0" ] ; then
    echo "Test 3x4 OK"
else
    echo "Test 3x4 NOT OK!"
fi 
