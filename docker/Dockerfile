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
    git \
    libncurses-dev \
    libftdi-dev \
    libgmp3-dev \
    libmpfr-dev \
    libmpc-dev \
    libncurses5-dev \
    libtool \
    libusb-1.0-0-dev \
    python-yaml \
    texinfo \
    zlib1g \
    zlib1g-dev \
    wget

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

WORKDIR /tmp

# install fmt-8.0.1 from source
RUN wget https://codeload.github.com/fmtlib/fmt/tar.gz/refs/tags/8.0.1 -O fmt.tar.gz && \
    tar -xvf fmt.tar.gz && cd fmt-8.0.1 && \
    rm -rf build && mkdir -p build && cd build && \
    cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE && \
    make -j$(nproc) && make install
