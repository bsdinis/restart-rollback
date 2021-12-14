#!/usr/bin/env zsh

function rem_lat() {
    $(ssh root@$1 '/home/bdinisa/dev/sigilum/add_lat/rem_lat.sh bond0')
}

rem_lat sigilium01
rem_lat sigilium02

for i in {1..10};
do
    rem_lat sgx$(printf "%02d" i)
done
