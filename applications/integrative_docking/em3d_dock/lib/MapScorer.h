/**
 *  \file MapScorer   \brief A class for fast cc scoring of docking models
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_MAP_SCORER_H
#define IMP_MAP_SCORER_H

#include <IMP/em/DensityMap.h>
#include <IMP/em/SampledDensityMap.h>
#include <IMP/em/MRCReaderWriter.h>
#include <IMP/em/CoarseCC.h>

class MapScorer {
public:
  MapScorer(IMP::Particles& rec_particles, IMP::Particles& lig_particles,
            const IMP::em::DensityMap& map);

  MapScorer(IMP::Particles& particles, const IMP::em::DensityMap& map);

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
  IMP::em::SampledDensityMap *receptor_map_, *ligand_map_, *docked_complex_map_;
};

#endif /* IMP_MAP_SCORER_H */
