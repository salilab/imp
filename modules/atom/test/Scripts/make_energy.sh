#!/usr/bin/bash
# $1 - log file
grep energy $1 | awk '{print $3,$6,$8}' | awk 'NF==3'
