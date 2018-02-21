# ADA-FS
This is a file system.

# Dependencies

## Rocksdb

### Debian/Ubuntu - Dependencies

- Upgrade your gcc to version at least 4.8 to get C++11 support.
- Install gflags. First, try: `sudo apt-get install libgflags-dev` If this doesn't work and you're using Ubuntu, here's
a nice tutorial: (http://askubuntu.com/questions/312173/installing-gflags-12-04)
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
__Notice:__ Once installed, please add the include path for gflags to your CPATH environment variable and the lib path
to LIBRARY_PATH. If installed with default settings, the include path will be /usr/local/include and the lib path will
be /usr/local/lib.
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

- Go to the subfolder `ifs/scripts` and first clone all dependencies projects. You can choose the according na_plugin
(execute the script for help):

```bash
usage: dl_dep.sh [-h] [-n <NAPLUGIN>] [-c <CLUSTER>]
                    source_path
 	

This script gets all ADA-FS dependency sources (excluding the fs itself)
 
positional arguments:
        source_path              path where the dependency downloads are put
 
 
optional arguments:
        -h, --help              shows this help message and exits
        -n <NAPLUGIN>, --na <NAPLUGIN>
                                network layer that is used for communication. Valid: {bmi,cci,ofi,all}
                                defaults to 'all'
        -c <CLUSTER>, --cluster <CLUSTER>
                                additional configurations for specific compute clusters
                                supported clusters: {mogon1,fh2}
```
- Now use the install script to compile them and install them to the desired directory. You can choose the according
na_plugin (execute the script for help):

```bash
usage: compile_dep.sh [-h] [-n <NAPLUGIN>] [-c <CLUSTER>] [-j <COMPILE_CORES>]
                      source_path install_path
	
 
This script compiles all ADA-FS dependencies (excluding the fs itself)
 
positional arguments:
    source_path 	path to the cloned dependencies path from clone_dep.sh
    install_path    path to the install path of the compiled dependencies
 
 
optional arguments:
    -h, --help      shows this help message and exits
    -n <NAPLUGIN>, --na <NAPLUGIN>
                network layer that is used for communication. Valid: {bmi,cci,ofi,all}
                defaults to 'all'
    -c <CLUSTER>, --cluster <CLUSTER>
                additional configurations for specific compute clusters
                supported clusters: {mogon1,mogon2,fh2}
    -j <COMPILE_CORES>, --compilecores <COMPILE_CORES>
                number of cores that are used to compile the depdencies
                defaults to number of available cores
```

## Compile ADA-FS
You need to decide what Mercury NA plugin you want to use. The following NA plugins are available, although only BMI is
considered stable at the moment.
The following options are available with cmake:
- `-DUSE_BMI` for using the bmi plugin with the tcp protocol (Stable for TCP/IP)
- `-DUSE_CCI` for using the cci plugin with Infiniband verbs (Stable with Infiniband)
- `-DUSE_OFI_VERBS` for using the libfabric plugin with Infiniband verbs (not threadsafe. Do not use.)
- `-DUSE_OFI_PSM2` for using the libfabric plugin with Intel Omni-Path (This plugin is still in development by the
Mercury team but will be used for the Omni-Path fabric.)

In addition you can add a specific directory where all dependencies are located, i.e., headers and libraries. This can
be done by using `-DADAFS_DEPS_INSTALL=<path>`. If the variable is not set this path points to `/usr/local`.
```bash
mkdir build && cd build
cmake -DUSE_{BMI,CCI,OFI_VERBS,OFI_PSM2}:BOOL=ON -DADAFS_DEPS_INSTALL=<path> -DCMAKE_BUILD_TYPE={Release, Debug}} ..
make
```

## Run ADA-FS

First on each node a daemon has to be started. This can be done in two ways using the `adafs_daemon` binary directly or
the corresponding startup and shutdown scripts. The latter is recommended for cluster usage. It requires pssh (or
parallel-ssh) with python2.

### Start and shut down daemon directly

`./build/bin/adafs_daemon -r <fs_data_path> -m <pseudo_mount_dir_path> --hosts <hosts_comma_separated>`
 
Shut it down by gracefully killing the process.
 
### Startup and shutdown scripts

The scripts are located in `ifs/scripts/{startup_adafs.py, shutdown_adafs.py}`. Use the -h argument for their usage.

## Miscellaneous

Metadata and actual data will be stored at the `<fs_data_path>`. The path where the application works on is set with
`<pseudo_mount_dir_path>`
 
Run the application with the preload library: `LD_PRELOAD=<path>/build/lib/libiointer.so ./application`. In the case of
an MPI application use the `{mpirun, mpiexec} -x` argument.
 
Please consult `configure_public.hpp` for log locations and verbosity, etc. `include/configure.hpp` contains file system
specific configurations normal user should not be needed to modify.

