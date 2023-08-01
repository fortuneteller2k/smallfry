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

        shellHook =
          let
            FreeRTOS-Kernel-SMP-src = pkgs.fetchFromGitHub {
              owner = "FreeRTOS";
              repo = "FreeRTOS-Kernel";
              rev = "570ade4001e50adbf06a074582ea993af562e0e1";
              hash = "sha256-AxXsNpf6zzmkyY8AeCyN1HtHnSNz8JECljVURLEgUeY=";
            };

            pico-extras = pkgs.fetchFromGitHub {
              owner = "raspberrypi";
              repo = "pico-extras";
              rev = "ed98c7acb694757715ede81c044a7404e1762499";
              hash = "sha256-mnK8BhtqTOaFetk3H7HE7Z99wBrojulQd5m41UFJrLI=";
            };
          in ''
            export PICO_SDK_PATH=${self'.packages.pico-sdk-full}/lib/pico-sdk
            export PICO_EXTRAS_PATH=${pico-extras}
            export FREERTOS_KERNEL_PATH=${FreeRTOS-Kernel-SMP-src}
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