/**
 *  \file cn_docking.h
 *  \brief Generate candidate cyclic (Cn) assemblies of a monomer.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 */

#ifndef IMPCNMULTIFIT_INTERNAL_CN_DOCKING_H
#define IMPCNMULTIFIT_INTERNAL_CN_DOCKING_H

#include <IMP/cnmultifit/cnmultifit_config.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/algebra/Transformation3D.h>
#include <utility>
#include <vector>

IMPCNMULTIFIT_BEGIN_INTERNAL_NAMESPACE

//! Parameters for dock_cn()
struct CnDockingParameters {
  //! Spacing (A) of the grid used for the FFT search; if <= 0, pick
  //! automatically based on the monomer size
  double coarse_spacing;
  //! Spacing (A) of the grid used to score candidates
  double fine_spacing;
  //! Angular spacing (degrees) of sampled symmetry axis directions;
  //! if <= 0, pick automatically based on the monomer size
  double angle_step;
  //! Number of peaks kept for each sampled axis direction
  unsigned int peaks_per_direction;
  //! Number of FFT candidates rescored on the fine grid
  unsigned int n_rescore;
  //! Number of top rescored candidates that are locally refined
  unsigned int n_refine;
  //! Maximum number of function evaluations for each refinement
  unsigned int refine_max_evaluations;
  //! RMSD (A) of the placed neighbor used to cluster solutions
  double cluster_rmsd;
  //! Maximum number of solutions returned
  unsigned int max_solutions;

  CnDockingParameters()
      : coarse_spacing(-1.), fine_spacing(0.5), angle_step(-1.),
        peaks_per_direction(4), n_rescore(2000), n_refine(100),
        refine_max_evaluations(120), cluster_rmsd(2.0), max_solutions(5000) {}
};

//! A candidate Cn assembly: interface score (higher is better) and the dock
//! transformation, T, such that the assembly is T^k(monomer), k=0..n-1.
typedef std::pair<double, algebra::Transformation3D> CnDockingSolution;

//! Generate candidate Cn assemblies of a monomer by docking.
/** This is an open replacement for the symmetric docking in libTAU.
    Every dock transformation is a rotation by 360/cn degrees about some
    axis, and so is determined by the axis direction (2 degrees of freedom)
    and the axis position (2 degrees of freedom). For each sampled axis
    direction, all axis positions are scored at once by FFT correlation of
    the monomer's contact potential with the rotated monomer. The best
    candidates are rescored on a finer grid, locally refined (with a clash
    penalty against non-neighboring subunits for cn >= 4) and clustered.

    \param[in] coords Coordinates of the monomer's heavy atoms (or of its
                      CA atoms only, if ca_only is true).
    \param[in] cn Cyclic symmetry degree (>= 2).
    \param[in] ca_only Whether coords contains only CA atoms.
    \param[in] params Docking parameters.
    \return Solutions sorted by decreasing score.
 */
IMPCNMULTIFITEXPORT std::vector<CnDockingSolution> dock_cn(
    const algebra::Vector3Ds &coords, int cn, bool ca_only,
    const CnDockingParameters &params = CnDockingParameters());

IMPCNMULTIFIT_END_INTERNAL_NAMESPACE

#endif /* IMPCNMULTIFIT_INTERNAL_CN_DOCKING_H */
