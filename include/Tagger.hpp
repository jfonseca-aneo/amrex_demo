#ifndef TAGGER_HPP
#define TAGGER_HPP

#include <AMReX.H>
#include <AMReX_Array4.H>
#include <TiffHolder.hpp>

class Tagger {

private:
  void get_pixel_neighbors(int i, int j, int k, TiffHolder &holder,
                           std::vector<uint32_t> &neighbors) {

    auto img = holder.imageVectors[k];

    for (int f = 0; f < 6; f++)
      neighbors[f] = -1;

    if (i - 1 > 0)
      neighbors[0] = img[j * holder.width + i - 1];

    if (i + 1 < holder.width)
      neighbors[1] = img[j * holder.width + i + 1];

    if (j - 1 > 0)
      neighbors[2] = img[(j - 1) * holder.width + i];

    if (j + 1 < holder.height)
      neighbors[3] = img[(j + 1) * holder.width + i];

    if (k - 1 > 0) {
      auto img_bottom = holder.imageVectors[k - 1];
      neighbors[4] = img_bottom[j * holder.width + i];
    }

    if (k + 1 < holder.layers) {
      auto img_up = holder.imageVectors[k + 1];
      neighbors[5] = img_up[j * holder.width + i];
    }
  }

  bool is_not_homogeneous(int i, int j, int k, int ref_factor,
                          TiffHolder &finest_image) {

    const auto lo_x = i * ref_factor;
    const auto lo_y = j * ref_factor;
    const auto lo_z = k * ref_factor;

    const auto hi_x = (i + 1) * ref_factor;
    const auto hi_y = (j + 1) * ref_factor;
    const auto hi_z = (k + 1) * ref_factor;

    auto val =
        finest_image.imageVectors[lo_z][lo_y * finest_image.width + lo_x];

    for (int kk = lo_z; kk < hi_z; ++kk) {
      auto img = finest_image.imageVectors[kk];
      for (int jj = lo_y; jj < hi_y; ++jj) {
        for (int ii = lo_x; ii < hi_x; ++ii) {
          if (img[jj * finest_image.width + ii] != val)
            return true;
        }
      }
    }

    for (int kk = lo_z; kk < hi_z; ++kk) {
      for (int jj = lo_y; jj < hi_y; ++jj) {
        for (int ii = lo_x; ii < hi_x; ++ii) {
          std::vector<uint32_t> neighbors(6, -1);
          get_pixel_neighbors(ii, jj, kk, finest_image, neighbors);
          for (int t = 0; t < 6; t++) {
            if (val != neighbors[t] && neighbors[t] != -1)
              return true;
          }
        }
      }
    }

    return false;
  }

public:
  void cell_marker(amrex::Box const &bx, int lev, int max_lev,
                   amrex::Array4<char> const &tag, TiffHolder &finest_tiff,
                   char tagval) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          auto r = 1 << (max_lev - lev);
          if (is_not_homogeneous(i, j, k, r, finest_tiff)) {
            // amrex::Print() << "Tagging all cells of box: " << bx << "\n";
            tag(i, j, k) = tagval;
          }
        }
      }
    }
  }
};
#endif
