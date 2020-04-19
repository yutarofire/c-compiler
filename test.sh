#!/bin/bash
set -e

cc -o 9cc 9cc.c
./9cc 123 > 9cc.s
cat 9cc.s

rm 9cc 9cc.s
