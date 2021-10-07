FROM gekkofs/deps:0.9.0-experimental

LABEL Description="Debian-based environment to test GekkoFS"

RUN \
    # install cmake 3.21 since we need to produce JUnit XML files
    curl -OL https://github.com/Kitware/CMake/releases/download/v3.21.1/cmake-3.21.1-linux-x86_64.sh && \
    chmod u+x ./cmake-3.21.1-linux-x86_64.sh && \
    ./cmake-3.21.1-linux-x86_64.sh --skip-license --prefix=/usr && \
    # install gcovr
    # (required for partial coverage reports in parallel runs)
    pip3 install gcovr && \
    # cleanup
    rm ./cmake-3.21.1-linux-x86_64.sh
