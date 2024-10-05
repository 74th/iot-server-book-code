#!/bin/bash
set -xe

IP_ADDRESS="192.168.1.113"

curl -X GET -H "Content-Type: application/json" ${IP_ADDRESS}/sht30