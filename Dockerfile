FROM ubuntu:22.04
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y build-essential \
                           gdb \
                           git \
                           cmake
