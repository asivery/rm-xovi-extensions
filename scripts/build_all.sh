#!/bin/bash
# This script is meant to be executed by the github action.

SCR_ROOT=$(pwd)
export TMP=$SCR_ROOT/tmp/xovi-extensions
export XOVI_REPO=/tmp/xovi
mkdir -p $TMP
git clone https://github.com/asivery/xovi /tmp/xovi

for dir in ../*; do
    if [ -d $dir ] && [ $dir != "../scripts" ]; then
        echo "Building from $dir..."
        cd "$dir"
        bash make-aarch64.sh
        cd "$SCR_ROOT"
    fi
done


mkdir output
find .. -iname "*.so" -not -path "**/release/deps/**" -exec cp -v {} output \;

