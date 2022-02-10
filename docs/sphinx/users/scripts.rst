Scripts
=======

The GekkoFS package includes several scripts that automate/simplify common
tasks. This section describes such scripts, which can be classified as follows:

.. contents::
    :local:
    :depth: 1
    :backlinks: none

Dependency Management Scripts
-----------------------------

GekkoFS provides two scripts for managing dependencies which can be found in
the :code:`PROJECT_ROOT/scripts` directory: :code:`dl_dep.sh` and
:code:`compile_dep.sh`. As their names suggest, the :code:`dl_dep.sh` script
is in charge of downloading any dependencies required by GekkoFS to build or
run correctly, while :code:`compile_dep.sh` is responsible for installing them
once downloaded.

Since dependencies may change or need to be configured differently depending
on the specifics of the particular GekkoFS build, both scripts rely on
:code:`configuration profiles` which define a set of related software
packages which should be downloaded and installed for a specific GekkoFS
version and/or configuration. To illustrate this, let's take a look at the
contents of the :code:`default` profile for GekkoFS version :code:`0.8.0`:

.. code-block:: console

    $ dl_dep.sh -l default:0.8.0
    Configuration profiles for 'default:0.8.0':

    * default:0.8.0 (/home/user/gekkofs/source/scripts/profiles/0.8.0/default.specs)

      All dependencies

        bzip2: 1.0.6
        zstd: 1.3.2
        lz4: 1.8.0
        snappy: 1.1.7
        capstone: 4.0.1
        bmi: 6ea0b78fce1b964e45102828cdd05df7040a94c8
        libfabric: HEAD@v1.8.1
        libfabric%experimental: HEAD@v1.9.1
        libfabric%verbs: HEAD@v1.7.2
        mercury: 41caa143a07ed179a3149cac4af0dc7aa3f946fd
        argobots: 1.0rc1
        margo: v0.6.3
        rocksdb: 6.2.2
        rocksdb%experimental: 6.11.4
        syscall_intercept: f7cebb7b7e7512a19b78a31ce236ad6ca22636dd
        date: e7e1482087f58913b80a20b04d5c58d9d6d90155
        psm2: 11.2.86
        agios: c26a6544200f823ebb8f890dd94e653d148bf226@development



Downloading dependencies
########################

Dependencies defined in a profile can be downloaded using the :code:`dl_dep.sh`
script and the :code:`-p` option. As shown below, profile names follow the
:code:`PROFILE_NAME[:VERSION_TAG]` naming convention, where :code:`PROFILE_NAME`
serves to uniquely identify a particular configuration (e.g. for a specific
supercomputer) followed by an optional :code:`VERSION_TAG`.

