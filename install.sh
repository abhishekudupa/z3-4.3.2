#!/bin/bash

rm -rf ./install
mkdir install
cp -a ./build/*.py ./install/
cp -a ./build/*.pyc ./install/
cp -a ./build/*.so ./install/
cp -a ./build/z3 ./install/
