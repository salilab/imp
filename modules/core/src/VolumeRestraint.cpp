/**
 *  \file VolumeRestraint.cpp
 *  \brief XXXX.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/core/VolumeRestraint.h"
#include <IMP/core/XYZR.h>
#include <IMP/macros.h>

IMPCORE_BEGIN_NAMESPACE

#ifdef IMP_CORE_USE_IMP_CGAL

VolumeRestraint::VolumeRestraint(UnaryFunction *f,
                                 SingletonContainer *sc,
                                 double volume):
  Restraint(sc->get_model(), "VolumeRestraint%1%"),
  sc_(sc), f_(f), volume_(volume)
{
}



double
VolumeRestraint::unprotected_evaluate(DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  IMP_CHECK_VARIABLE(da);
  IMP_USAGE_CHECK(!da, "VolumeRestraint does not support derivatives.");
  algebra::Sphere3Ds spheres;
  IMP_FOREACH_SINGLETON(sc_, {
      spheres.push_back(XYZR(_1).get_sphere());
    });
  double vol= algebra::get_surface_area_and_volume(spheres).second;
  return f_->evaluate(vol-volume_);
  /*IMP_LOG_VERBOSE( "Begin volume restraint." << std::endl);
  algebra::BoundingBox3D bb3;
  IMP_FOREACH_SINGLETON(sc_, {
      XYZR d(_1);
      bb3+= algebra::get_bounding_box(d.get_sphere());
    });
  algebra::Vector3D diag= bb3.get_corner(1)-bb3.get_corner(0);
  double ms= std::max(diag[0], std::max(diag[1], diag[2]));
  Ints volumes(sc_->get_number_of_particles(), 0),
    areas(sc_->get_number_of_particles(), 0);
  bool is_zero=false;
  int count=0;
  if (ms >.0001) {
    algebra::Vector3D vms(ms,ms,ms);
    bb3= algebra::BoundingBox3D(bb3.get_corner(0)-.1*vms,
                                bb3.get_corner(0)+1.2*vms);
    IMP_LOG_VERBOSE( "Bounding box is " << bb3 << std::endl);
    grid_.set_bounding_box(bb3);
    std::fill(grid_.all_voxels_begin(), grid_.all_voxels_end(), -1);
    const int offsets[][3]={{1,0,0},
                            {-1,0,0},
                            {0,1,0},
                            {0,-1,0},
                            {0,0,1},
                            {0,0,-1}};
    IMP_FOREACH_SINGLETON(sc_, {
        ++count;
        XYZR d(_1);
        algebra::Sphere3D s= d.get_sphere();
        algebra::BoundingBox3D bb= algebra::get_bounding_box(d.get_sphere());
        Grid::ExtendedIndex vl
          = grid_.get_extended_index(bb.get_corner(0));
        Grid::ExtendedIndex vu
          = grid_.get_extended_index(bb.get_corner(1));
        //std::cout << vl << " " << vu << std::endl;
        for (Grid::IndexIterator it= grid_.indexes_begin(vl, vu);
             it != grid_.indexes_end(vl, vu); ++it) {
          //std::cout << "Inspecting " << *it << std::endl;
          algebra::Vector3D c= grid_.get_center(*it);
          if (s.get_contains(c)) {
            grid_[*it]= _2;
            ++volumes[_2];
            for (unsigned int j=0; j< 6; ++j) {
              Grid::ExtendedIndex ci
                =grid_.get_extended_index(*it).get_offset(offsets[j][0],
                                                          offsets[j][1],
                                                          offsets[j][2]);
              if (!s.get_contains(grid_.get_center(ci))) {
                ++areas[_2];
                break;
              }
            }
          }
        }
      });
  } else {
    is_zero=true;
  }
  algebra::Vector3D v= grid_.get_unit_cell();
  double vc=v[0]*v[1]*v[2];
  unsigned int filled=0;
  if (!da) {
    if (is_zero) return f_->evaluate(0-volume_);
    else {
      for (Grid::AllIndexIterator it= grid_.all_indexes_begin();
           it != grid_.all_indexes_end(); ++it) {
        if (grid_[*it] != -1) ++filled;
      }
      double volume= filled*vc;
      IMP_LOG_VERBOSE( "Volume is " << volume
              << " and target is " << volume_
              << " and volume element is " << vc << std::endl);

      double rv= f_->evaluate(volume-volume_);
      return rv;
    }
  } else {
    Ints os[3];
    Ints rs(count, 0);
    double volume;
    if (is_zero) {
      volume=0;
      std::fill(rs.begin(), rs.end(), 1);
    } else {
      for (unsigned int i=0; i< 3; ++i) {
        os[i].resize(count, 0);
      }
      for (Grid::AllIndexIterator it= grid_.all_indexes_begin();
           it != grid_.all_indexes_end(); ++it) {
        if (grid_[*it] != -1) {
          ++filled;
          Grid::ExtendedIndex ei= grid_.get_extended_index(*it);
          Grid::ExtendedIndex vs[]={ei.get_offset(1,0,0),
                                             ei.get_offset(-1,0,0),
                                             ei.get_offset(0,1,0),
                                             ei.get_offset(0,-1,0),
                                             ei.get_offset(0,0,1),
                                             ei.get_offset(0,0,-1)};
          unsigned int s= grid_[*it];
          bool ri=false;
          for (unsigned int j=0; j< 6; ++j) {
            //if (grid_.get_index(vs[j])!= Grid::Index()) {
            if (grid_[grid_.get_index(vs[j])] ==-1) {
              int dir= (j%2 ==0) ? 1:-1;
              int c= j/2;
              os[c][s]+=dir;
              if (!ri) {
                IMP_INTERNAL_CHECK(s < rs.size(), "Out of range");
                ++rs[s];
                ri=true;
              }
            }
            //}
          }
        }
      }
      volume= filled*vc;
      IMP_LOG_VERBOSE( "Volume is " << volume
              << " and target is " << volume_
              << " and element is " << v << std::endl);
    }
    std::pair<double, double> rv= f_->evaluate_with_derivative(volume-volume_);
    IMP_LOG_VERBOSE( "Unary values are "
            << rv.first << " " << rv.second
            << std::endl);
    IMP_FOREACH_SINGLETON(sc_, {
      IMP_LOG_VERBOSE( "For particle " << _1->get_name()
              << " at " << XYZR(_1)
              << " sums are " << os[0][_2] << " "
              << os[1][_2] << " " << os[2][_2]
              << " " << rs[_2] << std::endl);
      // os[c] is how many cells will be filled if x shifts up
      XYZR dec(_1);
      for (unsigned int j=0; j< 3; ++j) {
        double af= os[j][_2]/static_cast<double>(areas[_2]);
        double d= rv.second*af*4*PI*square(dec.get_radius());
        dec.add_to_derivative(j, d,*da);
        IMP_LOG_VERBOSE( "Adding " << d << " to coordinate "
                << j << " derivative"
                << std::endl);
      }
      double rf=rs[_2]/static_cast<double>(areas[_2]);
      double dr=rv.second*rf*4*PI*square(dec.get_radius());
      IMP_LOG_VERBOSE( "Adding " << dr
              << " to radius derivative from "
              << rf << std::endl);
      dec.add_to_radius_derivative(dr, *da);
      });
      return rv.first;*/
}

void VolumeRestraint::do_show(std::ostream &out) const {
  out << "volume " << volume_ << std::endl;
  out << "container " << sc_->get_name() << std::endl;
}

ParticlesTemp VolumeRestraint::get_input_particles() const {
  return sc_->get_particles();
}


ContainersTemp VolumeRestraint::get_input_containers() const {
  return ContainersTemp(1, sc_);
}

#endif
IMPCORE_END_NAMESPACE
