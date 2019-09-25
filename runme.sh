#!/bin/bash -e

echo "compilando..."
g++ main.cpp -o tp1
echo "storage"
tail -n+1 storage/*
echo "request"
tail -n+1 request/*
echo "corriendo tp1 usando config..."
./tp1 config


