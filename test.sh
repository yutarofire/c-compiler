#!/bin/bash
set -e

echo =====

cc -o 9cc 9cc.c
./9cc 42
rm -f 9cc

echo =====
echo " "
echo DONE
