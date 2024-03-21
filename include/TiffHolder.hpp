#ifndef TiffHolder_HPP_
#define TiffHolder_HPP_

#include <iostream>
#include <stdlib.h>
#include <tiffio.h>
#include <time.h>

#include <AMReX.H>
#include <AMReX_Print.H>

class TiffHolder {

public:
  TIFF *tiff_to_track;
  uint32_t layers;
  uint32_t width;
  uint32_t height;
  std::vector<std::vector<uint32_t>> imageVectors;

  TiffHolder(std::string filePath) {

    const char *pathAsCString = filePath.c_str();

    tiff_to_track = TIFFOpen(pathAsCString, "r");

    if (tiff_to_track) {
      layers = TIFFNumberOfDirectories(tiff_to_track);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGEWIDTH, &width);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGELENGTH, &height);
      for (int l = 0; l < layers; l++) {

        TIFFSetDirectory(tiff_to_track, l);

        // Allocate memory for current image layer
        std::vector<uint32_t> raster(width * height);

        // Read the image
        if (TIFFReadRGBAImage(tiff_to_track, width, height, raster.data())) {
          // Add the image data to the vector
          imageVectors.push_back(std::move(raster));
        }
      }

      TIFFClose(tiff_to_track);
    }
  }

  ~TiffHolder() {}

  void printTiffInfo() {
    amrex::Print() << "Image dimension information:\n";
    amrex::Print() << "Dim X: " << width << "\n";
    amrex::Print() << "Dim Y: " << height << "\n";
    amrex::Print() << "Dim Z: " << layers << "\n";
  }

  void setPixels(amrex::Box const &bx, amrex::Array4<amrex::Real> const &a) {

    size_t imageSize = width * height;
    uint32_t *imageData = new uint32_t[imageSize];

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      auto img = imageVectors[k];
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          a(i, j, k) = static_cast<amrex::Real>(TIFFGetR(img[j * width + i]));
        }
      }
    }
  }

  void setPixelsDiag(amrex::Box const &bx,
                     amrex::Array4<amrex::Real> const &a) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          if (i == j)
            a(i, j, k) = 1;
          else
            a(i, j, k) = 0;
        }
      }
    }
  }

  void
  setPixelsTest(amrex::Box const &bx,
                amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> const &prob_lo,
                amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> const &dx,
                amrex::Array4<amrex::Real> const &a) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      for (int j = lo.y; j <= hi.y; ++j) {
        amrex::Real y = prob_lo[1] + (0.5 + j) * dx[1];
        for (int i = lo.x; i <= hi.x; ++i) {
          amrex::Real x = prob_lo[0] + (0.5 + i) * dx[0];
          amrex::Real r2 =
              (std::pow(x - 0.5, 2) + std::pow((y - 0.75), 2)) / 0.01;
          a(i, j, k) = 1.0 + std::exp(-r2);
        }
      }
    }
  }
};

#endif