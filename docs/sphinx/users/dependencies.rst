====================
GekkoFS dependencies
====================

--------
Required
--------

- A compiler with C++17 support, i.e. `GCC <https://gcc.gnu.org>`_ version 8.0 and newer or `Clang <https://clang.llvm.org/>`_ 5.0 and newer.

- `CMake <https://cmake.org>`_ version 3.6 or newer to build the main code. Version 3.11 or newer to build the tests.

- `RocksDB <https://github.com/facebook/rocksdb/>`_ version 6.2.2 or newer and its dependencies:

  - `bzip2 <https://www.sourceware.org/bzip2/>`_ version 1.0.6 or newer.

  - `zstd <https://github.com/facebook/zstd>`_ version 1.3.2 or newer.

  - `lz4 <https://github.com/lz4/lz4>`_ version 1.8.0 or newer.

  - `snappy <https://github.com/google/snappy>`_ version 1.1.7 or newer.


- `Margo <https://github.com/mochi-hpc/mochi-margo/releases>`_ version 0.6.3 and its dependencies:

  - `Argobots <https://github.com/pmodels/argobots/releases/tag/v1.0.1>`_ version 1.0rc1.
  - `Mercury <https://github.com/mercury-hpc/mercury/releases/tag/v2.0.0>`_ version 2.0.0.

    - `libfabric <https://github.com/ofiwg/libfabric>`_ and/or `bmi <https://github.com/radix-io/bmi/>`_.


- `syscall_intercept <https://github.com/pmem/syscall_intercept>`_ (commit f7cebb7 or newer) and its dependencies:
  - `capstone <https://www.capstone-engine.org/>`_ version 4.0.1 or newer.

- `date <https://github.com/HowardHinnant/date>`_  (commit e7e1482 or newer).

.. important::
    
    Mercury may need additional plugins depending on the fabric technology used in the cluster. For instance,
    OmniPath requires the `opa-psm2 <https://github.com/cornelisnetworks/opa-psm2>`_ library to work.

--------
Optional
--------

- `agios <https://github.com/francielizanon/agios>`_ (commit c26a654 or newer) to enable GekkoFWD mode.
