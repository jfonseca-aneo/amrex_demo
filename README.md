# About this repository

Demo code to show how to use the AMR software library
[amrex](https://github.com/AMReX-Codes/amrex) to create a
mesh from a `tiff` image. The motivation is that the image
corresponds to a certain material composed of various phases.
In addition, the image defines a fine uniform mesh in order to
depict all these phases and the goal is to coarsen that mesh
in the regions where the phases do not change and keep it
fine between phase transitions / boundaries.

Unlike a single-image approach, this demo feeds AMReX one `tiff` stack
per refinement level (coarsest to finest); cells are tagged for
refinement wherever the corresponding block of the finest image is not
homogeneous, so the resulting mesh stays coarse inside each phase and
refines down to the finest level only at phase boundaries.

## Compilation

### Prerequisites

- [amrex](https://github.com/AMReX-Codes/amrex), built and installed with
  `-DAMReX_PIC=ON` (this demo links AMReX into a shared library) and
  `-DAMReX_MPI=YES`
- [libTIFF](http://www.libtiff.org/)
- an MPI implementation (e.g. OpenMPI or MPICH)

Assuming all three are installed, and that AMReX's install prefix is on
`CMAKE_PREFIX_PATH` (so `find_package(AMReX)` can locate its
`AMReXConfig.cmake`), compilation follows the standard `cmake` workflow:

```bash

mkdir /path/to/build_dir
cd /path/to/build_dir
cmake -S /path/to/this/repository -DCMAKE_PREFIX_PATH=/path/to/amrex/install/dir
make

```

## Usage

Successful compilation yields the executable `demo_exe/main` under the
build directory, which takes as argument an input file describing the
problem, e.g. [inputFile_anode](./test/inputFile_anode) or
[inputFile_cathode](./test/inputFile_cathode). Among other AMReX
parameters, that file lists the `tiffPath` of one image stack per AMR
level (coarsest first) and a `material_name` used to name the output
plotfile:

```bash

mpirun -np <nprocs> ./demo_exe/main inputFile_anode

```

Sample synthetic input images (a coarse anode and cathode microstructure,
each provided at 5 resolutions from 640nm down to 40nm per pixel) are
provided under [`test/image_anode`](./test/image_anode) and
[`test/image_cathode`](./test/image_cathode); they are copied next to the
executable at build time, together with the two sample input files.

Successful runs write a multi-level plotfile (e.g. `amrex_demo_anode`)
that can be inspected with tools such as
[VisIt](https://visit-dav.github.io/visit-website/) or
[yt](https://yt-project.org/).
