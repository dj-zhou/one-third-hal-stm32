# syntax=docker/dockerfile:experimental

# prepare the docker
FROM ubuntu:20.04 AS build
ARG DEBIAN_FRONTEND=noninteractive

# By default, ubuntu docker images include code to remove downloaded packages.
# Use the invocation from examples to avoid this and make the apt cache effective.
RUN rm -f /etc/apt/apt.conf.d/docker-clean \
    && echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' \
    > /etc/apt/apt.conf.d/keep-cache

# make sure the command add-apt-repository exists in the next step
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    apt-get update && apt-get --no-install-recommends install -y \
    software-properties-common

# run export DOCKER_BUILDKIT=1 to enable BuildKit
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get --no-install-recommends install -y \
    autoconf\
    bison \
    build-essential \
    cmake \
    flex \
    gettext \
    libncurses-dev \
    libftdi-dev \
    libgmp3-dev \
    liblz4-tool \
    libmpfr-dev \
    libmpc-dev \
    libncurses5-dev \
    libtool \
    libusb-1.0-0-dev \
    openssh-client \
    python-yaml \
    texinfo \
    zlib1g \
    zlib1g-dev \
    wget

# error seen from ./build-in-container:
# fatal: unsafe repository ('/one-third-in-container' is owned by someone else)
# a good solution is to downgrade the git version
WORKDIR /tmp
RUN wget https://mirrors.edge.kernel.org/pub/software/scm/git/git-2.15.1.tar.xz --no-check-certificate && \
    tar -xvf git-2.15.1.tar.xz && cd git-2.15.1 && \
    make configure && ./configure --prefix=/usr && \
    make -j$(nproc) && make install

# install gcc-arm-none-eabi --------------------
ARG file="gcc-arm-none-eabi-10.3-2021.10"
ARG filename="${file}-x86_64-linux.tar.bz2"
ARG url="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm"
ARG link="${url}/10.3-2021.10/${filename}"

RUN wget "$link" --no-check-certificate  && \
    tar xjf ${filename} -C /usr/share/

RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-ar /usr/bin/arm-none-eabi-ar && \
    ln -sf /usr/share/${file}/bin/arm-none-eabi-gcc /usr/bin/arm-none-eabi-gcc  && \
    ln -sf /usr/share/${file}/bin/arm-none-eabi-g++ /usr/bin/arm-none-eabi-g++  && \
    ln -sf /usr/share/${file}/bin/arm-none-eabi-gdb /usr/bin/arm-none-eabi-gdb  && \
    ln -sf /usr/share/${file}/bin/arm-none-eabi-objcopy /usr/bin/arm-none-eabi-objcopy  && \
    ln -sf /usr/share/${file}/bin/arm-none-eabi-size /usr/bin/arm-none-eabi-size


# install fmt-8.0.1 from source --------------------
WORKDIR /tmp
RUN wget https://codeload.github.com/fmtlib/fmt/tar.gz/refs/tags/8.0.1 -O fmt.tar.gz && \
    tar -xvf fmt.tar.gz && cd fmt-8.0.1 && \
    rm -rf build && mkdir -p build && cd build && \
    cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE && \
    make -j$(nproc) && make install

# install magic enum --------------------
WORKDIR /tmp
RUN wget https://codeload.github.com/Neargye/magic_enum/tar.gz/refs/tags/v0.7.3 -O magic_enum.tar.gz && \
   tar -xvf magic_enum.tar.gz && cd magic_enum-0.7.3 && \
    cp include/magic_enum.hpp /usr/local/include/
