FROM debian:buster-slim

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
		cmake \
		gcc \
		g++ \
		# AGIOS dependencies
		libconfig-dev \
		# Mercury dependencies
		libltdl-dev \
		lbzip2 \
		# RocksDB dependencies
		libsnappy-dev \
		liblz4-dev \
		libzstd-dev \
		libbz2-dev \
		zlib1g-dev \
		# syscall_intercept dependencies
		libcapstone-dev \
		# GekkoFS dependencies
		libboost-filesystem-dev \
		libboost-program-options-dev \
		uuid-dev && \
    # Clean apt cache to reduce image layer size
    rm -rf /var/lib/apt/lists/* && \
    # Clean apt caches of packages
    apt-get clean && apt-get autoclean
