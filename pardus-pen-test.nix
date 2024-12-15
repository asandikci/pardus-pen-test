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
    rev = "ba9c63ceb86f49e2fde98749dbf7332b4e3e2f36";
    hash = "sha256-6ivoygu5EM7JcqkTQcZoKXzfnofszilEXfbMvvdufB0=";
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
