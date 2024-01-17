FROM amd64/ubuntu:22.04

ARG USERNAME=tuas USER_UID=1000 USER_GID=1000 DEBIAN_FRONTEND=noninteractive

# Create a non-root user
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME

# https://gist.github.com/SSARCandy/fc960d8905330ac695e71e3f3807ce3d
# OpenCV dependencies from above
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y build-essential \
                           sudo \
                           gdb \
                           git \
                           python3 \
                           python3-pip \
                           wget \
                           ccache \
                           vim \
                           curl \
                           unzip \
                           protobuf-compiler \
                           libopencv-dev

RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Download latest CMake from their repositories
RUN apt-get update \
    && rm -rf /var/lib/apt/lists/* \
    && wget https://github.com/Kitware/CMake/releases/download/v3.27.7/cmake-3.27.7-linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /opt/cmake-3.24.1 \
      && /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake-3.24.1 \
      && rm /tmp/cmake-install.sh \
      && ln -s /opt/cmake-3.24.1/bin/* /usr/local/bin


# install prebuilt MAVSDK to system
# https://github.com/mavlink/MAVSDK/releases
RUN wget https://github.com/mavlink/MAVSDK/releases/download/v1.4.17/libmavsdk-dev_1.4.17_ubuntu20.04_amd64.deb \
    && dpkg -i libmavsdk-dev_1.4.17_ubuntu20.04_amd64.deb \
    && rm libmavsdk-dev_1.4.17_ubuntu20.04_amd64.deb

# the official docs say also these
# https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
# cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
# python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

# pull pre-built libtorch
ARG LIBTORCH_VERSION=2.1.0
ARG LIBTORCH_INSTALL_DIR=/libtorch-tmp
WORKDIR ${LIBTORCH_INSTALL_DIR}
RUN wget "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}%2Bcpu.zip" \
    && unzip "libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}+cpu.zip"
# needed to build torchvision below and to build targets that use libtorch inside the container
ENV CMAKE_PREFIX_PATH="${LIBTORCH_INSTALL_DIR}/libtorch"

# pull and build torchvision
# refer to this page for version compatibilty with pytorch (libtorch) https://github.com/pytorch/pytorch/wiki/PyTorch-Versions
ARG TORCHVISION_VERSION=0.16.0
ARG TORCHVISION_INSTALL_DIR=/torchvision-tmp
WORKDIR ${TORCHVISION_INSTALL_DIR} 
RUN wget "https://github.com/pytorch/vision/archive/refs/tags/v${TORCHVISION_VERSION}.zip" \
    && unzip "v${TORCHVISION_VERSION}.zip" \
    && mkdir -p "${TORCHVISION_INSTALL_DIR}/vision-${TORCHVISION_VERSION}/build" \
    && cd "${TORCHVISION_INSTALL_DIR}/vision-${TORCHVISION_VERSION}/build" \
    && cmake -DWITH_CUDA=off -D_GLIBCXX_USE_CXX11_ABI=1 -DCMAKE_BUILD_TYPE=Release .. \
    && make -j2 \
    && make install

RUN pip3 install cpplint

# login as non-root user
USER $USERNAME
