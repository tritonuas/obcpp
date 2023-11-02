FROM amd64/ubuntu:22.04

# Create a non-root user
ARG USERNAME=tuas
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ARG DEBIAN_FRONTEND=noninteractive
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME


# https://gist.github.com/SSARCandy/fc960d8905330ac695e71e3f3807ce3d
# OpenCV dependencies from above
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y build-essential \
                           gdb \
                           git \
                           clang-tidy \
                           wget \
                           ccache \
                           vim \
                           libopencv-dev 
# cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
# python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

# the official docs say also these
# https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
# sudo apt update && sudo apt install -y cmake g++ wget unzip
# xz? bzip2?

# TODO: is it possible to save the built opencv in the docker build? Need to see what cmake keeps checking and wasting time on.

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
    && dpkg -i  libmavsdk-dev_1.4.17_ubuntu20.04_amd64.deb \
    && rm libmavsdk-dev_1.4.17_ubuntu20.04_amd64.deb

# login as non-root user
USER $USERNAME
