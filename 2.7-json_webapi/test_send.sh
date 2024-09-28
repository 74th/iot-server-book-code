#!/bin/bash
set -xe

IP_ADDRESS="192.168.1.113"
TYPE="SONY"
HEX="0xA90"

curl -X POST -H "Content-Type: application/json" -d "{\"type\" : \"${TYPE}\", \"hex\": \"${HEX}\"}" ${IP_ADDRESS}/ir/send