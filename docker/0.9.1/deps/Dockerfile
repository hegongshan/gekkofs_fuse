FROM gekkofs/core:0.9.1

LABEL Description="Debian-based environment to build GekkoFS"

ENV GKFS_PATH	/opt/gkfs
ENV GKFS_VERSION 0.9.1

ENV SCRIPTS_PATH	${GKFS_PATH}/scripts
ENV DEPS_SRC_PATH	${GKFS_PATH}/deps_src
ENV INSTALL_PATH	/usr/local

COPY scripts/dl_dep.sh		${SCRIPTS_PATH}/
COPY scripts/compile_dep.sh ${SCRIPTS_PATH}/
COPY scripts/patches        ${SCRIPTS_PATH}/patches
COPY scripts/profiles       ${SCRIPTS_PATH}/profiles

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
		python3 \
		python3-pip \
		python3-dev \
		python3-venv \
		python3-setuptools \
    libnuma-dev libyaml-dev libcurl4-openssl-dev \
    procps && \
    python3 -m pip install --upgrade pip && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get clean && apt-get autoclean
 
# Download and build dependencies
RUN cd ${SCRIPTS_PATH} && \
    /bin/bash ./dl_dep.sh -p ci:${GKFS_VERSION} ${DEPS_SRC_PATH} && \
    /bin/bash ./compile_dep.sh -j 8 -p ci:${GKFS_VERSION} ${DEPS_SRC_PATH} ${INSTALL_PATH} && \
    rm -rf ${DEPS_SRC_PATH} && \
    rm -rf ${SCRIPTS_PATH} && \
    rmdir ${GKFS_PATH} && \
    ldconfig
