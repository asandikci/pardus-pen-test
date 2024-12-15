# Pardus Pen
Simple qt based pen application

### Nixos Build
see `pardus-pen-test.nix` file

build with this command to fetch latest (or specified) commit from GitHub:

`nix-build --verbose -E 'with import <nixpkgs> {}; callPackage ./pardus-pen-test.nix { }' --option sandbox true --no-substitute --option use-binary-caches false --option build-use-substitutes false --option build-use-substitutes false`

build with this command to build locally

`nix-build --verbose -E 'with import <nixpkgs> {}; callPackage ./pardus-pen-test-local.nix { }' --option sandbox true --no-substitute --option use-binary-caches false --option build-use-substitutes false --option build-use-substitutes false`

## Features
* Pen, marker, eraser tools
* Line spline, circle drawing
* Color selection
* Thickness selection
* Pen preview
* White, black, transparent backgrounds
* Square, line, none overlays
* Multiple pages
* Undo, redo buttons
* Screenshot
* Clear, exit confirm dialogs
* Minimize button
* Multi touch support
* Gfx tablet support
* Movable floating toolbox

## How to build
### Installing Dependencies
For debian:

`apt install meson ninja-build qtbase5-dev qtchooser libglib2.0-dev`

For archlinux:

`pacman -Syu meson ninja qt5-base qt5-tools glib2`

For alpine:

`apk add qt5-qtbase-dev qt5-qttools-dev glib-dev`

### Building
```
meson setup build
ninja -C build
```

### Installing
```
ninja -C build install
```

## How to create deb package
### Installing Dependencies
```
apt install -yq meson devscripts
```
### Create Deb Package
```
# run as root
mk-build-deps -ir
# run as user
dpkg-buildpackage -us -uc
```
