#!/usr/bin/env bash
export PACKAGES_PATH=/home/chen/git/write_os/uefi/env/edk2:$(pwd)
# export PKG_OUTPUT_DIR=$(pwd)/Build

build -a X64 -p HelloWorld.dsc -m HelloWorld.inf -b RELEASE