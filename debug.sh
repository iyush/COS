#!/bin/bash
set -e

./build.sh
qemu-system-x86_64 -cdrom template.iso -s -S -m 256 -serial stdio &
gdb build/kernel -x kernel/init.gdb
wait