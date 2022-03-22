CWD:=$(shell pwd)
GIT_ROOT:=$(shell git rev-parse --show-toplevel)

.PHONY: all build mount-scripts umount-scripts

all: build mount-scripts remove-scripts
amd64: build-amd64 mount-scripts remove-scripts
aarch64: build-aarch64 mount-scripts remove-scripts

copy-scripts:
	cp -R $(GIT_ROOT)/scripts $(CWD)/scripts

build: copy-scripts
	docker build -t gekkofs/deps:0.9.1 .

build-amd64: copy-scripts
	docker build --platform amd64 -t gekkofs/deps:0.9.1 .

build-aarch64: copy-scripts
	docker build --platform aarch64 -t gekkofs/deps:0.9.1 .

remove-scripts:
	- rm -rf $(CWD)/scripts
