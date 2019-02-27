#!/bin/bash

curl localhost:3000/position -X POST --data "{\"value\": $(($RANDOM % 500))}" -H 'Content-Type: application/json'
echo
