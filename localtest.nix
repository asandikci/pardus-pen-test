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
  ...
}:

stdenv.mkDerivation {
  pname = "pardus-pen-test";
  version = "4.0.1";

  
  src = ./.;

  
  nativeBuildInputs = [ qt5.wrapQtAppsHook meson ninja pkg-config cmake libarchive];
  #qtWrapperArgs = [ ''--prefix PATH : /usr/bin/pardus-pen'' ];
  dontWrapQtApps = true;

  installPhase = ''
    runHook preInstall
    
    mkdir -p "$out/bin"
    cp -r "pardus-pen" "$out/bin"      
    chmod +x "$out/bin/pardus-pen"
    echo "name is:"
    echo "$name"
    runHook postInstall
  '';

  postFixup = ''
      wrapProgram "$out/bin/pardus-pen"  ''${qtWrapperArgs[@]}
  '';

}
