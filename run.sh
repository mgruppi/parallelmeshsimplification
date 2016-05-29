#!/bin/bash
inputFile="pierrot.off"
inputDir="../models/"
input=$inputDir$inputFile
range=(10 20 30 40 50 100)



for i in "${range[@]}"
do
  echo "Running $input in $i"
  time ./Simplify $input 0.90 elen $i
done 
