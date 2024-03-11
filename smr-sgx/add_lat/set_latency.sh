#!/usr/bin/env zsh

ssh root@sigilium01 '/home/bdinisa/dev/sigilum/add_lat/rem_lat.sh bond0'
ssh root@sigilium02 '/home/bdinisa/dev/sigilum/add_lat/rem_lat.sh bond0'

for i in {1..7};
do
    ssh root@sgx0$i '/home/bdinisa/dev/sigilum/add_lat/rem_lat.sh bond0'
done

for i in {1..7};
do
    ssh root@sgx0$i '/home/bdinisa/dev/sigilum/add_lat/add_lat.sh bond0 1ms 500us'
done
ssh root@sigilium02 '/home/bdinisa/dev/sigilum/add_lat/add_lat.sh bond0 1ms 500us'

~/dev/sigilum/add_lat/all_lats_show.sh
