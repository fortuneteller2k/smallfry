{
  description = "Pico SDK development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/master";
    parts.url = "github:hercules-ci/flake-parts";
  };

  outputs = inputs: inputs.parts.lib.mkFlake { inherit inputs; } {
    systems = [ "aarch64-darwin" "x86_64-darwin" ];

    perSystem = { self', lib, pkgs, ... }: {
      devShells.default = pkgs.mkShellNoCC {
        packages = __attrValues {
          inherit (pkgs)
            picotool
            cmake
            python3
            gnumake
            gcc-arm-embedded
            clang-tools_16;

          inherit (self'.packages) pico-sdk-full;
        };

        shellHook = ''
          export PICO_SDK_PATH=${self'.packages.pico-sdk-full}/lib/pico-sdk
          export C_INCLUDE_PATH=${pkgs.gcc-arm-embedded}/arm-none-eabi/include:$C_INCLUDE_PATH
        '';
      };

      packages = {
        pico-sdk-full = pkgs.pico-sdk.overrideAttrs (final: prev: {
          version = "1.5.1";

          src = pkgs.fetchFromGitHub {
            owner = "raspberrypi";
            repo = prev.pname;
            rev = final.version;
            hash = "sha256-GY5jjJzaENL3ftuU5KpEZAmEZgyFRtLwGVg3W1e/4Ho=";
            fetchSubmodules = true;
          };
        });
      };
    };
  };
}