#ifndef TAGGER_HPP
#define TAGGER_HPP

#include <AMReX.H>
#include <AMReX_Array4.H>

class Tagger {

private:
  AMREX_FORCE_INLINE
  bool check_isul(int i, int j, int k,
                  amrex::Array4<const amrex::Real> const &a) {
    auto tmp = a(i, j, k);
    if (a(i - 1, j, k) != tmp)
      return true;
    if (a(i + 1, j, k) != tmp)
      return true;
    if (a(i, j - 1, k) != tmp)
      return true;
    if (a(i, j + 1, k) != tmp)
      return true;
    if (a(i, j, k - 1) != tmp)
      return true;
    if (a(i, j, k + 1) != tmp)
      return true;

    return false;
  }

public:
  void cell_marker(amrex::Box const &bx, amrex::Array4<char> const &tag,
                   amrex::Array4<const amrex::Real> const &a, char tagval) {

    const auto lo = amrex::lbound(bx);
    const auto hi = amrex::ubound(bx);

    for (int k = lo.z; k <= hi.z; ++k) {
      for (int j = lo.y; j <= hi.y; ++j) {
        for (int i = lo.x; i <= hi.x; ++i) {
          if (!check_isul(i, j, k, a)) {
            tag(i, j, k) = tagval;
          }
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
