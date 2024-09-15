{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils, ... }@inputs: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
    in
    {
      devShells.default = pkgs.mkShell rec {
        nativeBuildInputs = with pkgs; [
          gdb
          bintools
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
