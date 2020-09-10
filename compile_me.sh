#!/bin/bash
set -x
g++ -std=c++17 -Wall -Wextra -pedantic wwwbk.cpp -o wwwbk
sudo cp wwwbk /usr/local/bin
