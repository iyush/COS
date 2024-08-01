#!/bin/bash

./build.sh
qemu-system-x86_64 -cdrom template.iso -s -S -m 256 -serial stdio 