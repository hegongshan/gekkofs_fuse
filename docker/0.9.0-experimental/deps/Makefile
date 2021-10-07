CWD:=$(shell pwd)
GIT_ROOT:=$(shell git rev-parse --show-toplevel)

.PHONY: all build mount-scripts umount-scripts

all: build mount-scripts umount-scripts

mount-scripts:
	mkdir -p $(CWD)/scripts
	mount --bind $(GIT_ROOT)/scripts $(CWD)/scripts

build: mount-scripts
	docker build -t gekkofs/deps:0.9.0-experimental .

umount-scripts:
	- umount $(CWD)/scripts
	- rmdir $(CWD)/scripts
