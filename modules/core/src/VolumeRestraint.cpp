/**
 *  \file VolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#include "IMP/core/VolumeRestraint.h"
#include <IMP/core/XYZR.h>

IMPCORE_BEGIN_NAMESPACE

VolumeRestraint::VolumeRestraint(UnaryFunction *f,
                                 SingletonContainer *sc,
                                 double volume):
  sc_(sc), f_(f), volume_(volume),
  grid_(100,100,100,
        algebra::BoundingBox3D(algebra::VectorD<3>(0,0,0),
                               algebra::VectorD<3>(1,1,1)),
        -1)
{
}



double
VolumeRestraint::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_LOG(VERBOSE, "Begin volume restraint." << std::endl);
  algebra::BoundingBox3D bb3;
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    XYZR d(sc_->get_particle(i));
    bb3+= algebra::get_bounding_box(d.get_sphere());
  }
  algebra::VectorD<3> diag= bb3.get_corner(1)-bb3.get_corner(0);
  double ms= std::max(diag[0], std::max(diag[1], diag[2]));
  std::vector<int> volumes(sc_->get_number_of_particles(), 0),
    areas(sc_->get_number_of_particles(), 0);
  bool is_zero=false;
  if (ms >.0001) {
    algebra::VectorD<3> vms(ms,ms,ms);
    bb3= algebra::BoundingBox3D(bb3.get_corner(0)-.1*vms,
                                bb3.get_corner(0)+1.2*vms);
    IMP_LOG(VERBOSE, "Bounding box is " << bb3 << std::endl);
    grid_.set_bounding_box(bb3);
    std::fill(grid_.voxels_begin(), grid_.voxels_end(), -1);
    for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
      XYZR d(sc_->get_particle(i));
      algebra::SphereD<3> s= d.get_sphere();
      algebra::BoundingBox3D bb= algebra::get_bounding_box(d.get_sphere());
      Grid::ExtendedIndex vl= grid_.get_extended_index(bb.get_corner(0));
      Grid::ExtendedIndex vu= grid_.get_extended_index(bb.get_corner(1));
      //std::cout << vl << " " << vu << std::endl;
      for (Grid::IndexIterator it= grid_.indexes_begin(vl, vu);
           it != grid_.indexes_end(vl, vu); ++it) {
        //std::cout << "Inspecting " << *it << std::endl;
        algebra::VectorD<3> c= grid_.get_center(*it);
        if (s.get_contains(c)) {
          grid_[*it]= i;
          ++volumes[i];
          Grid::ExtendedIndex vs[]={grid_.get_offset(*it, 1,0,0),
                                   grid_.get_offset(*it, -1,0,0),
                                   grid_.get_offset(*it, 0,1,0),
                                   grid_.get_offset(*it, 0,-1,0),
                                   grid_.get_offset(*it, 0,0,1),
                                   grid_.get_offset(*it, 0,0,-1)};
          for (unsigned int j=0; j< 6; ++j) {
            if (!s.get_contains(grid_.get_center(vs[j]))) {
              ++areas[i];
              break;
            }
          }
        }
      }
    }

  } else {
    is_zero=true;
  }
  algebra::VectorD<3> v= grid_.get_unit_cell();
  double vc=v[0]*v[1]*v[2];
  unsigned int filled=0;
  if (!da) {
    if (is_zero) return f_->evaluate(0-volume_);
    else {
      for (Grid::IndexIterator it= grid_.all_indexes_begin();
           it != grid_.all_indexes_end(); ++it) {
        if (grid_.get_voxel(*it) != -1) ++filled;
      }
      double volume= filled*vc;
      IMP_LOG(VERBOSE, "Volume is " << volume
              << " and target is " << volume_
              << " and volume element is " << vc << std::endl);

      double rv= f_->evaluate(volume-volume_);
      return rv;
    }
  } else {
    std::vector<int> os[3];
    std::vector<int> rs(sc_->get_number_of_particles(), 0);
    double volume;
    if (is_zero) {
      volume=0;
      std::fill(rs.begin(), rs.end(), 1);
    } else {
      for (unsigned int i=0; i< 3; ++i) {
        os[i].resize(sc_->get_number_of_particles(), 0);
      }
      for (Grid::IndexIterator it= grid_.all_indexes_begin();
           it != grid_.all_indexes_end(); ++it) {
        if (grid_.get_voxel(*it) != -1) {
          ++filled;
          Grid::ExtendedIndex vs[]={grid_.get_offset(*it, 1,0,0),
                                   grid_.get_offset(*it, -1,0,0),
                                   grid_.get_offset(*it, 0,1,0),
                                   grid_.get_offset(*it, 0,-1,0),
                                   grid_.get_offset(*it, 0,0,1),
                                   grid_.get_offset(*it, 0,0,-1)};
          unsigned int s= grid_.get_voxel(*it);
          bool ri=false;
          for (unsigned int j=0; j< 6; ++j) {
            //if (grid_.get_index(vs[j])!= Grid::Index()) {
            if (grid_.get_voxel(grid_.get_index(vs[j])) ==-1) {
              int dir= (j%2 ==0) ? 1:-1;
              int c= j/2;
              os[c][s]+=dir;
              if (!ri) {
                ++rs[s];
                ri=true;
              }
            }
            //}
          }
        }
      }
      volume= filled*vc;
      IMP_LOG(VERBOSE, "Volume is " << volume
              << " and target is " << volume_
              << " and element is " << v << std::endl);
    }
    std::pair<double, double> rv= f_->evaluate_with_derivative(volume-volume_);
    IMP_LOG(VERBOSE, "Unary values are "
            << rv.first << " " << rv.second
            << std::endl);
    for (unsigned int i=0; i < sc_->get_number_of_particles(); ++i) {
      IMP_LOG(VERBOSE, "For particle " << sc_->get_particle(i)->get_name()
              << " at " << XYZR(sc_->get_particle(i))
              << " sums are " << os[0][i] << " "
              << os[1][i] << " " << os[2][i]
              << " " << rs[i] << std::endl);
      // os[c] is how many cells will be filled if x shifts up
      XYZR dec(sc_->get_particle(i));
      for (unsigned int j=0; j< 3; ++j) {
        double af= os[j][i]/static_cast<double>(areas[i]);
        double d= rv.second*af*4*PI*square(dec.get_radius());
        dec.add_to_derivative(j, d,*da);
        IMP_LOG(VERBOSE, "Adding " << d << " to coordinate "
                << j << " derivative"
                << std::endl);
      }
      double rf=rs[i]/static_cast<double>(areas[i]);
      double dr=rv.second*rf*4*PI*square(dec.get_radius());
      IMP_LOG(VERBOSE, "Adding " << dr
              << " to radius derivative from "
              << rf << std::endl);
      dec.add_to_radius_derivative(dr, *da);
    }
    return rv.first;
  }
}

void VolumeRestraint::do_show(std::ostream &out) const {
  out << "volume " << volume_ << std::endl;
  out << "container " << sc_ << std::endl;
}

ParticlesList VolumeRestraint::get_interacting_particles() const {
  return ParticlesList(1,sc_->get_contained_particles());
}

ParticlesTemp VolumeRestraint::get_input_particles() const {
  return sc_->get_particles();
}


ContainersTemp VolumeRestraint::get_input_containers() const {
  return ContainersTemp(1, sc_);
}


IMPCORE_END_NAMESPACE
