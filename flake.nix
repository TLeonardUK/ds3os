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
      # nix run .#master-server
      apps.master-server = with pkgs; {
        type = "app";
        program = writeShellApplication {
            name = "app";
            runtimeInputs = [ nodejs ];
            text = ''
              tmp="$(mktemp -d)"
              trap 'rm -rf "$tmp"' EXIT
              cd "$tmp"
              ln -sf ${./Source/MasterServer/package.json} package.json
              ln -sf ${./Source/MasterServer/package-lock.json} package-lock.json
              ln -sf ${./Source/MasterServer/src} src
              npm ci --loglevel verbose --nodedir=${nodejs}/include/node
              NODE_PATH="$tmp/node_modules" npm run start
              '';
        } + "/bin/app";
      };
    }));
}
