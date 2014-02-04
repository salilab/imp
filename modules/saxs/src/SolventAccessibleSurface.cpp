/**
 * \file SolventAccessibleSurface \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/saxs/SolventAccessibleSurface.h>
#include <IMP/constants.h>
#include <IMP/algebra/standard_grids.h>

IMPSAXS_BEGIN_NAMESPACE

IMP::Floats SolventAccessibleSurface::get_solvent_accessibility(
    const core::XYZRs& ps, float probe_radius, float density) {
  IMP::Floats res;
  algebra::Vector3Ds coordinates(ps.size());
  IMP::Floats radii(ps.size());
  for (unsigned int i = 0; i < ps.size(); i++) {
    coordinates[i] = ps[i].get_coordinates();
    radii[i] = ps[i].get_radius();
  }

  // generate sphere dots for radii present in the ps set
  create_sphere_dots(ps, density);

  // init grid
  typedef IMP::algebra::DenseGrid3D<IMP::Ints> Grid;
  IMP::algebra::BoundingBox3D bb(coordinates);
  Grid grid(2.0, bb);
  for (unsigned int i = 0; i < coordinates.size(); i++) {
    Grid::Index grid_index = grid.get_nearest_index(coordinates[i]);
    grid[grid_index].push_back(i);
  }

  // compute surface
  static float max_radius = 3.0;
  for (unsigned int i = 0; i < ps.size(); i++) {
    const float atom_radius = radii[i];
    float radius = atom_radius + 2 * probe_radius + max_radius;
    // query
    IMP::algebra::BoundingBox3D bb(coordinates[i]);
    bb += radius;
    Grid::ExtendedIndex lb = grid.get_extended_index(bb.get_corner(0)),
                        ub = grid.get_extended_index(bb.get_corner(1));
    std::vector<int> neighbours1, neighbours2;
    for (Grid::IndexIterator it = grid.indexes_begin(lb, ub);
         it != grid.indexes_end(lb, ub); ++it) {
      for (unsigned int vIndex = 0; vIndex < grid[*it].size(); vIndex++) {
        int mol_index = grid[*it][vIndex];
        float radius_sum1 = atom_radius + radii[mol_index];
        float radius_sum2 = radius_sum1 + 2 * probe_radius;
        float dist2 = algebra::get_squared_distance(coordinates[i],
                                                    coordinates[mol_index]);

        if (dist2 < radius_sum1 * radius_sum1)
          neighbours1.push_back(mol_index);
        else if (dist2 < radius_sum2 * radius_sum2)
          neighbours2.push_back(mol_index);
      }
    }

    float ratio = (atom_radius + probe_radius) / atom_radius;
    const algebra::Vector3Ds& spoints = get_sphere_dots(atom_radius);

    int dotNum = 0;
    for (unsigned int s_index = 0; s_index < spoints.size(); s_index++) {
      IMP::algebra::Vector3D probe_center =
          coordinates[i] + ratio * spoints[s_index];
      // check for intersection with neighbours1
      bool collides = false;
      for (unsigned int n_index = 0; n_index < neighbours1.size(); n_index++) {
        if (is_intersecting(probe_center, coordinates[neighbours1[n_index]],
                            probe_radius, radii[neighbours1[n_index]])) {
          collides = true;
          break;
        }
      }
      if (!collides) {  // check for intersection with neighbours2
        for (unsigned int n_index = 0; n_index < neighbours2.size();
             n_index++) {
          if (is_intersecting(probe_center, coordinates[neighbours2[n_index]],
                              probe_radius, radii[neighbours2[n_index]])) {
            collides = true;
            break;
          }
        }
      }
      if (!collides) dotNum++;
    }
    res.push_back((float)dotNum / spoints.size());
  }
  return res;
}

algebra::Vector3Ds SolventAccessibleSurface::create_sphere_dots(float radius,
                                                                float density) {
  algebra::Vector3Ds res;
  float num_equat = 2 * PI * radius * sqrt(density);
  float vert_count = 0.5 * num_equat;

  for (int i = 0; i < vert_count; i++) {
    float phi = (PI * i) / vert_count;
    float z = cos(phi);
    float xy = sin(phi);
    float horz_count = xy * num_equat;
    for (int j = 0; j < horz_count - 1; j++) {
      float teta = (2 * PI * j) / horz_count;
      float x = xy * cos(teta);
      float y = xy * sin(teta);
      res.push_back(algebra::Vector3D(radius * x, radius * y, radius * z));
    }
  }
  return res;
}

void SolventAccessibleSurface::create_sphere_dots(const core::XYZRs& ps,
                                                  float density) {

  if (radii2type_.size() > 0 && density_ != density) {
    radii2type_.clear();
    sphere_dots_.clear();
    density_ = density;
  }
  for (unsigned int i = 0; i < ps.size(); i++) {
    float r = ps[i].get_radius();
    boost::unordered_map<float, int>::const_iterator it = radii2type_.find(r);
    if (it == radii2type_.end()) {
      int type = radii2type_.size();
      radii2type_[r] = type;
      algebra::Vector3Ds dots = create_sphere_dots(r, density);
      sphere_dots_.push_back(dots);
    }
  }
}

IMPSAXS_END_NAMESPACE
