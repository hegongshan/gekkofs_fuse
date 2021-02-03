FROM ruby:slim


RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        git \
        libssl-dev \
        libicu-dev \
        make \
        cmake \
        g++ \
        pkg-config \
    && \
    git clone https://github.com/cloudposse/copyright-header.git /tmp/copyright-header

COPY patch.diff /tmp/copyright-header

WORKDIR /tmp/copyright-header

RUN git apply patch.diff && \
    gem build copyright-header.gemspec && \
    gem install copyright-header-*.gem && \
    # cleanup
    rm -rf /tmp/copyright-header && \
    apt-get purge -y \
        git \
        libssl-dev \
        libicu-dev \
        make \
        cmake \
        g++ \
        pkg-config \
    && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get clean && \
    apt-get autoclean

VOLUME ["/usr/src"]

WORKDIR /usr/src

ENTRYPOINT ["/usr/local/bundle/bin/copyright-header"]
