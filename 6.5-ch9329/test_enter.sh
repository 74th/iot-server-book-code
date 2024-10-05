#!/bin/bash
set -xe

IP_ADDRESS="192.168.1.113"
KEYCODE=40

curl -X POST -H "Content-Type: application/json" -d "{\"keycode\": [${KEYCODE}], \"shift\": false, \"ctrl\": false, \"alt\": false, \"win\": false}" ${IP_ADDRESS}/ch9329/keyboard
