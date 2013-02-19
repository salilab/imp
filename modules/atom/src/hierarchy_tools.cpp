/**
 *  \file atom/hierarchy_tools.cpp
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/hierarchy_tools.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/estimates.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/distance.h>
#include <IMP/atom/Copy.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/core/ExcludedVolumeRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/generic.h>
#include <IMP/container/generic.h>
#include <IMP/container/AllBipartitePairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/ConnectingPairContainer.h>
#include <IMP/constants.h>
#include <algorithm>
IMPATOM_BEGIN_NAMESPACE

namespace {
  std::pair<int, double> compute_n(double V, double r, double f) {
    double n=/*.5*(3*V+2*PI*cube(r*f)-6*PI*cube(r)*square(f))
               /((-3*square(f)+cube(f)+2)*cube(r)*PI);*/
      V/(4.0/3.0*PI*cube(r));
    int in= static_cast<int>(std::ceil(n));
    double rr= /*std::pow(V/(.666*(2*in-3*square(f)*n+cube(f)*n
                 +3*square(f)-cube(f))*PI), .333333);*/
      std::pow(V/(in*4.0/3.0*PI)/(1-f), .3333);
    return std::make_pair(in, rr);
  }

  /*
  // ignores overlap
  double get_volume(Hierarchy h) {
    if (core::XYZR::particle_is_instance(h)) {
      return algebra::get_volume(core::XYZR(h).get_sphere());
    } else {
      double volume=0;
      for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
        volume+= get_volume(h.get_child(i));
      }
      return volume;
    }
  }
  */
  void gather_residue_indices(Hierarchy h, Ints &inds) {
    if (Residue::particle_is_instance(h)) {
      int i=Residue(h).get_index();
      inds.push_back(i);
    } else if (Fragment::particle_is_instance(h)
               && h.get_number_of_children() != 0) {
      Ints v= Fragment(h).get_residue_indexes();
      inds.insert(inds.end(), v.begin(), v.end());
    } else if (Domain::particle_is_instance(h)
               && h.get_number_of_children() == 0) {
      Domain d(h);
      for ( int i=d.get_begin_index();
           i != d.get_end_index(); ++i) {
        inds.push_back(i);
      }
    } else if (Atom::particle_is_instance(h)) {
      Residue r= get_residue(Atom(h));
      inds.push_back(r.get_index());
    } else {
      for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
        gather_residue_indices(h.get_child(i), inds);
      }
    }
  }


  double get_volume_measurement(algebra::Sphere3Ds ss,
                                double resolution) {
#ifdef IMP_ATOM_USE_IMP_CGAL
     // smooth
     for (unsigned int i=0; i< ss.size(); ++i) {
       ss[i]= algebra::Sphere3D(ss[i].get_center(),
                                ss[i].get_radius()+resolution);
     }
     double v= algebra::get_surface_area_and_volume(ss).second;
     return  4.0/3.0*PI*cube(algebra::get_ball_radius_from_volume_3d(v)
                             -resolution);
#else
     IMP_UNUSED(resolution);
     double v=0;
     for (unsigned int i=0; i< ss.size(); ++i) {
       v+= algebra::get_volume(ss[i]);
     }
     return v;
#endif
  }


 void setup_as_approximation_internal(Particle* p,
                                      const ParticlesTemp &other,
                                      double resolution=-1,
                                      double volume=-1, double mass=-1) {
   IMP_USAGE_CHECK(volume==-1 || volume>0,
                   "Volume must be positive if specified");
   IMP_USAGE_CHECK(other.size() >0,
                   "Must pass particles to approximate");
   double m=0;
   algebra::Sphere3Ds ss;
   Ints inds;
   algebra::Vector3D vv(0,0,0);
   for (unsigned int i=0; i< other.size(); ++i) {
     m+= get_mass(Selection(Hierarchy(other[i])));
     gather_residue_indices(Hierarchy(other[i]), inds);
     core::XYZR d(other[i]);
     if (volume <0) {
       ss.push_back(d.get_sphere());
     }
     vv+= d.get_coordinates();
   }
   if (resolution < 0) {
     algebra::BoundingBox3D bb;
     for (unsigned int i=0; i< ss.size(); ++i) {
       bb+= get_bounding_box(ss[i]);
     }
     resolution= (bb.get_corner(0)-bb.get_corner(1)).get_magnitude()/2.0;
   }
   if (mass>=0) {
     m=mass;
   }
   if (!Residue::particle_is_instance(p)
       && !Fragment::particle_is_instance(p)
       && !Domain::particle_is_instance(p)) {
     Fragment f= Fragment::setup_particle(p);
     f.set_residue_indexes(inds);
   }
   if (!Mass::particle_is_instance(p)) {
     Mass::setup_particle(p, m);
   } else {
     Mass(p).set_mass(m);
   }
   algebra::Sphere3D s;
   algebra::Vector3D center= vv/other.size();
   if (volume>=0) {
     s=algebra::Sphere3D(center,
                           algebra::get_ball_radius_from_volume_3d(volume));
   } else {
     //IMP_LOG_VERBOSE( "Approximating volume." << std::endl);
     double v= get_volume_measurement(ss, resolution);
     s=algebra::Sphere3D(center,
                           algebra::get_ball_radius_from_volume_3d(v));
   }

   if (core::XYZR::particle_is_instance(p)) {
     core::XYZR(p).set_sphere(s);
   } else {
     core::XYZR::setup_particle(p,s);
   }
 }


}

