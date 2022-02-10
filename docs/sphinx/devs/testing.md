# Testing

This page describes the testing infrastructure of the GekkoFS file system. It is
intended to provide a general idea of how the different infrastructure
components are organized and their interactions, so that developers can use them
to write new tests (and/or extend the framework) with as few prototyping
overhead as possible.

## Integration and functionality tests

GekkoFS provides an automated testing harness to simplify writing and running
integration and functional tests for the file system. For simplicity and ease of
development, tests are written in Python (3.6+ required). The harness itself
relies on the [pytest](https://docs.pytest.org/en/latest/) framework to simplify
testing the GekkoFS infrastructure. Among others, `pytest` offers the following
features:

- Detailed info on failing assert statements;
- [Auto-discovery](https://docs.pytest.org/en/latest/goodpractices.html#test-discovery)
  of test modules and functions;
- [Modular fixtures](https://docs.pytest.org/en/latest/fixture.html#fixture) for
  managing small or parametrized long-lived test resources;
- Rich plugin architecture, in case further functionality is required.

The code below shows an example of a simple test that creates a directory on the
file system's root by invoking `mkdir()` and checks the returned value
and `errno` error code.

```python
import harness, errno, stat
from pathlib import Path


def test_mkdir(gkfs_daemon, gkfs_client):
    """Create a new directory in the FS's root"""

    topdir = gkfs_daemon.mountdir / "top"

    ret = gkfs_client.mkdir(
        topdir,
        stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)

    assert ret.retval == 0
    assert ret.errno == 0
```

### Executing functional tests

In order for tests to be built, the `GKFS_BUILD_TESTS` option needs to be
enabled when configuring the package:

```console
$ mkdir build && cd build
$ cmake -DGKFS_BUILD_TESTS=ON ..
$ make -j8
```

Once built, tests can be run in several ways:

```console
# 1. run all tests registered with CTest
$ make test

# 2. build and run all tests registered with CTest
# (equivalent to ctest --force-new-ctest-process --verbose --output-on-failure)
$ make check

# 3. configure the run (see `ctest --help` for available options)
$ ctest [ options ]
```

It is also possible to invoke `pytest` directly by taking advantage of the
Python virtualenv that CMake creates to run the tests. This has the advantage
that the user can pass arguments directly to the `pytest` framework and that
pretty-printed output is sent directly to the console. 

```{eval-rst}
.. note::
  :code:`py.test` must be run from the :code:`build/tests/integration` 
  directory to work.
```


```console
# option 1: run the 'mkdir' test directly from the 'build/tests' directory 
# in verbose mode, without console capture and using the 'docker0' interface
$ pytest-venv/bin/py.test -s -v --interface=docker0 -k test_mkdir


# option2: same example but activating the virtualenv
$ source pytest-venv/bin/activate
(pytest-venv) $ py.test -s -v --interface=docker0 -k test_mkdir
(pytest-venv) $ deactivate
```

In both cases, the test output should be similar to this:

```shell
===================================================================================== test session starts ======================================================================================
platform linux -- Python 3.7.3, pytest-5.3.5, py-1.8.1, pluggy-0.13.1 -- /opt/gekkofs/build/tests/pytest-venv/bin/python3.7
cachedir: .pytest_cache
rootdir: /opt/gekkofs/build/tests, inifile: pytest.ini, testpaths: /opt/gekkofs/tests
plugins: xdist-1.31.0, dependency-0.4.0, forked-1.1.3
collected 20 items / 19 deselected / 1 selected                                                                                                                                                  

test_directories.py::test_mkdir 2020-02-25 14:42:08.923 | INFO     | harness.workspace:__init__:43 - workspace created at /tmp/pytest-of-testuser/pytest-190/test_mkdir0
2020-02-25 14:42:08.924 | INFO     | harness.gkfs:run:122 - spawning daemon
2020-02-25 14:42:08.924 | INFO     | harness.gkfs:run:123 - cmdline: /opt/gekkofs/build/src/daemon/gkfs_daemon --mountdir /tmp/pytest-of-testuser/pytest-190/test_mkdir0/mnt --rootdir /tmp/pytest-of-testuser/pytest-190/test_mkdir0/root -l docker0:12769
2020-02-25 14:42:08.924 | INFO     | harness.gkfs:run:124 - env: {'LD_LIBRARY_PATH': ':/opt/gekkofs/build/tests:/opt/gekkofs/prefix/lib', 'GKFS_HOSTS_FILE': PosixPath('/tmp/pytest-of-testuser/pytest-190/test_mkdir0/gkfs_hosts.txt'), 'GKFS_DAEMON_LOG_PATH': PosixPath('/tmp/pytest-of-testuser/pytest-190/test_mkdir0/logs/gkfs_daemon.log'), 'GKFS_LOG_LEVEL': '100'}
2020-02-25 14:42:08.927 | INFO     | harness.gkfs:run:134 - daemon process spawned (PID=14946)
2020-02-25 14:42:09.040 | INFO     | harness.gkfs:run:251 - running client
2020-02-25 14:42:09.041 | INFO     | harness.gkfs:run:252 - cmdline: /opt/gekkofs/build/tests/harness/gkfs.io /tmp/pytest-of-testuser/pytest-190/test_mkdir0/mnt/top 511
2020-02-25 14:42:09.042 | INFO     | harness.gkfs:run:253 - env: {'LD_LIBRARY_PATH': ':/opt/gekkofs/build/tests:/opt/gekkofs/prefix/lib', 'LD_PRELOAD': PosixPath('libgkfs_intercept.so'), 'LIBGKFS_HOSTS_FILE': PosixPath('/tmp/pytest-of-testuser/pytest-190/test_mkdir0/gkfs_hosts.txt'), 'LIBGKFS_LOG': 'all', 'LIBGKFS_LOG_OUTPUT': PosixPath('/tmp/pytest-of-testuser/pytest-190/test_mkdir0/logs/gkfs_client.log')}
2020-02-25 14:42:09.390 | DEBUG    | harness.gkfs:run:262 - command output: b'{\n  "errnum": 0,\n  "retval": 0\n}\n'
PASSED
2020-02-25 14:42:15.219 | INFO     | harness.gkfs:shutdown:197 - terminating daemon


=============================================================================== 1 passed, 19 deselected in 1.54s ================================================================================

```

```{eval-rst}
.. warning::

  Be careful not to run :code:`make` with the virtualenv activated. When that 
  happens, CMake considers the virtualenv's Python interpreter as the 
  system-wide one and caches this information, thus failing to run tests when 
  the virtualenv is deactivated.
```
***

### Integration with CMake

As shown in the examples above, the functional testing harness is integrated
with CMake's built-in testing tool. The CMake software suite includes
the `CTest` tool which can be used to automate the testing phase, or even the
entire process of configuring, building, testing and even submitting results to
a dashboard. Thus, integrating the `pytest` testing harness with it allows
end-users to easily execute tests, and also integrate with expected CMake
workflows. Nevertheless, `CTest` requires all tests to be defined by hand in any
involved `CMakeLists.txt`, whereas `pytest` is capable of automatically finding
all Pyhton test functions whose names follow certain patterns, which is why
tests are *semantically organized* into directories, and test groups are added
to CTest as shown below.

#### Adding new tests to CMake

The GekkoFS testing framework provides the `gkfs_add_python_test` CMake function
to simplify creating test groups. Thus, if a `${GKFS_ROOTDIR}/tests/io`
subdirectory exists that should contain all the tests that exercise and verify
that I/O works as expected, the test group could be added to CMake by adding the
following code to `${GKFS_ROOTDIR}/tests/CMakeLists.txt`:

```cmake
gkfs_add_python_test(
  NAME test_io
  PYTHON_VERSION 3.6
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  SOURCE tests/io/
)
```

The function creates a new `CTest` test called `test_io` that will internally
call `pytest` and instruct it to auto-discover tests starting on the file or
directory defined by `SOURCE`. It is also possible to define
the `WORKING_DIRECTORY` required for the test, though for now only a value
of `${PROJECT_SOURCE_DIR}` is supported.

### Testing harness organization

The testing harness resides on the `${GKFS_ROOTDIR}/tests` subdirectory and
consists of the following files, directories, and helper programs:

- `pytest.ini.in`: `pytest` determines a `rootdir` for each test run which
  depends on the command line arguments (specified test files, paths) and on the
  existence of *ini-files*, which allow to set pre-configured settings for a
  test run without having to rely on command line arguments (
  see [here](https://docs.pytest.org/en/latest/customize.html#initialization-determining-rootdir-and-inifile))
  . Unfortunately, `pytest`'s `rootdir` finding algorithm will always determine
  that the first directory to contain a `pytest.ini` file will become
  the `rootdir`. Since we want to be able to run the tests by
  invoking `make test` from `${CMAKE_BINARY_DIR}`, **but** we don't want to copy
  the tests source files to the binary directory, we use this template to
  generate a `pytest.ini` file that instructs `pytest` to run the tests
  in `${CMAKE_CURRENT_SOURCE_DIR}`, i.e. our main `tests` subdirectory.

- `conftest.py`: This is the main `pytest` configuration file that should
  contain the definition of all fixtures that should be shared by multiple
  tests (refer to `pytest`'
  s [documentation](https://docs.pytest.org/en/latest/fixture.html#fixtures-a-prime-example-of-dependency-injection)
  for more information). Its purpose is to add extra options to `pytest`'s CLI,
  setup logging, and define the fixtures for the `Workspace`, `Daemon`,
  and `Client` classes (see below).

- `conftest.py.in`: This template file allows adding CLI arguments to `pytest`
  regardless of whether it is run from `${CMAKE_CURRENT_SOURCE_DIR}`
  or `${CMAKE_BINARY_DIR}`. As mentioned, the `pytest` framework auto-discovers
  tests, *ini-files*, and `conftest.py` files and uses this information to
  determine the `rootdir` of the tests. Unfortunately, when the first ini-file
  is found, `pytest` considers its parent directory as the `rootdir` and expects
  to find the *main* `conftest.py` in the same directory. Thus, in order to
  successfully add extra arguments to `pytest`'s CLI *while keeping the tests
  sources in `${CMAKE_CURRENT_SOURCE_DIR}`*, we must have a valid `conftest.py`
  file in `${CMAKE_BINARY_DIR}`.

- `cli.py`: This module exports the function `add_cli_options()` that adds
  the `--interface`, `--bin-dir`, and `--lib-dir` CLI arguments to `pytest`.
  This file is the only python source file that is copied
  to `${CMAKE_BINARY_DIR}` so that it can be called
  from `${CMAKE_BINARY_DIR}/conftest.py`.

- `gkfs.io/`: This directory contains the sources for the `gkfs.io` helper
  program. This program acts as a proxy for the `Client` class described below
  to execute I/O-related system calls and library functions from a `LD_PRELOAD`
  context. Results from the function execution are returned in JSON format so
  that they can be easily parsed by the `Client` class (see `io` module below).

- `gkfs.py`: This module exports the `Daemon`, `Client`, and `ShellClient`
  classes which allow tests to interface easily with the GekkoFS daemon, client
  library, and shell, respectively (see below for more details).

- `io.py`: This module exports an `IOParser` class that is used internally by
  the `Client` class to deserialize any JSON output produced by the `gkfs.io`
  helper program. JSON deserialization relies
  on [marshmallow](https://marshmallow.readthedocs.io/en/stable/) fields and
  schemas to convert JSON strings to native Python datatypes.

- `cmd.py`: This module exports a `CommandParser` class that is used internally
  by `ShellClient` class to deserialize output strings generated by shell
  commands into native Python datatypes.

- `logger.py`: This module exports the `harness.logger` alias which hides the
  implementation details of the actual logging framework used. Any tests willing
  to produce logging messages only need to `import harness.logger` and call the
  appropriate `logger.LEVEL(msg)` function. All standard Python logging levels
  are supported.

- `workspace.py`: This module exports the `Workspace` class which allows tests
  to setup and interact with their workspace.

### Useful fixtures available to tests

The project's testing harness relies on `pytest` internal fixture mechanism to
setup and teardown resources for the test (e.g. daemon and client processes). By
leveraging `pytest`'s mechanism for automatic fixture injection, any test can
activate the harness' automatic resource management by simply declaring the use
of a harness fixture from test functions, modules, classes or whole projects.

In order to simplify writing tests and to ensure that they can run in parallel,
the harness currently provides the following fixtures, which are defined
in [`${GKFS_ROOTDIR}/tests/conftest.py`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/conftest.py):

#### workspace

The `workspace` fixture returns an instance of
the [`Worskpace`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/harness/workspace.py#L3)
class that implements a self-contained subdirectory where a test can run and
generate artifacts in a self-contained manner. Given a *test working
directory* `twd`, a workspace for a test is initialized by creating the
following directories under `twd`:

```shell
twd                                # base directory for test (typically under the system's temporary directory)
├── logs                           # directory for logs
│   ├── gkfs_client.log
│   └── gkfs_daemon.log
├── mnt                            # directory for GekkoFS' virtual mount point
├── root                           # directory for GekkoFS' internal data
│   └── 14935
│       ├── data
│       │   └── chunks
│       └── rocksdb
│           ├── 000003.log
│           ├── 000006.sst
│           ├── CURRENT
│           ├── IDENTITY
│           ├── LOCK
│           ├── LOG
│           ├── MANIFEST-000007
│           └── OPTIONS-000005
└── tmp                            # temporary directory for test
```

For convenience, the `Workspace` class can be passed a `bindirs` and `libdirs`
arguments that allow to respectively influence the `PATH` and `LD_LIBRARY_PATH`
environment variables that will be used when executing internal shell commands
under it.

***
**IMPORTANT**
*The workspace fixture is a direct dependency of the `gkfs_daemon`
and `gkfs_client` test fixtures described below, which means that it's
transitively added to any tests depending on these fixtures. There is seldom any
need to directly declare it or use it when writing a test.*
***

#### file_factory

The `file_factory` fixture returns an instance of
the [`harness.workspace.FileCreator`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/harness/workspace.py#L110)
class that allows test developers to create custom files with random binary
contents in the test workspace. To do so, the `FileCreator` class provides
a `create(pathname, size, unit)` function to generate the desired file, which in
turn returns a `workspace.File` object that represents the newly created file.
This `File` object offers convenience methods and properties to interact with
the created file in the context of a test (e.g. `md5sum()`):

```python
def test_foobar(gkfs_daemon, gkfs_shell, file_factory):
    # create a 4MB file and compute its md5sum 
    lf01 = file_factory.create(file01, size=4.0, unit='MB')
    digest = lf01.md5sum()  # digest => '7f45c62700402ce5f9abe5b8d70d2844'
```

#### gkfs_daemon

The `gkfs_daemon` fixture returns an instance of
the [`harness.gkfs.Daemon`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/daemon/tests/harness/gkfs.py#L87)
class that represents a local running daemon running in an isolated test
workspace. The `Daemon` class provides methods to control the daemon process
which are automatically invoked by `pytest` when a test starts/finishes to start
up/shut down the daemon required by the test.

The class relies on Python's [sh](https://amoffat.github.io/sh/) module to
configure, spawn and monitor a GekkoFS daemon process. The process is executed
independently of the test process and the class sends the appropriate `SIGTERM`
signal when the test finishes to properly shut down the daemon.

Currently, the daemon is executed with the following configuration:

- `LD_LIBRARY_PATH`: The `LD_LIBRARY_PATH` variable is set to the contents
  of `$LD_LIBRARY_PATH` when the test is invoked plus any additional paths
  defined in `libdirs`.
- `GKFS_HOSTS_FILE`: The `GKFS_HOSTS_FILE` variable is set
  to `<twd>/gkfs_hosts.txt`.
- `GKFS_DAEMON_LOG_PATH`: The `GKFS_DAEMON_LOG_PATH` variable is set
  to `<twd>/logs/gkfs_daemon.log`.
- `GKFS_LOG_LEVEL`: The `GKFS_LOG_LEVEL` variable is set to `100`.
- `--mountdir,-m`: The `--mountdir` CLI argument is set to `<twd>/mnt`.
- `--rootdir,-r`: The `--rootdir` CLI argument is set to `<twd>/root`.
- `--listen,-l`: The `--listen` CLI argument is set to an ephemeral network
  address (IPv4:port) generated from the interface defined provided by the
  test's `Workspace` (`lo` by default) and a randomly selected unused port in
  the range `[1024, 32768)`.

The `Daemon` class exposes the following properties from a running daemon to
tests:

- `cwd`: the daemon's current working directory.
- `rootdir`: the daemon's root directory in the host file system.
- `mountdir`: the daemon's mount directory in the host file system.
- `logdir`: the directory used by the daemon to store logs.
- `interface`: the daemon's address used for communication.

#### gkfs_client

The `gkfs_client` fixture returns an instance of
the [`harness.gkfs.Client`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/harness/gkfs.py#L220)
class. The `Client` class allows users to execute I/O-related functions from the
glibc (i.e. system calls and library functions) in their own separate processes
directly from Python code. The harness again relies on the `sh` module to spawn
the processes with an appropriately patched environment so that they can
communicate successfully with a running daemon.

To simplify usage and reduce coding overhead, the class provides a
special `__getattr__()` method that tries to transform any method called on
a `Client` instance to a `gkfs.io` command. Thus, the following code:

```python
gkfs_client.mkdir("/foobar/", stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
```

transforms into:

```console
$ gkfs.io mkdir /foobar/ 0777
{
  "errnum": 0,
  "retval": 0
}
```

As shown in the excerpt above, `gkfs.io` returns function call results as JSON
records through `stdout`, which are deserialized by the `IOParser`
in [`io.py`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/harness/io.py#L165)
and transformed into a Python's `namedtuple`. Thus, the previous function call
would return the following:

```ipython
In[1]: gkfs_client.mkdir("/foobar/", stat.S_IRWXU | stat.S_IRWXG | stat.S_IRWXO)
MkdirReturn(retval=0, errno=115)
```

```{eval-rst}
.. warning::
  Though providing a :code:`__getattr__()` method allows for more expressive 
  tests, it also means that **any function name** called from an instance of
  the :code:`Client class` automatically gets transformed into a 
  :code:`gkfs.io` argument. This may cause unexpected errors if the 
  corresponding subcommand has not yet been implemented in :code:`gkfs.io`
```

#### gkfs_shell

The `gkfs_shell` fixture returns an instance of
the [`harness.gkfs.ShellClient`](https://storage.bsc.es/gitlab/hpc/gekkofs/blob/master/tests/harness/gkfs.py#L346)
class. The `ShellClient` class allows users to execute shell commands and
scripts in their own separate processes directly from Python test code. The
harness again relies on the `sh` module to spawn the processes with an
appropriately patched environment so that they can communicate successfully with
a running daemon.

##### Single commands

To simplify usage and reduce coding overhead, the class provides a
special `__getattr__()` method that tries to transform any method called on
a `ShellClient` instance to a `bash -c` command. Thus, in the following code:

```python
def test_cp(gkfs_daemon, gkfs_shell, file_factory):
    """Copy a file into gkfs using the shell"""

    lf01 = file_factory.create(file01, size=4.0, unit='MB')

    # lf01.pathname: '${TWD}/tmp/file01'
    # gkfs_daemon.mountdir: '${TWD}/mnt/'
    cmd = gkfs_shell.cp(lf01.pathname, gkfs_daemon.mountdir)
    assert cmd.exit_code == 0
    assert cmd.stdout.decode() == ''
    assert cmd.stderr.decode() == ''
```

`gkfs_shell.cp(lf01.pathname, gkfs_daemon.mountdir)` transforms into:

```console
$ LD_LIBRARY_PATH=<XXX> LD_PRELOAD=<YYY> bash -c 'cp ${TWD}/tmp/file01 ${TWD}/mnt/'
```

where `XXX` and `YYY` are respectively substituted by the appropriate library
paths and `libgfs_intercept.so` library required for the test.

Similarly to the `sh` module, the raw output generated by a shell command
executed in isolation (e.g. `cp`, `stat`, `md5sum`, etc.) can be accessed using
the `stdout` and `stderr` properties as exemplified in the code above. Note,
however, that the output for such commands can be accessed more conveniently as
a Python object using the `parsed_stdout` property, provided that an
appropriate `CommandParser` has been implemented for the command:

```ipython 
In[1]: gkfs_shell.stat("--terse /tmp/foobar")
statOutput(filename='/tmp/foobar', size=4000000, blocks=0, raw_mode='81b4', uid=1000, gid=1000, device='0', inode=10075480095715127217, hard_links=1, major='0', minor='0', last_access=0, last_modification=0, last_status_change=0, creation=0, transfer_size=524288)
```

If no command parser is available for the command, a `NotImplementedError` will
be raised by the `harness.cmd.CommandParser` class.

##### Complex scripts

For convenience, the `ShellClient` class also provides a `script()` method that
allows to execute complex chains of commands:

```python
def test_shell_if_e(gkfs_daemon, gkfs_shell, file_factory):
    """
    Copy a file into gkfs using the shell and check that it exists using `if [[ -e <file> ]]`.
    """

    logger.debug("creating input file")
    lf01 = file_factory.create(file01, size=4.0, unit='MB')

    logger.debug("copying into gkfs")
    cmd = gkfs_shell.cp(lf01.pathname, gkfs_daemon.mountdir)
    assert cmd.exit_code == 0

    logger.debug("checking if file exists")
    cmd = gkfs_shell.script(
        f"""
            expected_pathname={gkfs_daemon.mountdir / file01}
            if [[ -e ${{expected_pathname}} ]];
            then
                exit 0
            fi
            exit 1
        """)

    assert cmd.exit_code == 0
```

Thanks to Python 3's [f-strings](https://realpython.com/python-f-strings/), it
is really simple to expand Python expressions in the shell script, though care
must be taken to escape braces when referencing shell variables in the script
code (e.g. `${{expected_pathname}}` in the excerpts above).

Note that by default, `gkfs_shell.script()` will patch the `LD_LIBRARY_PATH`
and `LD_PRELOAD` of the shell executing the script so that it gets intercepted
by the GekkoFS client library. If such intrusive interception is not desired, (
because a test may require a single command of a complex script to be
intercepted), it is possible to disable it by setting the `intercept_shell`
argument to `False` as shown in the code below:

```python
def test_shell_stat_script(gkfs_daemon, gkfs_shell, file_factory):
    """
    Copy a file into gkfs using the shell and check that `stat <file> succeeds`
    """

    logger.debug("creating input file")
    lf01 = file_factory.create(file01, size=4.0, unit='MB')

    logger.debug("copying into gkfs")
    cmd = gkfs_shell.cp(lf01.pathname, gkfs_daemon.mountdir)
    assert cmd.exit_code == 0

    logger.debug("checking metadata")
    cmd = gkfs_shell.script(
        f"""
            expected_pathname={gkfs_daemon.mountdir / file01}
            {gkfs_shell.patched_environ} stat ${{expected_pathname}}
            exit $?
        """,
        intercept_shell=False)

    assert cmd.exit_code == 0
```

As demonstrated by the code above, the `gkfs_shell` class provides
the `patched_environ` property to make it simple to provide the appropriate
environment variables to any commands that should be intercepted. Thus, the
above line:

```python
f"""
    expected_pathname={gkfs_daemon.mountdir / file01}
    {gkfs_shell.patched_environ} stat ${{expected_pathname}}
    exit $?
""",
``` 

will be transformed by the harness into:

```shell
LD_LIBRARY_PATH="/opt/gekkofs/build/tests:/opt/gekkofs/prefix/lib" \
LD_PRELOAD="/opt/gekkofs/build/src/client/libgkfs_intercept.so" \
LIBGKFS_HOSTS_FILE="/tmp/pytest-of-user/pytest-45/test_stat_script0/gkfs_hosts.txt" \
LIBGKFS_LOG="all" LIBGKFS_LOG_OUTPUT="/tmp/pytest-of-user/pytest-45/test_stat_script0/logs/gkfs_client.log" \
    stat ${expected_pathname}
```

```{eval-rst}
.. warning::
  Please note that, unlike for single shell commands, the :code:`parsed_stdout`
  property is not available for complex shell scripts run
  using :code:`gkfs_shell.script()`. The reason is that it is not possible to 
  provide a generic parser for all possible scripts. The raw output generated 
  by a script, if any, can however be accessed using the :code:`stdout` and 
  :code:`stderr` properties.
```

Please note that, unlike for single shell commands, the `parsed_stdout`
property is not available for complex shell scripts run
using `gkfs_shell.script()`. The reason is that it is not possible to provide a
generic parser for all possible scripts. The raw output generated by a script,
if any, can however be accessed using the `stdout` and `stderr` properties.

## Unit tests

TBD