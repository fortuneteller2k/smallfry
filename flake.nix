{
  description = "Pico SDK development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/master";
    parts.url = "github:hercules-ci/flake-parts";

    # others
    freertos-smp-src = { url = "github:FreeRTOS/FreeRTOS-Kernel/smp"; flake = false; };
    pico-extras-src = { url = "github:raspberrypi/pico-extras"; flake = false; };
    pico-sdk-src = { type = "git"; url = "https://github.com/raspberrypi/pico-sdk"; submodules = true; flake = false; };
    openocd-src = { type = "git"; url = "https://github.com/raspberrypi/openocd"; submodules = true; flake = false; };
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

          inherit (self'.packages)
            pico-sdk-full
            openocd-rp2040;
        };

        env = {
          PICO_SDK_PATH = "${self'.packages.pico-sdk-full}/lib/pico-sdk";
          PICO_EXTRAS_PATH = "${inputs.pico-extras-src}";
          FREERTOS_KERNEL_PATH = "${inputs.freertos-smp-src}";
        };

        shellHook =
          let
            arm-none-eabi-toolchain = "${pkgs.gcc-arm-embedded}/arm-none-eabi";

            clangdConfig = pkgs.writeText ".clangd" ''
              CompileFlags:
                Add: [
                  -std=gnu++2b,
                  -xc++,
                  -I${arm-none-eabi-toolchain}/include,
                  -I${arm-none-eabi-toolchain}/include-fixed,
                  -I${arm-none-eabi-toolchain}/include/c++/12.2.1,
                  -I${arm-none-eabi-toolchain}/include/c++/12.2.1/arm-none-eabi,
                  -I${arm-none-eabi-toolchain}/include/c++/12.2.1/backward
                ]

                Remove: -W*
                Compiler: ${pkgs.gcc-arm-embedded}/bin/arm-none-eabi-g++"
            '';
          in
          ''
            ln -sf ${clangdConfig} .clangd
          '';
      };

      packages = {
        openocd-rp2040 = (pkgs.openocd.override {
          extraHardwareSupport = [ "cmsis-dap" ];
        }).overrideAttrs (old: {
          src = inputs.openocd-src;
          nativeBuildInputs = old.nativeBuildInputs ++ [ pkgs.autoreconfHook ];
        });

        pico-sdk-full = pkgs.pico-sdk.overrideAttrs {
          version = "1.5.1";
          src = inputs.pico-sdk-src;
        };
      };
    };
  };
}
