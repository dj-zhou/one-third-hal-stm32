# syntax=docker/dockerfile:experimental

# prepare the docker   -----------------
FROM ubuntu:20.04 AS build
ARG DEBIAN_FRONTEND=noninteractive
# By default, ubuntu docker images include code to remove downloaded packages.
# Use the invocation from examples to avoid this and make the apt cache effective.
RUN rm -f /etc/apt/apt.conf.d/docker-clean \
    && echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' \
    > /etc/apt/apt.conf.d/keep-cache

# run export DOCKER_BUILDKIT=1 to enable BuildKit
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    apt-get update && apt-get --no-install-recommends install -y \
    build-essential git flex bison libgmp3-dev libmpfr-dev \ 
    libncurses5-dev libmpc-dev autoconf texinfo libtool \
    libftdi-dev libusb-1.0-0-dev zlib1g zlib1g-dev python-yaml \ 
    libncurses-dev wget


ARG compiler_date="9-2020"
ARG compiler_q="q2"
ARG url="https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm"
ARG file="gcc-arm-none-eabi-${compiler_date}-${compiler_q}-update"
ARG filename="${file}-x86_64-linux.tar.bz2"

RUN wget "${url}/${compiler_date}${compiler_q}/${filename}" --no-check-certificate
RUN tar xjf ${filename} -C /usr/share/

RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-ar /usr/bin/arm-none-eabi-ar
RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-gcc /usr/bin/arm-none-eabi-gcc
RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-g++ /usr/bin/arm-none-eabi-g++
RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-gdb /usr/bin/arm-none-eabi-gdb
RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-objcopy /usr/bin/arm-none-eabi-objcopy
RUN ln -sf /usr/share/${file}/bin/arm-none-eabi-size /usr/bin/arm-none-eabi-size

# mount the project directory to the container
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    --mount=type=bind,source=../,target=/proj
