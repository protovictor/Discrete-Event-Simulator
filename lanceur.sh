#!/bin/bash

make clean
make install
make examples
./examples/test_HTTP
