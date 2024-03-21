#include <AMReX.H>
#include <AmrMeshFromTiff.hpp>

using namespace amrex;

int main(int argc, char *argv[]) {

  amrex::Initialize(argc, argv);
  {
    AmrMeshFromTiff amr_mesh;

    amr_mesh.InitData();
  }
  amrex::Finalize();

  return 0;
}