Hierarchy create_protein(Model *m,
                         std::string name,
                         double resolution,
                         int number_of_residues,
                         int first_residue_index,
                         double volume,
                         bool ismol) {
  double mass= atom::get_mass_from_number_of_residues(number_of_residues)/1000;
  if (volume < 0) {
    volume= atom::get_volume_from_mass(mass*1000);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac=.2;
  std::pair<int, double> nr= compute_n(volume, resolution, overlap_frac);
  Hierarchy pd=Hierarchy::setup_particle(new Particle(m));
  if (ismol) Molecule::setup_particle(pd);
  pd->set_name(name);
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(m);
    std::ostringstream oss;
    oss << name << "-" << i;
    pc->set_name(oss.str());
    atom::Fragment pcd
      =atom::Fragment::setup_particle(pc);
    pd.add_child(pcd);
    core::XYZR xyzd=core::XYZR::setup_particle(pc);
    xyzd.set_radius(nr.second);
    xyzd.set_coordinates_are_optimized(true);
    ps.push_back(pc);
    Ints indexes;
    for (int j= i*(number_of_residues/nr.first)
           + first_residue_index; j <(i+1)*(number_of_residues/nr.first)
           + first_residue_index; ++j) {
      indexes.push_back(j);
    }
    pcd.set_residue_indexes(indexes);
    atom::Mass::setup_particle(pc, mass/nr.first);
  }
  IMP_INTERNAL_CHECK(pd.get_is_valid(true),
                     "Invalid hierarchy produced " << pd);
  return pd;
}



Hierarchy create_protein(Model *m,
                         std::string name,
                         double resolution,
                         const Ints db) {
  Hierarchy root= Hierarchy::setup_particle(new Particle(m));
  Domain::setup_particle(root, db.front(),
                         db.back());
  for (unsigned int i=1; i< db.size(); ++i) {
    std::ostringstream oss;
    oss << name << i;
    Hierarchy cur= create_protein(m, oss.str(), resolution,
                                  db[i]-db[i-1], db[i-1],
                                  atom::get_volume_from_mass(
      atom::get_mass_from_number_of_residues(db[i]-db[i-1])),
                                  false);
    root.add_child(cur);
  }
  Molecule::setup_particle(root);
  root->set_name(name);
  return root;
}


namespace {

  // create a particle which approximates the input set
  Hierarchy create_approximation_of_residues(const Hierarchies &t) {
    static WarningContext wc;
    IMP_IF_CHECK(USAGE) {
      for (unsigned int i=0; i< t.size(); ++i) {
        IMP_USAGE_CHECK(Residue::particle_is_instance(t[i]),
                        "The residue is not a residue, it is "
                        << t[i]);
      }
    }
    if (t.empty()) {
      return Hierarchy();
    }
    double v=0;
    for (unsigned int i=0; i< t.size();  ++i) {
      ResidueType rt= Residue(t[i]).get_residue_type();
      try {
        v+= get_volume_from_residue_type(rt);
      } catch (ValueException) {
        IMP_WARN_ONCE(rt.get_string(),
                      "Computing volume for non-standard residue "
                      << rt, wc);
        algebra::Sphere3Ds ss;
        Hierarchies gl= get_leaves(t[i]);
        for (unsigned int i=0; i< gl.size(); ++i) {
          ss.push_back(core::XYZR(gl[i]).get_sphere());
        }
        v+= get_volume_measurement(ss, 5.0);
      }
    }
    Model *mm= t[0]->get_model();
    Particle *p= new Particle(mm);
    ParticlesTemp children;
    for (unsigned int i=0; i< t.size(); ++i) {
      Hierarchies cur= t[i].get_children();
      children.insert(children.end(), cur.begin(), cur.end());
    }
    setup_as_approximation_internal(p, children,
                                    -1,
                                    v);
    std::ostringstream oss;
    Ints rids= Fragment(p).get_residue_indexes();
    std::sort(rids.begin(), rids.end());
    oss << "Fragment";
    if (!rids.empty()) {
      oss << " [" << rids.front() << "-" << rids.back()+1
          << ")";
    }
    p->set_name(oss.str());
    wc.dump_warnings();
    return Hierarchy(p);
  }
}

