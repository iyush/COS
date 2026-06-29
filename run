#!/usr/bin/env bash

set -e
./build.sh

qemu-system-x86_64 -d int -D ./logs.txt -cdrom template.iso -m 512M -serial stdio 
# qemu-system-x86_64 -cdrom template.iso -m 256M -monitor stdio -no-reboot -no-shutdown
