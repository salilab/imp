/**
 *  \file IMP/integrative_docking/MapScorer.h
 *  \brief A class for fast cc scoring of docking models
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_MAP_SCORER_H
#define IMPINTEGRATIVE_DOCKING_MAP_SCORER_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/CoarseCC.h>

IMPINTEGRATIVEDOCKING_BEGIN_NAMESPACE

class MapScorer {
 public:
  MapScorer(IMP::kernel::Particles& rec_particles,
            IMP::kernel::Particles& lig_particles,
            const IMP::em::DensityMap& map);

  MapScorer(IMP::kernel::Particles& particles, const IMP::em::DensityMap& map);

  float score(const IMP::algebra::Transformation3D& rec_trans,
              const IMP::algebra::Transformation3D& lig_trans) const;

  float score(const IMP::algebra::Transformation3D& map_trans) const;

 private:
  IMP::em::DensityMap* get_transformed(
      const IMP::algebra::Transformation3D& rec_trans,
      const IMP::algebra::Transformation3D& lig_trans) const;

  IMP::em::DensityMap* get_transformed(
      const IMP::algebra::Transformation3D& trans) const;

 private:
  const IMP::em::DensityMap& complex_map_;
  IMP::em::SampledDensityMap* receptor_map_, *ligand_map_, *docked_complex_map_;
};

IMPINTEGRATIVEDOCKING_END_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_MAP_SCORER_H */
