#!/bin/sh

set -e

PKGS_DIR=$1

if [ ! "$PKGS_DIR" ]; then
    echo "ERROR: Could not install Arena SDK. Did not specify installation directory. Must provide installation directory as an argument"
    exit 1
fi

mkdir -p "$PKGS_DIR"

ARCH=`uname -m`
OS=`uname`

echo "Installing Arena SDK for $ARCH on $OS to $PKGS_DIR"
if [ $OS != "Linux" ]; then
    echo "ERROR: Arena SDK can only be installed on Linux"
    exit 1
else
    # pull Arena SDK from TUAS Google Drive 
    # https://drive.google.com/drive/folders/1Ek1luFtO-FpDUJHP9_NQ9RH5dYhyhWXi?usp=share_link
    if [ $ARCH = "aarch64" ]; then
        FILE_NAME="ArenaSDK_v0.1.49_Linux_ARM64.tar.gz"
        FILE_ID="1VtBji-cWfetM5nXZwt55JuHPWPGahQOH"
        ARENA_SDK_DIR="ArenaSDK_Linux_ARM64"
        ARENA_CONF="Arena_SDK_ARM64.conf"
    elif [ $ARCH = "x86_64" ]; then
        FILE_NAME="ArenaSDK_v0.1.68_Linux_x64.tar.gz"
        FILE_ID="1pQtheOK-f2N4C2CDi43HTqttGuxHKptg"
        ARENA_SDK_DIR="ArenaSDK_Linux_x64"
        ARENA_CONF="Arena_SDK_Linux_x64.conf"
    else
        echo "ERROR: Unable to install Arena-SDK. Unkown architecture $ARCH. Architecture must be aarch64 or x86_64."
        exit 1
    fi;

    if [ ! -d "$PKGS_DIR/$ARENA_SDK_DIR" ]; then \
        # Check that wget is installed
        if ! command -v wget > /dev/null; then \
            echo "ERROR: Unable to install Arena-SDK. wget is not installed. Install wget for your system and retry."
            exit 1
        fi;

        # Check that tar is installed
        if ! command -v tar > /dev/null; then \
            echo "ERROR: Unable to install Arena-SDK. tar is not installed. Install tar for your system and retry."
            exit 1
        fi;

        # /home/tuas/.local/bin/gdown "${FILE_ID}" -O "${PKGS_DIR}/${FILE_NAME}"
        # tar xf "$PKGS_DIR/$FILE_NAME" --directory="$PKGS_DIR"
    else \
        echo "WARNING: Arena SDK is already installed at $PKGS_DIR/$ARENA_SDK_DIR. Will not download again."; \
    fi;

    # Symlink arch specific arena install to architecture agnostic directory.
    # Build system can point to this folder without knowing which architecture to use.
    SYMLINKED_DIR="$PKGS_DIR/extracted"
    echo "Symlinking $ARENA_SDK_DIR to $SYMLINKED_DIR"
    ln -sf "$PKGS_DIR/$ARENA_SDK_DIR" "$SYMLINKED_DIR"
fi;
