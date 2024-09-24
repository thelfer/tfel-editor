# Prerequisites

- `cmake` development packages.
- [`qt6` development packages]().
    - On debian system, one shall install `libqt6svg5-dev`
- hunspell (optional). On debian system, one shall install
- `TFEL/MFront` project
- `tfel-plot` project (optional)
- a decent C++-11 compiler. 

### Ubuntu

The following packages are needed to get all the features of
`TFEL/MFront`, `tfel-plot` and `tfel-editor`:

~~~~{.bash}
$ sudo apt-get install git cmake g++ gfortran libboost-all-dev
$ sudo apt-get install libqt6svg6-dev
~~~~

# Build instructions

The `build` system used by `tfel-editor` is
[`cmake`](https://cmake.org/). This paragraph covers the usage of
`cmake` from the command line.

It is divided in three steps:

- the configure step
- the build step
- the installation step

## Configuration step

### Useful `cmake` options

The following variable affects the build of the editor:

- `CMAKE_BUILD_TYPE`: type of build
- `CMAKE_INSTALL_PREFIX`: path where `tfel-editor` will be installed
- `Qt6_DIR`: path to where `Qt6` is installed (optional if `Qt` is
  installed system-wide or if appropriate environment variables, such as
  `PATH` and `LD_LIBRARY_PATH` has been set).
- `enable-tfel-plot`: boolean value enabling/disabling the support of
  `tfel-plot` (`OFF` by default)
- `TFELHOME`: path where `tfel` is installed (optional if `tfel-config`
  is in the current executable path)
- `TFELPLOTHOME`: path where `tfel-plot-config` is installed (optional
  if `tfel-config` is in the current executable path)

Note that you must take care that the versions of `tfel` and `tfel-plot`
are compatible with the version of `tfel-editor` that you want to build.
By conventions, all `master` branches are compatible.

The `CMAKE_BUILD_TYPE` has no meaning when building against `Visual Studio`.

### Typical `cmake` call

Assuming that `tfel-config` and `tfel-plot-config` are in the current
executable path, a typical call to cmake is as follows:

~~~~
mkdir -p build
pushd build
cmake <PATH_TO_TFEL_EDITOR_SOURCES> -DCMAKE_BUILD_TYPE=Release -Denable-tfel-plot=ON -DCMAKE_INSTALL_PREFIX=<PATH_TO_TFEL_EDITOR_INSTALLATION_DIRECTOR>
~~~~

Here you must replace `<PATH_TO_TFEL_EDITOR_SOURCES>` and
`<PATH_TO_TFEL_EDITOR_INSTALLATION_DIRECTOR>` by the appropriate paths.

## Build step

### Using `Visual Studio`

In the `build` directory

~~~~
cmake --build . --config=Release --target=all
~~~~

### Under `UniX`

~~~~
make
~~~~

## Installation step

### Using `Visual Studio`

In the `build` directory

~~~~
cmake --build . --config=Release --target=install
~~~~

### Under `UniX`

~~~~
make install
~~~~
