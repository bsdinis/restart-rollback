#!/usr/bin/env bash

# root ca
openssl genrsa -out rootca.key 4096
openssl req -new -x509 -days 1826 -key rootca.key -out rootca.crt

# ica
openssl genrsa -out intermediateca.key 4096
openssl req -new -key intermediateca.key -out intermediateca.csr
openssl x509 -req -days 1000 -in intermediateca.csr -CA rootca.crt -CAkey rootca.key -CAcreateserial -out intermediateca.crt

# server
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr
openssl x509 -req -days 1000 -in server.csr -CA intermediateca.crt -CAkey intermediateca.key -set_serial 0101 -out server.crt -sha1

# client
openssl genrsa -out client.key 2048
openssl req -new -key client.key -out client.csr
openssl x509 -req -days 1000 -in client.csr -CA intermediateca.crt -CAkey intermediateca.key -set_serial 0101 -out client.crt -sha1

# mv
mv *.key *.csr *.crt *.srl ../certs
