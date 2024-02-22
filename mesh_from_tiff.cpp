#include <AMReX.H>
#include <AMReX_Geometry.H>
#include <AMReX_MultiFab.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>
#include <AMReX_Print.H>
#include <iostream>
#include <stdlib.h>
#include <tiffio.h>
#include <time.h>

using namespace amrex;

class TiffHolder {
private:
  TIFF *tiff_to_track = nullptr;

  Vector<MultiFab> phi_new;
  Vector<MultiFab> phi_old;

public:
  uint32_t layers;
  uint32_t width;
  uint32_t height;

  TiffHolder(std::string filePath) {

    const char *pathAsCString = filePath.c_str();

    tiff_to_track = TIFFOpen(pathAsCString, "r");

    if (tiff_to_track) {
      layers = TIFFNumberOfDirectories(tiff_to_track);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGEWIDTH, &width);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGELENGTH, &height);
    }
  }

  ~TiffHolder() { TIFFClose(tiff_to_track); }

  void printTiffInfo() {
    Print() << "Image dimension information:\n";
    Print() << "Dim X: " << width << "\n";
    Print() << "Dim Y: " << height << "\n";
    Print() << "Dim Z: " << layers << "\n";
  }

  void setPixels(Box const &bx, Array4<Real> const &a) {

    size_t imageSize = width * height;
    uint32_t *imageData = new uint32_t[imageSize];

    const auto lo = lbound(bx);
    const auto hi = ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      TIFFSetDirectory(tiff_to_track, k);
      TIFFReadRGBAImage(tiff_to_track, width, height, imageData);
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          a(i, j, k) = TIFFGetR(imageData[j * width + i]);
        }
      }
    }
  }
};

int main(int argc, char *argv[]) {

  amrex::Initialize(argc, argv);
  {
    ParmParse pp;

    std::string filePath;
    pp.query("tiffPath", filePath);

    TiffHolder simul_tif(filePath);

    simul_tif.printTiffInfo();

    BoxArray box_array;
    Geometry geometry;

    // AMREX_D_DECL means "do the first X of these, where X is the
    // dimensionality of the simulation"
    IntVect dom_lo(AMREX_D_DECL(0, 0, 0));
    IntVect dom_hi(AMREX_D_DECL(simul_tif.width - 1, simul_tif.height - 1,
                                simul_tif.layers - 1));

    // Make a single box that is the entire domain
    Box domain(dom_lo, dom_hi);

    // Initialize the boxarray from the single box "domain"
    box_array.define(domain);
    Print() << "BoxArray size is " << box_array.size() << "\n";

    // Chop single box in chunks of size 32
    box_array.maxSize(32);
    Print() << "BoxArray size after partition is " << box_array.size() << "\n";

    // Distribute boxes among MPI ranks
    DistributionMapping dm(box_array);

    // This defines the physical box, [0,1] in each direction.
    RealBox real_box({AMREX_D_DECL(0.0, 0.0, 0.0)},
                     {AMREX_D_DECL(1.0, 1.0, 1.0)});

    // This sets the boundary conditions to be doubly or triply periodic
    Array<int, AMREX_SPACEDIM> is_periodic{AMREX_D_DECL(0, 0, 0)};

    // This says we are using Cartesian coordinates
    int coord = 0;

    // This defines a Geometry object
    geometry.define(domain, real_box, coord, is_periodic);

    // Create a MultiFab to store data on the grids
    int ncomp = 1; // Number of components
    int ngrow = 0; // Number of ghost cells
    MultiFab multi_fab(box_array, dm, ncomp, ngrow);

    multi_fab.setVal(0);

    for (MFIter mfi(multi_fab); mfi.isValid(); ++mfi) // Loop over grids
    {
      // This is the valid Box of the current FArrayBox.
      // By "valid", we mean the original ungrown Box in BoxArray.
      const Box &box = mfi.validbox();

      // A reference to the current FArrayBox in this loop iteration.
      FArrayBox &fab = multi_fab[mfi];

      // Obtain Array4 from FArrayBox.  We can also do
      //     Array4<Real> const& a = mf.array(mfi);
      Array4<Real> const &a = fab.array();

      simul_tif.setPixels(box, a);
    }

    const std::string &plotfilename = "amrex_mesh";
    WriteSingleLevelPlotfile(plotfilename, multi_fab, {"data"}, geometry, 0.,
                             0.);
  }
  amrex::Finalize();

  return 0;
}
