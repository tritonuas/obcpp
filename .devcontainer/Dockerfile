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
                           gdb \
                           git \
                           clang-tidy \
                           wget \
                           ccache \
                           vim \
                           curl \
                           unzip \
                           libopencv-dev

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

# the official docs say also these
# https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
# cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
# python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev

# Add clang-tidy-cache (doesn't work)
ARG CLANG_TIDY_SHA1=c191254ea00d47ade11d7170ef82fe038c213774
RUN curl -Lo /usr/bin/clang-tidy-cache \
        "https://raw.githubusercontent.com/matus-chochlik/ctcache/$CLANG_TIDY_SHA1/clang-tidy-cache" \
    && chmod +x /usr/bin/clang-tidy-cache

# pull pre-built torch
WORKDIR /stuff
RUN wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-shared-with-deps-latest.zip
RUN unzip libtorch-shared-with-deps-latest.zip
ENV CMAKE_PREFIX_PATH="/stuff/libtorch"

# pull and build torchvision
WORKDIR /stuff
RUN wget "https://github.com/pytorch/vision/archive/refs/tags/v0.16.0.zip"
RUN unzip v0.16.0.zip
WORKDIR /stuff/vision-0.16.0/build
RUN cmake -DWITH_CUDA=off -DCMAKE_PREFIX_PATH="/stuff/libtorch" ..
RUN make
RUN make install

# login as non-root user
USER $USERNAME
