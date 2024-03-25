#ifndef TAGGER_HPP
#define TAGGER_HPP

#include <AMReX.H>
#include <AMReX_Array4.H>

class Tagger {

private:
  AMREX_FORCE_INLINE
  bool is_not_homogeneous(amrex::Box const &f_bx,
                          amrex::Array4<const amrex::Real> const &f_a) {

    const auto f_lo = amrex::lbound(f_bx);
    const auto f_hi = amrex::ubound(f_bx);

    auto val = f_a(f_lo.x, f_lo.y, f_lo.z);

    for (int k = f_lo.z; k <= f_hi.z; ++k) {
      for (int j = f_lo.y; j <= f_hi.y; ++j) {
        for (int i = f_lo.x; i <= f_hi.x; ++i) {
          if (f_a(i, j, k) != val)
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
  void cell_marker(amrex::Box const &bx, amrex::Box const &f_bx,
                   amrex::Array4<char> const &tag,
                   amrex::Array4<const amrex::Real> const &a,
                   amrex::Array4<const amrex::Real> const &f_a, char tagval) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    if (is_diagonal(f_bx, f_a))
      for (int k = lo.z; k <= hi.z; ++k) {
        for (int j = lo.y; j <= hi.y; ++j) {
          for (int i = lo.x; i <= hi.x; ++i) {
            tag(i, j, k) = tagval;
          }
        }
      }
  }

  void cell_marker_test(amrex::Box const &bx, amrex::Array4<char> const &tag,
                        amrex::Array4<const amrex::Real> const &a,
                        char tagval) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          if (i == j) {
            tag(i, j, k) = tagval;
          }
        }
      }
    }
  }
};
#endif
