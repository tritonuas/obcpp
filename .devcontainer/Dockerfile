FROM amd64/ubuntu:22.04

ARG USERNAME=tuas USER_UID=1000 USER_GID=1000 DEBIAN_FRONTEND=noninteractive

# Needed to spawn up a GUI in headless mode for matplotplus to work
ENV QT_QPA_PLATFORM="vnc"

# Create a non-root user
RUN groupadd -f --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME
# exit 0 ensures that it won't prematurely stop if for some reason the user already exists

# https://gist.github.com/SSARCandy/fc960d8905330ac695e71e3f3807ce3d
# OpenCV dependencies from above
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean \ 
    && apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y build-essential \
                           software-properties-common \
                           sudo \
                           gdb \
                           git \
                           wget \
                           ccache \
                           vim \
                           curl \
                           unzip \
                           protobuf-compiler \
                           libopencv-dev \
                           # Need these to install Python 3.11 from pyenv
                           python3-pip \
                           libssl-dev \
                           zlib1g-dev \
                           libbz2-dev \
                           libreadline-dev \
                           libsqlite3-dev \
                           libncursesw5-dev \
                           xz-utils \
                           tk-dev \
                        #    libxm12-dev \
                           libxmlsec1-dev \
                           libffi-dev \
                           liblzma-dev \
                           gnuplot # needed for matplotplus

RUN apt-add-repository universe
RUN apt-get install -y cpplint

# Update Python to 3.11 so that libtorch can be properly built from source
ENV PYENV_ROOT="/.pyenv"
RUN curl https://pyenv.run | bash
ENV PATH="${PATH}:${PYENV_ROOT}/bin"
RUN eval "$(pyenv init -)"
RUN pyenv install 3.11
RUN pyenv global 3.11
RUN pip3 install typing-extensions PyYAML

RUN echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Download latest CMake from their repositories
RUN --mount=target=/var/lib/apt/lists,type=cache,sharing=locked \
    --mount=target=/var/cache/apt,type=cache,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean \ 
    && apt-get update \
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
RUN git clone -b main --recurse-submodule https://github.com/pytorch/pytorch.git
RUN mkdir pytorch-build
RUN cd pytorch-build
RUN cmake -D_GLIBCXX_USE_CXX11_ABI=1 -DBUILD_SHARED_LIBS:BOOL=ON -DCMAKE_BUILD_TYPE:STRING=Release -DPYTHON_EXECUTABLE:PATH=`which python3` -DCMAKE_PREFIX_PATH=../pytorch-install ${LIBTORCH_INSTALL_DIR}/pytorch
RUN cmake --build . --target install

# RUN wget "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}%2Bcpu.zip" \
#     && unzip "libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}+cpu.zip"
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

# login as non-root user
USER $USERNAME
