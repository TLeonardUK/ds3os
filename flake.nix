{
  description = "ds3os flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { nixpkgs, flake-utils, ... }:
    (flake-utils.lib.eachSystem [ "x86_64-linux" ] (system: let
      pkgs = nixpkgs.outputs.legacyPackages.${system};
      ds3os = pkgs.callPackage ./default.nix {};
    in {
      # nix build .
      packages.default = pkgs.callPackage ds3os {};
      # nix build .#clang
      packages.clang = pkgs.callPackage ds3os { stdenv = pkgs.clangStdenv; };
    }));
}
