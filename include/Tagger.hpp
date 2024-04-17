#ifndef TAGGER_HPP
#define TAGGER_HPP

#include <AMReX.H>
#include <AMReX_Array4.H>
#include <TiffHolder.hpp>

class Tagger {

private:
  void get_z_neighbors(int &i, int &j, int &k, TiffHolder &holder,
                       std::vector<uint32_t> &neighbors) {

    if (k - 1 > 0) {
      auto img_bottom = holder.imageVectors[k - 1];
      neighbors[0] = img_bottom[j * holder.width + i];
    }

    if (k + 1 < holder.layers) {
      auto img_up = holder.imageVectors[k + 1];
      neighbors[1] = img_up[j * holder.width + i];
    }
  }

  void get_y_neighbors(int &i, int &j, int &k, TiffHolder &holder,
                       std::vector<uint32_t> &neighbors) {

    auto img = holder.imageVectors[k];

    if (j - 1 > 0)
      neighbors[0] = img[(j - 1) * holder.width + i];

    if (j + 1 < holder.height)
      neighbors[1] = img[(j + 1) * holder.width + i];
  }

  void get_x_neighbors(int &i, int &j, int &k, TiffHolder &holder,
                       std::vector<uint32_t> &neighbors) {

    auto img = holder.imageVectors[k];

    if (i - 1 > 0)
      neighbors[0] = img[j * holder.width + i - 1];

    if (i + 1 < holder.width)
      neighbors[1] = img[j * holder.width + i + 1];
  }

  bool is_not_homogeneous(int &i, int &j, int &k, int &lev, int &max_lev,
                          TiffHolder &finest_image) {

    auto ref_factor = 1 << (max_lev - lev);

    auto lo_x = i * ref_factor;
    auto lo_y = j * ref_factor;
    auto lo_z = k * ref_factor;
    auto hi_x = (i + 1) * ref_factor;
    auto hi_y = (j + 1) * ref_factor;
    auto hi_z = (k + 1) * ref_factor;

    auto z_limits = {lo_z, hi_z - 1};
    auto y_limits = {lo_y, hi_y - 1};
    auto x_limits = {lo_x, hi_x - 1};

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

    if (lev < max_lev) {
      /* Check if z neighbors current patch hold different material*/
      for (auto kk : z_limits) {
        for (auto jj = lo_y; jj < hi_y; ++jj) {
          for (auto ii = lo_x; ii < hi_x; ++ii) {
            std::vector<uint32_t> neighbors(2, -1);
            get_z_neighbors(ii, jj, kk, finest_image, neighbors);
            for (auto &nn : neighbors) {
              if (val != nn && nn != -1)
                return true;
            }
          }
        }
      }

      /* Check if y neighbors current patch hold different material*/
      for (auto jj : y_limits) {
        for (int kk = lo_z; kk < hi_z; ++kk) {
          for (int ii = lo_x; ii < hi_x; ++ii) {
            std::vector<uint32_t> neighbors(2, -1);
            get_y_neighbors(ii, jj, kk, finest_image, neighbors);
            for (auto &nn : neighbors) {
              if (val != nn && nn != -1)
                return true;
            }
          }
        }
      }

      /* Check if x neighbors current patch hold different material*/
      for (auto ii : x_limits) {
        for (int kk = lo_z; kk < hi_z; ++kk) {
          for (int jj = lo_y; jj < hi_y; ++jj) {
            std::vector<uint32_t> neighbors(2, -1);
            get_x_neighbors(ii, jj, kk, finest_image, neighbors);
            for (auto &nn : neighbors) {
              if (val != nn && nn != -1)
                return true;
            }
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
          if (is_not_homogeneous(i, j, k, lev, max_lev, finest_tiff)) {
            // amrex::Print() << "Tagging all cells of box: " << bx << "\n";
            tag(i, j, k) = tagval;
          }
        }
      }
    }
  }
};
#endif
