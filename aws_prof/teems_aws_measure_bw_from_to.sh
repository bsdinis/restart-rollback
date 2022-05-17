#!/usr/bin/env bash

AWS=(dummy ec2-34-244-45-244.eu-west-1.compute.amazonaws.com ec2-34-205-76-144.compute-1.amazonaws.com ec2-13-56-248-36.us-west-1.compute.amazonaws.com ec2-13-244-254-121.af-south-1.compute.amazonaws.com ec2-35-77-101-140.ap-northeast-1.compute.amazonaws.com ec2-3-26-44-34.ap-southeast-2.compute.amazonaws.com ec2-157-175-237-92.me-south-1.compute.amazonaws.com ec2-15-228-60-29.sa-east-1.compute.amazonaws.com ec2-13-127-46-201.ap-south-1.compute.amazonaws.com ec2-13-51-150-250.eu-north-1.compute.amazonaws.com)

from=${AWS[$1]}
to=${AWS[$2]}

ssh-add -D
echo "$1 -> $2"
ssh -i ~/.ssh/aws$1.pem $(printf "aws%02d" $1) "iperf3 -4 -c ${to} -p 6666 | grep sender"
