FROM debian:bullseye-slim

LABEL Description="Debian-based environment suitable to build GekkoFS and its dependencies"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
		git \
		curl \
		ca-certificates \
		libtool \
		pkg-config \
		make \
		automake \
		gcc \
		g++ \
		ninja-build \
		procps \
		# AGIOS dependencies
		libconfig-dev \
		# Mercury dependencies
		libltdl-dev \
		lbzip2 \
        # Margo dependencies \
        libjson-c-dev \
		# RocksDB dependencies
		liblz4-dev \
		# syscall_intercept dependencies
		libcapstone-dev \
		# GekkoFS dependencies
		libboost-program-options-dev \
		uuid-dev && \
    # install cmake 3.14+ since it's needed for some dependencies
    curl -OL https://github.com/Kitware/CMake/releases/download/v3.25.2/cmake-3.25.2-Linux-x86_64.sh && \
    chmod u+x ./cmake-3.25.2-Linux-x86_64.sh && \
    ./cmake-3.25.2-Linux-x86_64.sh --skip-license --prefix=/usr && \
    # Clean apt cache to reduce image layer size
    rm -rf /var/lib/apt/lists/* && \
    # Clean apt caches of packages
    apt-get clean && apt-get autoclean && \
    rm ./cmake-3.25.2-Linux-x86_64.sh
