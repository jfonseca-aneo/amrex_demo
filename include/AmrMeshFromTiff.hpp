#ifndef AmrMeshFromTiff_HPP_
#define AmrMeshFromTiff_HPP_

#include <AMReX_AmrCore.H>
#include <AMReX_PlotFileUtil.H>
#include <Tagger.hpp>
#include <TiffHolder.hpp>

class AmrMeshFromTiff : public amrex::AmrCore {
public:
  // constructor - reads in parameters from inputs file
  //             - sizes multilevel arrays and data structures
  AmrMeshFromTiff();
  virtual ~AmrMeshFromTiff();

  // initializes multilevel data
  void InitData();

  // Make a new level using provided BoxArray and DistributionMapping and
  // fill with interpolated coarse level data.
  // overrides the pure virtual function in AmrCore
  virtual void
  MakeNewLevelFromCoarse(int lev, amrex::Real time, const amrex::BoxArray &ba,
                         const amrex::DistributionMapping &dm) override;

  // Remake an existing level using provided BoxArray and DistributionMapping
  // and fill with existing fine and coarse data. overrides the pure virtual
  // function in AmrCore
  virtual void RemakeLevel(int lev, amrex::Real time, const amrex::BoxArray &ba,
                           const amrex::DistributionMapping &dm) override;

  // Delete level data
  // overrides the pure virtual function in AmrCore
  virtual void ClearLevel(int lev) override;

  // Make a new level from scratch using provided BoxArray and
  // DistributionMapping. Only used during initialization. overrides the pure
  // virtual function in AmrCore
  virtual void
  MakeNewLevelFromScratch(int lev, amrex::Real time, const amrex::BoxArray &ba,
                          const amrex::DistributionMapping &dm) override;

  // tag all cells for refinement
  // overrides the pure virtual function in AmrCore
  virtual void ErrorEst(int lev, amrex::TagBoxArray &tags, amrex::Real time,
                        int ngrow) override;

private:
  void ReadParameters();

  amrex::Vector<int> istep;

  amrex::Vector<TiffHolder> tiff_images;

  amrex::Vector<std::string> tiff_paths;

  amrex::Vector<amrex::MultiFab> pixel_data;

  Tagger tagger;

  std::string material_name{"anode"};

  std::string plot_file{"amrex_demo"};

  std::string BuildPlotFileName() const;

  amrex::Vector<const amrex::MultiFab *> PlotFileMF() const;

  void WritePlotFile() const;
};

#endif /*AmrMeshFromTiff_HPP_*/