.. code-block:: console

   $ ./dl_dep.sh -p default:0.8.0 /home/user/gfks/deps
   Destination path is set to  "/tmp/foo"
   Profile name: default
   Profile version: 0.8.0
   ------------------------------------
   Cloned 'https://github.com/francielizanon/agios.git' to 'agios' with commit '[c26a6544200f823ebb8f890dd94e653d148bf226]' and flags '--branch=development'
   Downloaded 'https://github.com/pmodels/argobots/archive/v1.0rc1.tar.gz' to 'argobots'
   Downloaded 'https://github.com/google/snappy/archive/1.1.7.tar.gz' to 'snappy'
   Cloned 'https://github.com/radix-io/bmi/' to 'bmi' with commit '[6ea0b78fce1b964e45102828cdd05df7040a94c8]' and flags ''
   Downloaded 'https://github.com/lz4/lz4/archive/v1.8.0.tar.gz' to 'lz4'
   Cloned 'https://github.com/pmem/syscall_intercept.git' to 'syscall_intercept' with commit '[f7cebb7b7e7512a19b78a31ce236ad6ca22636dd]' and flags ''
   Checking patch include/libsyscall_intercept_hook_point.h...
   Checking patch src/intercept.c...
   Hunk #2 succeeded at 700 (offset 31 lines).
   Hunk #3 succeeded at 730 (offset 31 lines).
   Checking patch test/test_clone_thread_preload.c...
   Applied patch include/libsyscall_intercept_hook_point.h cleanly.
   Applied patch src/intercept.c cleanly.
   Applied patch test/test_clone_thread_preload.c cleanly.
   Downloaded 'https://github.com/facebook/zstd/archive/v1.3.2.tar.gz' to 'zstd'
   Downloaded 'https://github.com/intel/opa-psm2/archive/PSM2_11.2.86.tar.gz' to 'psm2'
   Downloaded 'https://github.com/aquynh/capstone/archive/4.0.1.tar.gz' to 'capstone'
   Cloned 'https://github.com/HowardHinnant/date.git' to 'date' with commit '[e7e1482087f58913b80a20b04d5c58d9d6d90155]' and flags ''
   Cloned 'https://xgitlab.cels.anl.gov/sds/margo.git' to 'margo' with commit '[v0.6.3]' and flags ''
   Downloaded 'https://github.com/facebook/rocksdb/archive/v6.2.2.tar.gz' to 'rocksdb'
   Downloaded 'https://github.com/facebook/rocksdb/archive/v6.11.4.tar.gz' to 'rocksdb%experimental'
   Cloned 'https://github.com/mercury-hpc/mercury' to 'mercury' with commit '[41caa143a07ed179a3149cac4af0dc7aa3f946fd]' and flags '--recurse-submodules'
   Cloned 'https://github.com/ofiwg/libfabric.git' to 'libfabric%verbs' with commit '[HEAD]' and flags '--branch=v1.7.2'
   Cloned 'https://github.com/ofiwg/libfabric.git' to 'libfabric' with commit '[HEAD]' and flags '--branch=v1.8.1'
   Cloned 'https://github.com/ofiwg/libfabric.git' to 'libfabric%experimental' with commit '[HEAD]' and flags '--branch=v1.9.1'
   Downloaded 'https://sourceforge.net/projects/bzip2/files/bzip2-1.0.6.tar.gz' to 'bzip2'
   Done

It is also possible to download a specific dependency with the :code:`-d`
option. In this case, dependency names follow the
:code:`DEPENDENCY_NAME[@PROFILE_NAME[:VERSION_TAG]]` naming convention.

.. code-block:: console

   $ ./dl_dep.sh -d mercury@default:0.8.0 /home/user/gfks/deps
   Destination path is set to  "/tmp/foo"
   Profile name: default
   Profile version: 0.8.0
   ------------------------------------
   Cloned 'https://github.com/mercury-hpc/mercury' to 'mercury' with commit '[41caa143a07ed179a3149cac4af0dc7aa3f946fd]' and flags '--recurse-submodules'
   Done

.. warning::

    Note that :code:`PROFILE_NAME` and :code:`VERSION_TAG` can be optional
    in most script invocations. If :code:`PROFILE_NAME` is left unspecified,
    the scripts will assume that the :code:`default` profile was selected.
    Similarly, if a :code:`VERSION_NAME` is not provided, the scripts will
    assume that the :code:`latest` version should be used.

Installing dependencies
########################

Once dependencies in a configuration profile have been downloaded to a
certain directory (e.g. :code:`/home/user/gkfs/deps`), the
:code:`compile_dep.sh` script can be used to install them.

.. code-block:: console

   $ ./compile_dep.sh -p default:0.8.0 /home/user/gkfs/deps /home/user/gkfs/install -j8
   CORES = 8 (default)
   Sources download path = /tmp/foo
   Installation path = /tmp/bar
   Profile name: default
   Profile version: 0.8.0
   ------------------------------------


   ######## Installing:  bzip2 ###############################
   ...

   ######## Installing:  zstd ###############################
   ...

   ######## Installing:  lz4 ###############################
   ...

   ######## Installing:  snappy ###############################
   ...

   ######## Installing:  capstone ###############################
   ...

   ######## Installing:  bmi ###############################
   ...

   ######## Installing:  libfabric ###############################
   ...

   ######## Installing:  mercury ###############################
   ...

   ######## Installing:  argobots ###############################
   ...

   ######## Installing:  margo ###############################
   ...

   ######## Installing:  rocksdb ###############################
   ...

   ######## Installing:  syscall_intercept ###############################
   ...

   ######## Installing:  date ###############################
   ...

   ######## Installing:  psm2 ###############################
   ...

   ######## Installing:  agios ###############################
   ...
   Done
