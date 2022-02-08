Forwarding mode
===============

GekkoFS is capable of working as an I/O forwarding layer by means of
the :code:`GekkoFWD` plugin. This plugin enables an I/O forwarding mode that
allows GekkoFS data servers to function as intermediate I/O nodes between
compute nodes and parallel file system servers.

In GekkoFS, data operations are typically distributed across all nodes. Once an
operation is intercepted, the client forwards it to the responsible server,
determined by hashing the file's path. Furthermore, GekkoFS uses the node-local
FS to store data and metadata and no request scheduling in the daemon.

.. image:: /images/gekkofs-small.png

In GekkoFWD, conversely, the data is forwarded to a single server
determined by an allocation policy, without breaking the file into chunks,
as the PFS already has its striping mechanism. In the daemon, requests can be
scheduled before being issued to the file system. GekkoFWD relies on the shared
PFS for storage, instead of a local store available at the compute nodes.

.. image:: /images/gekkofwd-small.png

Enabling GekkoFWD
------------------

To enable the I/O forwarding mode of GekkoFS, the
:code:`GKFS_ENABLE_FORWARDING` CMake option should be enabled, :ref:`when
configuring <building_gekkofs>` the build:

.. code-block:: console

    $ cmake -DENABLE_FORWARDING:BOOL=ON

I/O Scheduling
--------------

Because a forwarding layer is transparent to applications, it usually is the
target of I/O optimizations such as file-level request scheduling. For that
reason, we have integrated the `AGIOS <https://github
.com/francielizanon/agios>`_ scheduling library into GekkoFWD. AGIOS
has several schedulers available, and it also allows us to prototype new
scheduling solutions. On the GekkoFWD daemon running at the I/O nodes, once a
request is received, it is sent to AGIOS to determine when it should be
processed. Once scheduled, it is then dispatched and executed.

.. note::
    I/O scheduling in :code:`GekkoFWD` is an optional feature. You can enable
    it during compilation time with the :code:`GKFWD_ENABLE_AGIOS` CMake option.

You need to make sure to have the :code:`agios.conf` file in the :code:`/tmp`
folder of each node (or in another pre-defined path)· This file contains
the AGIOS configurations, such as the I/O scheduler it should use. An example
of this file can be found in the
`GekkoFS repository <https://storage.bsc.es/gitlab/hpc/gekkofs/snippets/6>`_.

Running
-------

Before running any applications with GekkoFWD, you need to have a forwarding
map file that can be dynamically updated according to an allocation policy. It
should contain a hostname of a compute node and the ID of an I/O node, to which
all intercepted requests should be forwarded. Here is an example with four
compute nodes and two I/O nodes:

.. code-block:: console

    $ cat gkfwd.map
    node-1 0
    node-2 0
    node-3 1
    node-4 1

In this scenario, :code:`node-1` and :code:`node-2` will forward all its I/O
requests to the first I/O node (i.e. node 0), while :code:`node-3` and
:code:`node-4` will forward all its I/O requests to the second I/O node (i.e.
node 1).

An environment variable named :code:`LIBGKFS_FORWARDING_MAP_FILE` is provided
to allow users to identify the map file on each client. Since
:code:`GekkoFWD` doesn't yet have a mechanism to distribute this
information across all participating nodes, this file should be stored in a
shared infrastructure such as the default PFS. This guarantees that all
:code:`GekkoFWD` clients can see the same mapping.
