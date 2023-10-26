{pkgs ? import <nixpkgs> {}}:
(pkgs.buildFHSUserEnv {
  name = "OBCPP";
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
