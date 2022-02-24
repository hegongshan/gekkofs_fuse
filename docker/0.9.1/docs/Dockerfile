FROM gekkofs/deps:0.9.1

LABEL Description="Debian-based environment suitable to build GekkoFS' documentation"

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        # install dependencies for Doxygen
        python \
        flex \
        bison \
		graphviz && \
	# install doxygen (repo version is kind of old)
	cd /tmp && curl -OL https://www.doxygen.nl/files/doxygen-1.9.2.src.tar.gz && \
	tar xvfz /tmp/doxygen-1.9.2.src.tar.gz && \
	mkdir -p /tmp/doxygen-1.9.2/build && \
	cd /tmp/doxygen-1.9.2/build && \
	cmake -G "Unix Makefiles" .. && \
	make -j8 install && \
    # install sphinx, breathe and exhale
    pip3 install \
        'sphinx==4.4.0' \
        sphinx_rtd_theme \
        'breathe==4.33.1' \
        'exhale==0.3.1' \
        'sphinx-copybutton==0.5.0' \
        'sphinx-multiversion==0.2.4' \
        'myst_parser==0.17.0' && \
    # Clean apt cache to reduce image layer size
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /tmp/doxygen-1.9.2 && \
    rm /tmp/doxygen-1.9.2.src.tar.gz && \
    # Clean apt caches of packages
    apt-get clean && apt-get autoclean
