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

  ParmParse pp;
  std::string path;

  if (auto nt = pp.countval("tiffPath")) {
    pp.getarr("tiffPath", tiff_paths, 0, nt);
  }
  for (auto p : tiff_paths)
    Print() << p << "\n";

  int max_levels;
  pp.get("amr.max_level", max_levels);

  num_levels = max_levels + 1;

  if (num_levels != pp.countval("tiffPath")) {
    throw std::runtime_error(
        "Max level must be equal to the number of tif files provided ");
  }

  istep.resize(num_levels, 0);
  pp.get("material_name", material_name);

  /* pixel_data[num_levels] will contain the finnest grid */
  pixel_data.resize(num_levels);

  for (auto p : tiff_paths) {
    TiffHolder tiff_holder(p);
    tiff_holder.printTiffInfo();
    tiff_images.push_back(std::move(tiff_holder));
  }
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
  const int nghost = 0;

  static bool first_run = true;
  if (first_run) {

    BoxArray finner_ba = ba;
    int max_ref = 1 << max_level;
    finner_ba.refine(max_ref);

    pixel_data[max_level].define(finner_ba, dm, ncomp, nghost);

    MultiFab &fstate = pixel_data[max_level];
    for (MFIter mfi(fstate); mfi.isValid(); ++mfi) {
      const Box &box = mfi.validbox();
      auto const &a = fstate.array(mfi);
      tiff_images[max_level].setPixels(box, a);
    }

    const std::string &plotfilename = BuildPlotFileName("finnest_level");
    WriteSingleLevelPlotfile(plotfilename, fstate, {material_name},
                             Geom(max_level), 0., 0.);

    first_run = false;

    pixel_data[max_level].clear();
  }

  amrex::Print() << "MakeNewFromScratch: init data for level " << lev << "\n";

  pixel_data[lev].define(ba, dm, ncomp, nghost);

  TiffHolder this_level_tiff = tiff_images[lev];

  MultiFab &state = pixel_data[lev];
  for (MFIter mfi(state); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    auto const &a = state.array(mfi);
    this_level_tiff.setPixels(box, a);
  }

  // Update ghost cells at the innerboundary of same level patches
  // TODO: inplement FillFromCoarse to fill ghost cells at the boundary of two
  // levels
  // state.FillBoundary();
}

void AmrMeshFromTiff::ErrorEst(int lev, TagBoxArray &tags, Real,
                               int ngrow = 0) {

  const int tagval = TagBox::SET;

  const MultiFab &state = pixel_data[lev];

  amrex::Print() << "ERROREST for level " << lev << "\n";

  for (MFIter mfi(state); mfi.isValid(); ++mfi) {
    const Box &box = mfi.validbox();
    const auto &tag_arr = tags.array(mfi);
    tagger.cell_marker(box, lev, max_level, tag_arr, tiff_images.back(),
                       tagval);
  }
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
  for (int lev = 0; lev < num_levels; ++lev) {
    r.push_back(&pixel_data[lev]);
  }
  return r;
}

std::string AmrMeshFromTiff::BuildPlotFileName(std::string base_name) const {
  std::string plotName = base_name;
  plotName.append("_");
  plotName.append(material_name);
  return plotName;
}

void AmrMeshFromTiff::WritePlotFile() const {
  const std::string &plotfilename = BuildPlotFileName(plot_file);
  const auto &mf = PlotFileMF();

  amrex::Print() << "Writing plotfile " << plotfilename << "\n";

  amrex::WriteMultiLevelPlotfile(plotfilename, num_levels, mf, {material_name},
                                 Geom(), 0.0, istep, refRatio());
}