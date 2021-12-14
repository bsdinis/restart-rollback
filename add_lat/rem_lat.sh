#!/usr/bin/env bash

# usual latency inside a cluster: 3ms

if [ $EUID -ne 0 ]
then
  echo $EUID
  echo "this command needs to be run as root"
  exit -1
fi

if [ $# -ne 1 ]
then
  echo $#
  echo "usage: ./rem_lat [interface]"
  exit
fi

tc qdisc del dev $1 root netem
tc -s qdisc | grep netem
