# Pardus Pen
Simple qt based pen application

### `dev-mine` branch related
* Added Features:
  * more history (500)
  * change eraser size with pressure (for better ux in gfx tablets)
  * added related files to build on NixOS
* How to build/test locally on **NixOS**
  * `nix-build --verbose -E 'with import <nixpkgs> {}; callPackage ./localtest.nix { }'`
  * pardus-pen-test will be installed in `./result/bin/pardus-pen`. You can call this executable with cli or any shortcut
  * Note: I'll add this package to NixOS packages directly in my free time

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
