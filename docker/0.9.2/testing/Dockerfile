FROM gekkofs/deps:0.9.2

LABEL Description="Debian-based environment to test GekkoFS"

RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends \
        # required by lcov's genhtml
        libgd-perl && \
    # install cmake 3.21+ since we need to produce JUnit XML files
    curl -OL https://github.com/Kitware/CMake/releases/download/v3.25.2/cmake-3.25.2-Linux-x86_64.sh && \
    chmod u+x ./cmake-3.25.2-Linux-x86_64.sh && \
    ./cmake-3.25.2-Linux-x86_64.sh --skip-license --prefix=/usr && \
    # install loguru
    # (required by several of our scripts)
    pip3 install loguru && \
    # install lcov_cobertura
    # (required to produce Cobertura XML reports)
    pip3 install lcov_cobertura && \
    # install lcov
    # (required to produce partial coverage reports in parallel runs)
    curl -OL https://github.com/linux-test-project/lcov/releases/download/v1.16/lcov-1.16.tar.gz && \
    tar xfz lcov-1.16.tar.gz && \
    cd lcov-1.16 && \
    make install && \
    cd .. && \
    # cleanup
    rm -rf /var/lib/apt/lists/* && \
    apt-get clean && \
    apt-get autoclean && \
    rm ./cmake-3.25.2-Linux-x86_64.sh && \
    rm -rf ./lcov-1.16.*
