let
  pkgs = import <nixpkgs> {};
  crossPkgs = pkgs.pkgsCross.x86_64-embedded;
in
  pkgs.mkShell {
    nativeBuildInputs = [
      crossPkgs.buildPackages.binutils
      crossPkgs.buildPackages.gcc
      crossPkgs.buildPackages.gdb
      # crossPkgs.buildPackages.bochs

      pkgs.xorriso
      pkgs.qemu
      pkgs.nasm
    ];

    shellHook = ''
    alias gs='git status'
    alias gap='git add -p'
    alias gd='git diff'
    alias gds='git diff --staged'
    '';
  }
