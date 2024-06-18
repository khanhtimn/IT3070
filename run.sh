#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Hãy cung cấp đường dẫn đến chương trình: $0 <program>"
  exit 1
fi

PROGRAM=$1

export LD_LIBRARY_PATH=$(pwd)

./"$PROGRAM"
