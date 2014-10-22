/**
 * \file soap_score.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_SOAP_SCORE_H
#define IMP_SOAP_SCORE_H

#include <IMP/score_functor/Soap.h>
#include <IMP/score_functor/OrientedSoap.h>

#include <IMP/kernel/Model.h>

#include <IMP/algebra/standard_grids.h>

#include <IMP/atom/StereochemistryPairFilter.h>

namespace {

double soap_score(const IMP::score_functor::Soap* soap_score,
                  IMP::kernel::Model* model, IMP::kernel::ParticleIndexes& pis1,
                  IMP::kernel::ParticleIndexes& pis2,
                  float distance_threshold) {

  IMP::algebra::Vector3Ds coordinates1;
  for (unsigned int i = 0; i < pis1.size(); i++) {
    coordinates1.push_back(IMP::core::XYZ(model, pis1[i]).get_coordinates());
  }
  float distance_threshold2 = distance_threshold * distance_threshold;

  // save one pdb in grid for faster interface finding
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates1);
  Grid grid(2.0, bb);
  for (unsigned int j = 0; j < coordinates1.size(); j++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates1[j]);
    grid[grid_index].push_back(j);
  }

  // score
  double score = 0.0;
  // iterate ligand atoms
  for (unsigned int l_index = 0; l_index < pis2.size(); l_index++) {
    IMP::core::XYZ d(model, pis2[l_index]);
    IMP::algebra::Vector3D v(d.get_coordinates());
    // access grid to see if interface atom
    IMP::algebra::BoundingBox3D bb(v);
    bb += distance_threshold;
    Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
                        ub = grid.get_extended_index(bb.get_corner(1));
    for (Grid::IndexIterator it = grid.indexes_begin(lb, ub);
         it != grid.indexes_end(lb, ub); ++it) {
      for (unsigned int vIndex = 0; vIndex < grid[*it].size(); vIndex++) {
        int r_index = grid[*it][vIndex];
        float dist2 =
            IMP::algebra::get_squared_distance(coordinates1[r_index], v);
        if (dist2 < distance_threshold2) {
          score += soap_score->get_score(
              model,
              IMP::kernel::ParticleIndexPair(pis1[r_index], pis2[l_index]),
              sqrt(dist2));
        }
      }
    }
  }
  return score;
}

double oriented_soap_score(const IMP::score_functor::OrientedSoap* soap_score,
                           IMP::kernel::Model* model,
                           IMP::kernel::ParticleIndexes& pis1,
                           IMP::kernel::ParticleIndexes& pis2) {

  IMP::algebra::Vector3Ds coordinates1;
  for (unsigned int i = 0; i < pis1.size(); i++) {
    coordinates1.push_back(IMP::core::XYZ(model, pis1[i]).get_coordinates());
  }

  float distance_threshold = soap_score->get_distance_threshold();
  float distance_threshold2 = distance_threshold * distance_threshold;

  // save one pdb in grid for faster interface finding
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates1);
  Grid grid(2.0, bb);
  for (unsigned int j = 0; j < coordinates1.size(); j++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates1[j]);
    grid[grid_index].push_back(j);
  }

  // score
  double score = 0.0;
  // iterate ligand atoms
  for (unsigned int l_index = 0; l_index < pis2.size(); l_index++) {
    IMP::core::XYZ d(model, pis2[l_index]);
    IMP::algebra::Vector3D v(d.get_coordinates());
    // access grid to see if interface atom
    IMP::algebra::BoundingBox3D bb(v);
    bb += distance_threshold;
    Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
                        ub = grid.get_extended_index(bb.get_corner(1));
    for (Grid::IndexIterator it = grid.indexes_begin(lb, ub);
         it != grid.indexes_end(lb, ub); ++it) {
      for (unsigned int vIndex = 0; vIndex < grid[*it].size(); vIndex++) {
        int r_index = grid[*it][vIndex];
        float dist2 =
            IMP::algebra::get_squared_distance(coordinates1[r_index], v);
        if (dist2 < distance_threshold2) {
          score += soap_score->get_score(
              model,
              IMP::kernel::ParticleIndexPair(pis1[r_index], pis2[l_index]),
              sqrt(dist2));
        }
      }
    }
  }
  return score;
}


double oriented_soap_score(const IMP::score_functor::OrientedSoap* soap_score,
                           IMP::kernel::Model* model,
                           IMP::kernel::ParticleIndexes& pis,
                           IMP::atom::StereochemistryPairFilter* filter) {

  IMP::algebra::Vector3Ds coordinates;
  for (unsigned int i = 0; i < pis.size(); i++) {
    coordinates.push_back(IMP::core::XYZ(model, pis[i]).get_coordinates());
  }

  float distance_threshold = soap_score->get_distance_threshold();
  float distance_threshold2 = distance_threshold * distance_threshold;

  double score = 0.0;
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    for (unsigned int j = i+1; j < coordinates.size(); j++) {
      if (!filter->get_value_index(model, IMP::kernel::ParticleIndexPair(pis[i], pis[j]))) {
        float dist2 =
          IMP::algebra::get_squared_distance(coordinates[i], coordinates[j]);
        if (dist2 < distance_threshold2) {
          score += soap_score->get_score(
              model,
              IMP::kernel::ParticleIndexPair(pis[i], pis[j]),
              sqrt(dist2));
        }
      }
    }
  }
  return score;
}


}

#endif /* IMP_SOAP_SCORE_H */
