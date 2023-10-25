# { pkgs ? import <nixpkgs> {} }:

# pkgs.mkShell {
#   buildInputs = [
#     pkgs.gcc
#     pkgs.cmake
#     pkgs.libclang
#     pkgs.xz
#     pkgs.bzip2
#   ];
  # LD_LIBRARY_PATH = lib.makeLibPath [ pkgs.bzip2 pkgs.xz ];

  #  shellHook = ''
  #   CURRENTDIR=$PWD
  #   CONF_FILE=Arena_SDK.conf

  #   echo
  #   echo "Arena SDK configuration script"
  #   echo "Usage: Arena_SDK_Linux_x64.conf [-r]"
  #   echo "-r: Remove existing $CONF_FILE before add new paths"
  #   echo

  #   if [ "$1" = "-r" ]; then
  #       echo "Removing existing $CONF_FILE"
  #       sudo rm -f /etc/ld.so.conf.d/$CONF_FILE
  #       echo
  #   fi

  #   echo "Adding the following Arena SDK library paths to /etc/ld.so.conf.d/$CONF_FILE:"
  #   echo
  #   echo "$CURRENTDIR/lib64"
  #   echo "$CURRENTDIR/GenICam/library/lib/Linux64_x64"
  #   echo "$CURRENTDIR/ffmpeg"

  #   sh -c "echo $CURRENTDIR/deps/arena-sdk/ArenaSDK_Linux_x64/lib64/lib64 > /etc/ld.so.conf.d/$CONF_FILE"
  #   sh -c "echo $CURRENTDIR/deps/arena-sdk/ArenaSDK_Linux_x64/lib64/GenICam/library/lib/Linux64_x64 >> /etc/ld.so.conf.d/$CONF_FILE"
  #   sh -c "echo $CURRENTDIR/deps/arena-sdk/ArenaSDK_Linux_x64/lib64/ffmpeg >> /etc/ld.so.conf.d/$CONF_FILE"

  #   echo
  #   echo "Please remember to install these packages if not already installed before proceeding:"
  #   echo "- g++ 5 or higher"
  #   echo "- make"
  # '';
# }


{pkgs ? import <nixpkgs> {}}:
(pkgs.buildFHSUserEnv {
  name = "python 3.9";
  targetPkgs = pkgs: (with pkgs; [
	python3
  ccache
  # ninja
	# pipenv

	# LSP
	# python39Packages.python-lsp-server

	# C deps
	glib
	glibc
	stdenv
	zlib

  gcc
  cmake
  libclang
  xz
  bzip2

  ]);
runScript = "bash";
profile = ''
    LD_LIBRARY_PATH=${pkgs.zlib}/lib:${pkgs.bzip2}/lib:$LD_LIBRARY_PATH
    # set SOURCE_DATE_EPOCH so that we can use python wheels
    SOURCE_DATE_EPOCH=$(date +%s)
    PATH=$HOME/.local/bin:$PATH
    # PYTHONPATH=$HOME/.local/lib/python3.9/site-packages:$PYTHONPATH
    # export PIP_PREFIX=$(pwd)/_build/pip_packages
    # export PYTHONPATH="$PIP_PREFIX/${pkgs.python3.sitePackages}:$PYTHONPATH"
    # export PATH="$PIP_PREFIX/bin:$PATH"
    # unset SOURCE_DATE_EPOCH
  '';
}).env
