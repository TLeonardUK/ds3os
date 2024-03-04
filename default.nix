# native deps
{
    lib
    , cmake
    , pkg-config
    , removeReferencesTo
    , writeShellApplication
    , jq
}:

# build deps
{
    stdenv
    , libuuid
}:

with builtins;
with lib;

let
    pkg = stdenv.mkDerivation rec {
        name = "ds3os";

        src = with fileset; toSource {
            root = ./.;
            fileset = unions [ ./CMakeLists.txt ./Source ./Tools/Build ];
        };

        nativeBuildInputs = [ cmake pkg-config removeReferencesTo ];
        buildInputs = [ libuuid ];

        enableParallelBuilding = true;

        installPhase = ''
            install -Dm755 ../bin/x64_release/libsteam_api.so $out/lib/libsteam_api.so
            install -Dm755 ../bin/x64_release/Server $out/bin/Server
            mkdir -p $out/share/ds3os
            cp -R ../bin/x64_release/WebUI $out/share/ds3os/WebUI
            '';

        # google's generated protobuf headers puts absolute file path garbage into the binaries
        # which will break reproducible builds
        fixupPhase = ''
            find "$out" -type f -exec remove-references-to -t "${src}" '{}' +
            '';

        cmakeFlags = [
            # Fix third party builds
            "-DCMAKE_C_STANDARD=99"
            "-DCMAKE_C_FLAGS=-Wno-implicit-function-declaration"
        ];

        # Can't pass multiple flags through cmakeFlags *sigh*
        # https://github.com/NixOS/nixpkgs/issues/114044
        # Though these really should be fixed in ds3os itself
        NIX_CFLAGS_COMPILE = [ "-Wno-format-security" "-Wno-non-pod-varargs" ];

        meta = {
            homepage = "https://github.com/TLeonardUK/ds3os";
            license = licenses.mit;
            platforms = [ "x86_64-linux" ];
        };
    };
in writeShellApplication {
    runtimeInputs = [ jq ];
    name = "ds3os";
    text = ''
        tmp="$(mktemp -d)"
        trap 'rm -rf "$tmp"' EXIT
        cd "$tmp"

        export LD_LIBRARY_PATH="${pkg}/lib:''${LD_LIBRARY_PATH:-}"
        config="''${XDG_CONFIG_HOME:-$HOME/.config/ds3os}"
        mkdir -p "$config"

        if [[ -f "$config/default/config.json" ]]; then
            game_type="$(jq -r '.GameType' "$config/default/config.json")"
        fi

        case "''${game_type:-DarkSouls3}" in
            DarkSouls2)
                echo "GameType: DarkSouls2"
                echo 335300 > steam_appid.txt
                ;;
            DarkSouls3)
                echo "GameType: DarkSouls3"
                echo 374320 > steam_appid.txt
                ;;
        esac

        ln -sf "${pkg}/share/ds3os/WebUI" .
        ln -sf "${pkg}/bin/Server" .
        ln -sf "$config" Saved
        exec ./Server "$@"
        '';
}
