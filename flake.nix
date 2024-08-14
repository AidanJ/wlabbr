{
  description = "wlabbr";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {nixpkgs, ...}: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.mkShell.override { stdenv = pkgs.gcc13Stdenv; } {
      packages = with pkgs; [ 
        clang-tools_17
        valgrind
        gdb
      ];
      buildInputs = with pkgs; [
        wayland-protocols
        wayland
        cjson
      ];
      nativeBuildInputs = with pkgs; [
        pkg-config
        ninja
        meson
        wayland-scanner
      ];

      shellHook = ''
      '';
    };
  };
}
