#!/bin/bash
RANDOM_VALUE=$(($RANDOM % 500))
VALUE=${1:-$RANDOM_VALUE}

curl localhost:3000/position -X POST --data "{\"value\": $VALUE}" -H 'Content-Type: application/json'
echo