namespace {
Hierarchy create_simplified_along_backbone(Chain in,
                                           int num_res,
                                           bool keep_detailed) {
  if (in.get_number_of_children() ==0) {
    return Hierarchy();
  }
  IntRanges rs;
  int ib= Residue(in.get_child(0)).get_index();
  int ie= Residue(in.get_child(in.get_number_of_children()-1)).get_index()+1;
  int i;
  for (i=ib; i< ie; i+= num_res) {
    rs.push_back(IntRange(i, std::min(i+num_res, ie)));
  }
  return create_simplified_along_backbone(in, rs, keep_detailed);
}

}


Hierarchy create_simplified_along_backbone(Hierarchy in,
                                           int num_res,
                                           bool keep_detailed) {
  Hierarchies chains= get_by_type(in, CHAIN_TYPE);
  if (chains.size() > 1) {
    Hierarchy root= Hierarchy::setup_particle(new Particle(in->get_model(),
                                                           in->get_name()));
    for (unsigned int i=0; i< chains.size(); ++i) {
      Chain chain(chains[i].get_particle());
      root.add_child(create_simplified_along_backbone(chain, num_res));
    }
    return root;
  } else if (chains.size()==1) {
    // make sure to cast it to chain to get the right overload
    return create_simplified_along_backbone(Chain(chains[0]), num_res,
                                            keep_detailed);
  } else {
    IMP_THROW("No chains to simplify", ValueException);
  }
}



Hierarchy
create_simplified_along_backbone(Chain in,
                                 const IntRanges& residue_segments,
                                 bool keep_detailed) {
  IMP_USAGE_CHECK(in.get_is_valid(true), "Chain " << in
                  << " is not valid.");
  if (in.get_number_of_children() ==0 || residue_segments.empty()) {
    IMP_LOG_TERSE( "Nothing to simplify in " << (in? in->get_name(): "nullptr")
            << " with " << residue_segments.size() << " segments.\n");
    return Hierarchy();
  }
  for (unsigned int i=0; i< residue_segments.size(); ++i) {
    IMP_USAGE_CHECK(residue_segments[i].first < residue_segments[i].second,
                    "Residue intervals must be non-empty");
  }
  IMP_IF_LOG(VERBOSE) {
    for (unsigned int i=0; i < residue_segments.size(); ++i) {
      IMP_LOG_VERBOSE( "[" << residue_segments[i].first
              << "..." << residue_segments[i].second << ") ");
    }
    IMP_LOG_VERBOSE( std::endl);
  }
  unsigned int cur_segment=0;
  Hierarchies cur;
  Hierarchy root=create_clone_one(in);
  for (unsigned int i=0; i< in.get_number_of_children(); ++i) {
    Hierarchy child=in.get_child(i);
    int index= Residue(child).get_index();
    IMP_LOG_VERBOSE( "Processing residue " << index
            << " with range " << residue_segments[cur_segment].first
            << " " << residue_segments[cur_segment].second << std::endl);
    if (index >= residue_segments[cur_segment].first
        && index < residue_segments[cur_segment].second) {
    } else if (!cur.empty()) {
      IMP_LOG_VERBOSE( "Added particle for "
              << residue_segments[cur_segment].first
              << "..." << residue_segments[cur_segment].second
              << std::endl);
      Hierarchy cur_approx=create_approximation_of_residues(cur);
      root.add_child(cur_approx);
      if (keep_detailed) {
        for (unsigned int j=0; j< cur.size(); ++j) {
          cur[j].get_parent().remove_child(cur[j]);
          cur_approx.add_child(cur[j]);
        }
      }
      cur.clear();
      ++cur_segment;
    }
    cur.push_back(child);
  }
  if (!cur.empty()) {
    root.add_child(create_approximation_of_residues(cur));
  }
  /*#ifdef IMP_ATOM_USE_IMP_CGAL
  double ov= get_volume(in);
  double cv= get_volume(root);
  double scale=1;
  ParticlesTemp rt= get_by_type(root, XYZR_TYPE);
  Floats radii(rt.size());
  for (unsigned int i=0; i< rt.size(); ++i) {
    core::XYZR d(rt[i]);
    radii[i]=d.get_radius();
  }
  do {
    show(root);
    double f= ov/cv*scale;
    scale*=.95;
    IMP_LOG_TERSE( "Bumping radius by " << f << std::endl);
    for (unsigned int i=0; i< rt.size(); ++i) {
      core::XYZR d(rt[i]);
      d.set_radius(radii[i]*f);
    }
    double nv=get_volume(root);
    IMP_LOG_TERSE( "Got volume " << nv << " " << ov << std::endl);
    if (nv < ov) {
      break;
    }
  } while (true);
#else
#endif*/
  IMP_INTERNAL_CHECK(root.get_is_valid(true),
                     "Invalid hierarchy produced " << root);
  return root;
}





 void setup_as_approximation(Particle* p,
                             const ParticlesTemp &other,
                             double resolution) {
   setup_as_approximation_internal(p, other, resolution);
 }


void setup_as_approximation(Hierarchy h, double resolution) {
  setup_as_approximation_internal(h, get_leaves(h), resolution);
}

IMPATOM_END_NAMESPACE
