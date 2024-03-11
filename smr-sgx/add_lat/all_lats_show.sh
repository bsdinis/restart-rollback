#!/usr/bin/env zsh

function show_lat() {
    lat=$(ssh root@$1 '/home/bdinisa/dev/sigilum/add_lat/show_lat.sh bond0')
    wait
    echo "[$1]: ${lat}"
}

show_lat sigilium01
show_lat sigilium02

for i in {1..10};
do
    show_lat sgx$(printf "%02d" i)
done
