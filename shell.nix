let
  pkgs = import <nixpkgs> {};
  crossPkgs = pkgs.pkgsCross.x86_64-embedded;
in
  pkgs.mkShell {
    nativeBuildInputs = [
      crossPkgs.buildPackages.binutils
      crossPkgs.buildPackages.gcc
      crossPkgs.buildPackages.gdb

      pkgs.xorriso
      pkgs.qemu
    ];
  }
