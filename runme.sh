#!/bin/bash -e

echo "compilando..."
g++ main.cpp -o tp1
echo "corriendo tp1 usando config..."
./tp1 config


