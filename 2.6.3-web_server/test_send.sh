#!/bin/bash
set -xe

IP_ADDRESS="192.168.1.113"

curl -X POST -H "Content-Type: application/json" -d "{\"hoge\" : \"fuga\"}" ${IP_ADDRESS}/ir/send