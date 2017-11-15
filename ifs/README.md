# ADA-FS
This is a file system.

# Dependencies

## Rocksdb

### Debian/Ubuntu - Dependencies

- Upgrade your gcc to version at least 4.8 to get C++11 support.
- Install gflags. First, try: `sudo apt-get install libgflags-dev` If this doesn't work and you're using Ubuntu, here's a nice tutorial: (http://askubuntu.com/questions/312173/installing-gflags-12-04)
- Install snappy. This is usually as easy as: `sudo apt-get install libsnappy-dev`
- Install zlib. Try: `sudo apt-get install zlib1g-dev`
- Install bzip2: `sudo apt-get install libbz2-dev`
- Install zstandard: `sudo apt-get install libzstd-dev`
- Install lz4 `sudo apt-get install liblz4-dev`

### CentOS/Red Hat - Dependencies
- Upgrade your gcc to version at least 4.8 to get C++11 support: yum install gcc48-c++
- Install gflags:

```bash
    git clone https://github.com/gflags/gflags.git
    cd gflags
    git checkout v2.0
    ./configure && make && sudo make install
```
__Notice:__ Once installed, please add the include path for gflags to your CPATH environment variable and the lib path to LIBRARY_PATH. If installed with default settings, the include path will be /usr/local/include and the lib path will be /usr/local/lib.
- Install snappy:
    `sudo yum install snappy snappy-devel`
- Install zlib:
    `sudo yum install zlib zlib-devel`
- Install bzip2:
    `sudo yum install bzip2 bzip2-devel`
- Install ASAN (optional for debugging):
    `sudo yum install libasan`
- Install zstandard:

```bash
   wget https://github.com/facebook/zstd/archive/v1.1.3.tar.gz
   mv v1.1.3.tar.gz zstd-1.1.3.tar.gz
   tar zxvf zstd-1.1.3.tar.gz
   cd zstd-1.1.3
   make && sudo make install
```

## Mercury

- Install libev to get libev.h, ev.h etc.
`sudo apt install libev-dev` or `sudo apt install libev`

# Usage

## Clone and compile direct ADA-FS dependencies

- Go to the subfolder `ifs/scripts` and first clone all dependencies projects: `./clone_dep.sh <git_clone_path>`
- Now use the install script to compile them and install them to the desired directory: `./compile_dep.sh <git_clone_path> <install_path>`

## Compile ADA-FS
You need to decide what Mercury NA plugin you want to use. The following NA plugins are available, although only BMI is considered stable at the moment.
The following options are available with cmake:
- `-DUSE_BMI` for using the bmi plugin with the tcp protocol
- `-DUSE_CCI` for using the cci plugin with Infiniband verbs
- `-DUSE_OFI_VERBS` for using the libfabric plugin with Infiniband verbs
- `-DUSE_OFI_PSM2` for using the libfabric plugin with Intel Omnipath

```bash
mkdir build && cd build
cmake -DUSE_{BMI,CCI,OFI_VERBS,OFI_PSM2} ..
make
```

## Run ADA-FS

First on each node a daemon has to be started: 
`./build/bin/adafs_daemon -r <fs_data_path> -m <pseudo_mount_dir_path> --hosts <hosts_comma_separated>`

Metadata and actual data will be stored at the `<fs_data_path>`. The path where the application works on is set with `<pseudo_mount_dir_path>`

Run the application with the preload library: `LD_PRELOAD=<path>/build/lib/libiointer.so ./application`

Please consult `configure.hpp` for log locations and verbosity, etc.

