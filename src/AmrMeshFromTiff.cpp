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

AmrMeshFromTiff::AmrMeshFromTiff() { ReadParameters(); }

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

  static bool first_run = true;
  const int ncomp = 1;
  const int nghost = 1;

  /* Remake finnest level with the current box array and distribution mapping */
  amrex::Print() << "MakeNewFromScratch: Processing finnest level " << lev
                 << "\n";

  pixel_data[max_level].clear();

  BoxArray finner_ba = ba;
  int max_ref = 1 << (max_level - lev);
  finner_ba.refine(max_ref);

  pixel_data[max_level].define(finner_ba, dm, ncomp, nghost);

  TiffHolder finnest_level_tiff(tiff_paths.back());

  MultiFab &fstate = pixel_data[max_level];
  for (MFIter mfi(fstate); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    auto const &a = fstate.array(mfi);
    finnest_level_tiff.setPixels(box, a);
  }

  if (first_run) {
    first_run = false;
    WriteSingleLevelPlotfile({"finner_level"}, fstate, {material_name},
                             Geom(max_level), 0., 0.);
  }

  amrex::Print() << "MakeNewFromScratch: init data for level " << lev << "\n";

  pixel_data[lev].define(ba, dm, ncomp, nghost);

  TiffHolder this_level_tiff(tiff_paths[lev]);
  this_level_tiff.printTiffInfo();

  MultiFab &state = pixel_data[lev];
  // const auto problo = Geom(lev).ProbLoArray();
  // const auto dx = Geom(lev).CellSizeArray();
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
  const MultiFab &fine_state = pixel_data[max_level];

  amrex::Print() << "ERROREST for level " << lev << "\n";

  for (MFIter mfi(state); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    Box b = box;
    int ref = 1 << (max_level - lev);
    b.refine(ref);
    const auto &state_arr = state.array(mfi);
    const auto &fine_state_arr = fine_state.array(mfi);
    const auto &tag_arr = tags.array(mfi);
    tagger.cell_marker(box, b, tag_arr, state_arr, fine_state_arr, tagval);
    // tagger.cell_marker_test(box, tag_arr, state_arr, tagval);
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

  /* pixel_data[max_lev + 1] will contain the finnest grid */
  pixel_data.resize(max_levels + 1);
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