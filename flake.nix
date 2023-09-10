{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs";
    };

    outputs = { self, nixpkgs, ... }:
        let
            system = "x86_64-linux";
            pkgs = nixpkgs.legacyPackages.${system};
        in
        {
            devShells.${system}.default = pkgs.mkShell.override { stdenv = pkgs.gcc13Stdenv; } {
                buildInputs = with pkgs; [
                    gnumake
                    gcc13
                    coreutils
                    grub2 # grub-mkrescue needed
                    xorriso
                    # clang-fmt
                    clang-tools_16

                    qemu
                    bochs
                ];
            };
        };
}
