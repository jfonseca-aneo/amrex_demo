#ifndef TAGGER_HPP
#define TAGGER_HPP

#include <AMReX.H>
#include <AMReX_Array4.H>
#include <TiffHolder.hpp>

class Tagger {

private:
  bool is_not_homogeneous(amrex::Box const &fine_bx, TiffHolder &ref_image) {

    const auto lo = amrex::lbound(fine_bx);
    const auto hi = amrex::ubound(fine_bx);

    auto val = ref_image.imageVectors[lo.z][lo.y * ref_image.width + lo.x];
    for (int k = lo.z; k <= hi.z; ++k) {
      auto img = ref_image.imageVectors[k];
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          if (img[j * ref_image.width + i] != val)
            return true;
        }
      }
    }

    return false;
  }

  bool is_diagonal(amrex::Box const &f_bx,
                   amrex::Array4<const amrex::Real> const &f_a) {

    const auto f_lo = amrex::lbound(f_bx);
    const auto f_hi = amrex::ubound(f_bx);

    if (f_lo.y == f_lo.x)
      return true;

    return false;
  }

public:
  void cell_marker(amrex::Box const &bx, amrex::Box const &fine_bx,
                   amrex::Array4<char> const &tag, TiffHolder &finnest_tiff,
                   char tagval) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    if (is_not_homogeneous(fine_bx, finnest_tiff)) {
      for (int k = lo.z; k <= hi.z; ++k) {
        for (int j = lo.y; j <= hi.y; ++j) {
          for (int i = lo.x; i <= hi.x; ++i) {
            tag(i, j, k) = tagval;
          }
        }
      }
    }
  }
};
#endif
