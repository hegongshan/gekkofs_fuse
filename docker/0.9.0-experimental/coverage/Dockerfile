FROM debian:buster-slim

LABEL Description="Environment to generate coverage reports in GekkoFS"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget \
        git \
        cmake \
        gcc \
        g++ \
        lcov \
        python3 \
        python3-pip \
        python3-setuptools && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get clean && \
    apt-get autoclean && \
    python3 -m pip install --upgrade pip && \
    pip3 install gcovr
