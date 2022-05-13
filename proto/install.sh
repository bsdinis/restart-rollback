#!/usr/bin/env sh

# installs the protocol in the relevant places

for f in $(ls *.h);
do
    cp $f ../clnt/lib;
    cp $f ../server/enclave;
done
