#ifndef TiffHolder_HPP_
#define TiffHolder_HPP_

#include <iostream>
#include <stdlib.h>
#include <tiffio.h>
#include <time.h>

#include <AMReX.H>
#include <AMReX_Print.H>

class TiffHolder {

private:
  std::string tiff_path;

public:
  uint32_t layers;
  uint32_t width;
  uint32_t height;
  std::vector<std::vector<uint32_t>> imageVectors;

  TiffHolder(std::string filePath) {

    tiff_path = filePath;

    auto tiff_to_track = TIFFOpen(tiff_path.c_str(), "r");

    if (tiff_to_track) {
      layers = TIFFNumberOfDirectories(tiff_to_track);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGEWIDTH, &width);
      TIFFGetField(tiff_to_track, TIFFTAG_IMAGELENGTH, &height);

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

  void setImageVectors() {

    auto tiff_to_track = TIFFOpen(tiff_path.c_str(), "r");

    if (tiff_to_track) {

      imageVectors.reserve(layers);

      for (int l = 0; l < layers; l++) {

        TIFFSetDirectory(tiff_to_track, l);
        std::vector<uint32_t> raster(width * height);

        if (TIFFReadRGBAImage(tiff_to_track, width, height, raster.data())) {
          // Add current image layer to imageVectors (move = does not copy)
          imageVectors.push_back(std::move(raster));
        }
      }

      TIFFClose(tiff_to_track);
    }
  }

  void setPixels(amrex::Box const &bx, amrex::Array4<amrex::Real> const &a) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    auto tiff_to_track = TIFFOpen(tiff_path.c_str(), "r");

    if (tiff_to_track) {
      for (int k = lo.z; k <= hi.z; ++k) {
        auto img = TIFFSetDirectory(tiff_to_track, k);
        std::vector<uint32_t> raster(width * height);
        for (int j = lo.y; j <= hi.y; ++j) {
          for (int i = lo.x; i <= hi.x; ++i) {
            a(i, j, k) =
                static_cast<amrex::Real>(TIFFGetR(raster[j * width + i]));
          }
        }
      }
      TIFFClose(tiff_to_track);
    }
  }
};

#endif