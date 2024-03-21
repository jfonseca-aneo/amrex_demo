#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>
#include <AMReX_Print.H>

#include <AMReX_ParallelDescriptor.H>
#include <AmrMeshFromTiff.hpp>
#include <Tagger.hpp>
#include <TiffHolder.hpp>

using namespace amrex;

class NotImplemented : public std::logic_error {
public:
  NotImplemented() : std::logic_error("Function not yet implemented"){};
};

AmrMeshFromTiff::AmrMeshFromTiff() {
  ReadParameters();
  int nlevs_max = max_level + 1;
  pixel_data.resize(nlevs_max);
}

AmrMeshFromTiff::~AmrMeshFromTiff() {}

void AmrMeshFromTiff::MakeNewLevelFromCoarse(int lev, Real time,
                                             const BoxArray &ba,
                                             const DistributionMapping &dm) {
  throw NotImplemented();
}

void AmrMeshFromTiff::RemakeLevel(int lev, Real time, const BoxArray &ba,
                                  const DistributionMapping &dm) {

  throw NotImplemented();
}

void AmrMeshFromTiff::ClearLevel(int lev) { throw NotImplemented(); }

void AmrMeshFromTiff::MakeNewLevelFromScratch(int lev, Real time,
                                              const BoxArray &ba,
                                              const DistributionMapping &dm) {

  const int ncomp = 1;
  const int nghost = 1;

  pixel_data[lev].define(ba, dm, ncomp, nghost);

  TiffHolder this_level_tiff(tiff_paths[lev]);

  this_level_tiff.printTiffInfo();

  MultiFab &state = pixel_data[lev];

  const auto problo = Geom(lev).ProbLoArray();
  const auto dx = Geom(lev).CellSizeArray();

  amrex::Print() << "MakeNewFromScratch: init data for level " << lev << "\n";
  for (MFIter mfi(state); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    auto const &a = state.array(mfi);
    this_level_tiff.setPixels(box, a);
    // this_level_tiff.setPixelsDiag(box, a);
    // this_level_tiff.setPixelsTest(box, problo, dx, a);
  }

  // Update ghost cells at the innerboundary of same level patches
  // TODO: inplement FillFromCoarse to fill ghost cells at the boundary of two
  // levels
  state.FillBoundary();
}

void AmrMeshFromTiff::ErrorEst(int lev, TagBoxArray &tags, Real,
                               int ngrow = 1) {

  const int tagval = TagBox::SET;

  const MultiFab &state = pixel_data[lev];

  amrex::Print() << "ERROREST for level " << lev << "\n";

  for (MFIter mfi(state); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    const auto &state_arr = state.array(mfi);
    const auto &tag_arr = tags.array(mfi);
    tagger.cell_marker_test(box, tag_arr, state_arr, tagval);
  }
}

void AmrMeshFromTiff::ReadParameters() {
  ParmParse pp;
  std::string path;

  if (auto nt = pp.countval("tiffPath")) {
    pp.getarr("tiffPath", tiff_paths, 0, nt);
  }
  for (auto p : tiff_paths)
    Print() << p << "\n";

  int max_levels;
  pp.get("amr.max_level", max_levels);
  istep.resize(max_levels + 1, 0);
  pp.get("material_name", material_name);
}

void AmrMeshFromTiff::InitData() {
  const Real time = 0.0;
  InitFromScratch(time);
  if (ParallelDescriptor::MyProc() == 0)
    printGridSummary(amrex::OutStream(), 0, max_level);
  WritePlotFile();
}

Vector<const MultiFab *> AmrMeshFromTiff::PlotFileMF() const {
  Vector<const MultiFab *> r;
  for (int lev = 0; lev <= finest_level; ++lev) {
    r.push_back(&pixel_data[lev]);
  }
  return r;
}

std::string AmrMeshFromTiff::BuildPlotFileName() const {
  std::string plotName = "";
  plotName.append(plot_file);
  plotName.append("_");
  plotName.append(material_name);
  return plotName;
}

void AmrMeshFromTiff::WritePlotFile() const {
  const std::string &plotfilename = BuildPlotFileName();
  const auto &mf = PlotFileMF();

  amrex::Print() << "Writing plotfile " << plotfilename << "\n";

  amrex::WriteMultiLevelPlotfile(plotfilename, finest_level + 1, mf,
                                 {material_name}, Geom(), 0.0, istep,
                                 refRatio());
}