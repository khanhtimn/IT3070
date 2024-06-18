#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Hãy cung cấp đường dẫn đến file ELF (.so): $0 <program>"
    exit 1
fi

ELF_FILE=$1

SOURCE_FILE="dynamic_loader.c"

gcc -o dynamic_loader $SOURCE_FILE

if [ $? -ne 0 ]; then
    echo "Biên dịch thất bại. Vui lòng kiểm tra lại file $SOURCE_FILE."
    exit 1
fi

export LD_LIBRARY_PATH=$(pwd)

./dynamic_loader "$ELF_FILE"

if [ $? -ne 0 ]; then
    echo "Thực thi thất bại."
    exit 1
fi

echo "Thực thi thành công."
