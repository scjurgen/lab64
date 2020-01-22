#!/bin/bash

../build/LabNew -j ../configs/prismatic.json -w 1920 -h 1200 -o prismatic --oversample 2 -r 2
../build/LabNew -j ../configs/blue-velvet.json -w 800 -h 600 -o blue-velvet --oversample 1 -r 2

