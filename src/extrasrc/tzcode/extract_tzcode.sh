#!/bin/sh
#
# A simple shell script which takes the tzcodeXXXXx.tar.gz archive as an argument
# and extracts all the necessary code files from it.

TZCODE="$1"

tar xzf ${TZCODE} asctime.c difftime.c localtime.c private.h strftime.c tzfile.h
