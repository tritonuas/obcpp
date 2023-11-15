#!/bin/sh

set -e
set -x

INSTALL_LOCATION=$1
TORCHVISION_VERSION=$2
LIBTORCH_LOCATION=$3

if [ ! "$INSTALL_LOCATION" ]; then
    echo "ERROR: Could not install Torchvision. Did not specify installation directory. Must provide installation directory as an argument"
    exit 1
fi

ZIP_PATH="${INSTALL_LOCATION}/torchvision-${TORCHVISION_VERSION}.zip"

UNZIP_DIR="${INSTALL_LOCATION}/vision-${TORCHVISION_VERSION}/"
# don't redownload and unzip dir if it already exists
if [ ! -d "${UNZIP_DIR}" ]; then
    mkdir -p "${INSTALL_LOCATION}"
    curl -L "https://github.com/pytorch/vision/archive/refs/tags/v${TORCHVISION_VERSION}.zip" --output "${ZIP_PATH}"

    unzip -o "${ZIP_PATH}" -d "${INSTALL_LOCATION}"
fi

cd ${UNZIP_DIR}
mkdir -p build
cd build
# NOTE: there is an option to build with cuda we should eventually use for the jetson
# -DWITH_CUDA=on
cmake -DCMAKE_PREFIX_PATH="\"${LIBTORCH_LOCATION}\"" "${UNZIP_DIR}"
make