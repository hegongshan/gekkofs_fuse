FROM debian:stable-slim

LABEL Description="This is a debian based environment to build GekkoFS"

ENV GKFS_PATH	/opt/gkfs

ENV SCRIPTS_PATH	${GKFS_PATH}/scripts
ENV DEPS_SRC_PATH	${GKFS_PATH}/deps_src
ENV INSTALL_PATH	${GKFS_PATH}/build_deps

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
		# RocksDB
		libsnappy-dev \
		liblz4-dev \
		libzstd-dev \
		libbz2-dev \
		zlib1g-dev \
		# syscall_intercept dependencies
		libcapstone-dev \
		# GekkoFS
		libboost-filesystem-dev \
		libboost-program-options-dev \
		valgrind \
		uuid-dev \
		python3 \
		python3-pip \
		python3-dev \
		python3-venv \
		python3-setuptools \
		expect \
		# clang 10 deps
		lsb-release \
		wget \
		software-properties-common \
		gnupg2 \
    # add clang-10 repos
    wget https://apt.llvm.org/llvm.sh -P /tmp && chmod +x /tmp/llvm.sh && /tmp/llvm.sh 10 && \
    # install clang-format
    apt-get update && apt-get install -y --no-install-recommends clang-format-10 && \
    # install gcovr
    # (required for partial coverage reports in parallel runs)
    python3 -m pip install --upgrade pip && \
    pip3 install gcovr && \
    # Clean apt cache to reduce image layer size
    rm -rf /var/lib/apt/lists/* && rm /tmp/llvm.sh && \
    # Clean apt caches of packages
    apt-get clean && apt-get autoclean

## COPY scripts/dl_dep.sh		$SCRIPTS_PATH/
## COPY scripts/compile_dep.sh $SCRIPTS_PATH/
## COPY scripts/patches        $SCRIPTS_PATH/patches
## 
## # Download dependencies source
## RUN /bin/bash $SCRIPTS_PATH/dl_dep.sh $DEPS_SRC_PATH all
## 
## # Compile dependencies
## RUN /bin/bash $SCRIPTS_PATH/compile_dep.sh $DEPS_SRC_PATH $INSTALL_PATH
