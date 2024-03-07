# About this repository

Demo code to show how to use the AMR software library
[amrex](https://github.com/AMReX-Codes/amrex) to create a
mesh from a `tiff` image. The motivation is that the image
corresponds to a certain material composed of various phases.
In addition, the image defines a fine uniform mesh in order to
depict all these phases and the goal is to coarsen that mesh
in the regions where the phases do not change and keep it
fine between phase transitions / boundaries.

## Compilation

### Prerequisites

- [amrex](https://github.com/AMReX-Codes/amrex)
- [libTIFF](http://www.libtiff.org/)

Assuming that both libraries are in your `$PATH`, compilation follows the standard `cmake` workflow

```bash

mkdir /path/to/build_dir
cd /path/to/build_dir
cmake -S /path/to/this/repository
make

```

## Usage

Successful compilation yields the  executable `/build/AMREX_DEMO` which takes
as argument the file '[inputFile](./test/inputFile)' that contains the path to the desired `.tif`
image to build the mesh from.
