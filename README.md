# COS

Tiny x86_64 Operating System written in C. The OS can:
1. Handle Interrupts.
2. Allocate Physical Memory
3. Load Executables (ELF).
4. Premptively Schedule tasks
5. Do syscalls

Logging is forwarded to stdout of the host system.
The goal writing this OS was to bootstrap syscalls and preemptive scheduling in the shortest amount 
of time and with least lines of code possible. Currently it has 3K lines of code.

The OS does not currently have (but will at some point in the future):
1. Virtual Memory Manager (It is very simple atm).
2. Graphics Stack
3. Networking Stack


## Building

Make sure you have [nix](https://nixos.org/) installed. Make sure that you have pulled this repo recursively to pull limine. The current limine version supported is:
```
HEAD detached at origin/v7.x-binary
```

1. Pop into nix-shell
   ```
   nix-shell
   ```
2. Build limine
    ```
    cd limine
    make
    ```
3. Build the OS and Userland
   ```
   ./build.sh
   ```

## Running

Run:
```
./run.sh
```

## Debugging

Run:
```
./debug.sh
```
