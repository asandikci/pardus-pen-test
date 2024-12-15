{ 
  stdenv,
  lib,
  qt5,
  fetchFromGitHub,
  meson,
  ninja,
  pkg-config,
  cmake,
  libarchive,
  glib,
  ...
}:

stdenv.mkDerivation {
  pname = "pardus-pen-test";
  version = "4.0.0";

  
  src = fetchFromGitHub {
    owner = "asandikci";
    repo = "pardus-pen-test";
    rev = "4f042e83122dfd8d4db42fda02c7b68bb95df9d3";
    hash = "sha256-t4gvUZ4Vq5/MYs5J3rZ+ltWjtwPA8JcoVeeMJrfEev0=";
  };

  
  nativeBuildInputs = [ qt5.wrapQtAppsHook meson ninja pkg-config cmake libarchive glib];
  #qtWrapperArgs = [ ''--prefix PATH : /usr/bin/pardus-pen'' ];
  dontWrapQtApps = true;

  installPhase = ''
    runHook preInstall
    
    mkdir -p "$out/bin"
    cp -r "pardus-pen" "$out/bin"      
    chmod +x "$out/bin/pardus-pen"
    echo "name is:"
    echo "$name"
    mkdir -p "$out/share/gsettings-schemas/$name/glib-2.0/schemas/"
    cp "../data/tr.org.pardus.pen.gschema.xml" "$out/share/gsettings-schemas/$name/glib-2.0/schemas/"
    glib-compile-schemas $out/share/gsettings-schemas/$name/glib-2.0/schemas/    
    
    runHook postInstall
  '';

  postFixup = ''
      wrapProgram "$out/bin/pardus-pen"  ''${qtWrapperArgs[@]}
  '';

}
