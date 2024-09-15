{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils, pkgs, ... }@inputs: flake-utils.lib.eachDefaultSystem (system: {
    devShells.default = pkgs.mkShell rec {
      nativeBuildInputs = with pkgs; [
        gdb
        llvmpackages.bintools
      ];

      buildInputs = with pkgs; [
        stdenv
        gnumake
        ccls
        pkg-config
      ];

      LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath nativeBuildInputs;
    };
  });
}
