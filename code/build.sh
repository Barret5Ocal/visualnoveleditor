#!/bin/bash

CODE_HOME="$PWD"
OPTS=-g
cd ..\..\build > /dev/null
g++ $OPTS $CODE_HOME/win32_vnedit.cpp -o win32_vnedit.exe
cd $CODE_HOME > /dev/null
