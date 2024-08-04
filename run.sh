#!/bin/bash

set -e
./build.sh

qemu-system-x86_64 -cdrom template.iso -m 256M -serial stdio 