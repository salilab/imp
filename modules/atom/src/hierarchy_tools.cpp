/**
 *  \file atom/hierarchy_tools.cpp
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
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
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/TableRefiner.h>
#include <algorithm>

IMPATOM_BEGIN_NAMESPACE

namespace {
  std::pair<int, double> compute_n(double V, double r, double f) {
    double n=.5*(3*V+2*PI*cube(r*f)-6*PI*cube(r)*square(f))
      /((-3*square(f)+cube(f)+2)*cube(r)*PI);
    int in= static_cast<int>(std::ceil(n));
    double rr= std::pow(V/(.666*(2*in-3*square(f)*n+cube(f)*n
                                 +3*square(f)-cube(f))*PI), .333333);
    return std::make_pair(in, rr);
  }
}

Hierarchy create_protein(Model *m,
                         double resolution,
                         int number_of_residues,
                         int first_residue_index,
                         double volume,
                         double /*spring strength*/) {
  double mass= atom::get_mass_from_number_of_residues(number_of_residues)/1000;
  if (volume < 0) {
    volume= atom::get_volume_from_mass(mass*1000);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac=.2;
  std::pair<int, double> nr= compute_n(volume, resolution, overlap_frac);
  Hierarchy pd=Hierarchy::setup_particle(new Particle(m));
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(m);
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


namespace {

  double get_mass(Hierarchy h) {
    if (Mass::particle_is_instance(h)) {
      return Mass(h).get_mass();
    } else {
      double mass=0;
      for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
        mass+= get_mass(h.get_child(i));
      }
      return mass;
    }
  }
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
    } else {
      for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
        gather_residue_indices(h.get_child(i), inds);
      }
    }
  }

  // create a particle which approximates the input set
  Hierarchy create_approximation(const HierarchiesTemp &t) {
    if (t.empty()) {
      return Hierarchy();
    }
    double m=0, v=0;
    Ints inds;
    algebra::VectorD<3> vv(0,0,0);
    unsigned int n=0;
    for (unsigned int i=0; i< t.size(); ++i) {
      m+= get_mass(t[i]);
      v+= get_volume(t[i]);
      gather_residue_indices(t[i], inds);
      core::XYZsTemp ls(get_leaves(t[i]));
      for (unsigned int i=0; i< ls.size(); ++i) {
        vv+= ls[i].get_coordinates();
      }
      n+= ls.size();
    }
    Model *mm= t[0]->get_model();
    Particle *p= new Particle(mm);
    Fragment f= Fragment::setup_particle(p);
    f.set_residue_indexes(inds);
    Mass::setup_particle(p, m);
    core::XYZR::setup_particle(p,
                         algebra::SphereD<3>(vv/n,
                         algebra::get_ball_radius_from_volume_3d(v)));
    return f;
  }
}


IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
                                                   int num_res) {
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
  return create_simplified_along_backbone(in, rs);
}

IMPATOMEXPORT Hierarchy
create_simplified_along_backbone(Chain in,
                            const IntRanges& residue_segments) {
  if (in.get_number_of_children() ==0 || residue_segments.empty()) {
    return Hierarchy();
  }
  for (unsigned int i=0; i< residue_segments.size(); ++i) {
    IMP_USAGE_CHECK(residue_segments[i].first < residue_segments[i].second,
                    "Residue intervals must be non-empty");
  }
  IMP_IF_LOG(VERBOSE) {
    for (unsigned int i=0; i < residue_segments.size(); ++i) {
      IMP_LOG(VERBOSE, "[" << residue_segments[i].first
              << "..." << residue_segments[i].second << ") ");
    }
    IMP_LOG(VERBOSE, std::endl);
  }
  unsigned int cur_segment=0;
  HierarchiesTemp cur;
  Hierarchy root=create_clone_one(in);
  for (unsigned int i=0; i< in.get_number_of_children(); ++i) {
    Hierarchy child=in.get_child(i);
    int index= Residue(child).get_index();
    IMP_LOG(VERBOSE, "Processing residue " << index
            << " with range " << residue_segments[cur_segment].first
            << " " << residue_segments[cur_segment].second << std::endl);
    if (index >= residue_segments[cur_segment].first
        && index < residue_segments[cur_segment].second) {
    } else if (!cur.empty()) {
      IMP_LOG(TERSE, "Added particle for "
              << residue_segments[cur_segment].first
              << "..." << residue_segments[cur_segment].second
              << std::endl);
      root.add_child(create_approximation(cur));
      cur.clear();
      ++cur_segment;
    }
    cur.push_back(child);
  }
  if (!cur.empty()) {
    root.add_child(create_approximation(cur));
  }
  IMP_INTERNAL_CHECK(root.get_is_valid(true),
                     "Invalid hierarchy produced " << root);
  return root;
}


std::string get_molecule_name(Hierarchy h) {
  do {
    if (!Residue::particle_is_instance(h)
        && !Atom::particle_is_instance(h)
        && !Chain::particle_is_instance(h)) {
      return h->get_name();
    }
  } while (h=h.get_parent());
  IMP_THROW("Hierarchy " << h << " has no molecule name.",
            ValueException);
}

namespace {
  Ints get_tree_residue_indexes(Hierarchy h) {
    if (Residue::particle_is_instance(h)) {
      return Ints(1,Residue(h).get_index());
    }
    Ints ret;
    if (Domain::particle_is_instance(h)) {
      for ( int i=Domain(h).get_begin_index();
            i< Domain(h).get_end_index(); ++ i) {
        ret.push_back(i);
      }
    } else if (Fragment::particle_is_instance(h)) {
      Ints cur= Fragment(h).get_residue_indexes();
      ret.insert(ret.end(), cur.begin(), cur.end());
    }
    if (ret.empty()) {
      if (h.get_number_of_children() >0) {
        for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
          Ints cur= get_tree_residue_indexes(h.get_child(0));
          ret.insert(ret.end(), cur.begin(), cur.end());
        }
      }
    }
    return ret;
  }
}

Ints get_residue_indexes(Hierarchy h) {
    do {
      Ints ret= get_tree_residue_indexes(h);
      if (!ret.empty()) return ret;
    } while (h=h.get_parent());
    IMP_THROW("Hierarchy " << h << " has no residue index.",
              ValueException);
}
ResidueType get_residue_type(Hierarchy h) {
  do {
    if (Residue::particle_is_instance(h)) {
      return Residue(h).get_residue_type();
    }
  } while (h=h.get_parent());
    IMP_THROW("Hierarchy " << h << " has no residue type.",
              ValueException);
}
char get_chain(Hierarchy h) {
  do {
    if (Chain::particle_is_instance(h)) {
      return Chain(h).get_id();
    }
  } while (h=h.get_parent());
    IMP_THROW("Hierarchy " << h << " has no chain.",
              ValueException);
}
AtomType get_atom_type(Hierarchy h) {
  do {
    if (Atom::particle_is_instance(h)) {
      return Atom(h).get_atom_type();
    }
  } while (h=h.get_parent());
    IMP_THROW("Hierarchy " << h << " has no atom type.",
              ValueException);
}
std::string get_domain_name(Hierarchy h) {
  do {
    if (Domain::particle_is_instance(h)) {
      return Domain(h)->get_name();
    }
  } while (h=h.get_parent());
    IMP_THROW("Hierarchy " << h << " has no domain name.",
              ValueException);
}


bool Named::check_nonradius(Hierarchy h) const {
  try {
    if (!molecules_.empty()) {
      std::string molname= get_molecule_name(h);
      if (!std::binary_search(molecules_.begin(), molecules_.end(),
                              molname)) return false;
    }
    if (!residue_indices_.empty()) {
      Ints ris= get_residue_indexes(h);
      std::sort(ris.begin(), ris.end());
      Ints intersect;
      std::set_intersection(ris.begin(), ris.end(),
                            residue_indices_.begin(), residue_indices_.end(),
                            std::back_inserter(intersect));
      if (intersect.empty()) return false;
    }
    if (!residue_types_.empty()) {
      ResidueType rt= get_residue_type(h);
      if (!std::binary_search(residue_types_.begin(), residue_types_.end(),
                              rt)) return false;
    }
    if (!chains_.empty()) {
      char chain= get_chain(h);
      if (!std::binary_search(chains_.begin(), chains_.end(),
                              chain)) return false;
    }
    if (!atom_types_.empty()) {
      AtomType chain= get_atom_type(h);
      if (!std::binary_search(atom_types_.begin(), atom_types_.end(),
                              chain)) return false;
    }
    if (!domains_.empty()) {
      std::string chain= get_domain_name(h);
      if (!std::binary_search(domains_.begin(), domains_.end(),
                              chain)) return false;
    }
  } catch (ValueException) {
    return false;
  }
  return true;
}

bool Named::operator()(Hierarchy h) const
{
  try {
    if (!check_nonradius(h)) return false;
    if (!core::XYZ::particle_is_instance(h)) return false;
    bool found=false;
    for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
      if (check_nonradius(h.get_child(i))) {
        if (core::XYZR::particle_is_instance(h)
            && core::XYZR(h).get_radius() >radius_) {
          found=true;
          break;
        }
      }
    }
    if (found) return false;
    // check terminus
    if (terminus_!= NONE) {
      Hierarchy cur=h;
      bool fail=false;
      do {
        Hierarchy p= cur.get_parent();
        if (!p) break;
        unsigned int i= p.get_child_index(cur);
        if (terminus_==C && i+1 != p.get_number_of_children()){
          fail=true;
          break;
        } else if (terminus_==N && i != 0) {
          fail=true;
          break;
        }
        cur=p;
        if (!Fragment::particle_is_instance(cur)
            || !Domain::particle_is_instance(cur)
            || !Residue::particle_is_instance(cur)) break;
      } while (true);
      if (fail) return false;
    }
  } catch (ValueException) {
    return false;
  }
  return true;
}
ParticlesTemp Named::get_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< h_.size(); ++i) {
    core::gather_slice(h_[i], boost::bind(&IMP::atom::Named::operator(),
                                       this, _1), std::back_inserter(ret));
  }
  return ret;
}


Restraint* create_distance_restraint(const Named &n0,
                                     const Named &n1,
                                     double x0, double k) {
  ParticlesTemp p0= n0.get_particles();
  ParticlesTemp p1= n1.get_particles();
  Pointer<Restraint> ret;
  if (p0.empty() || p1.empty()) {
    IMP_THROW("Named does not refer to any particles.", ValueException);
  } else if (p1.size() ==1 && p0.size()==1) {
    double d= core::XYZR(p0[0]).get_radius()
      + core::XYZR(p1[0]).get_radius() + x0;
    ret= new core::PairRestraint(new core::HarmonicDistancePairScore(d, k),
                           ParticlePair(p0[0], p1[0]));
  } else {
    Pointer<core::TableRefiner> r= new core::TableRefiner();
    r->add_particle(n0.get_hierarchies()[0], p0);
    r->add_particle(n1.get_hierarchies()[0], p1);
    Pointer<PairScore> ps
      = new core::KClosePairsPairScore(
               new core::HarmonicSphereDistancePairScore(x0, k),
                                       r, 1);
    ret= new core::PairRestraint(ps, ParticlePair(n0.get_hierarchies()[0],
                                                  n1.get_hierarchies()[0]));
  }
  return ret.release();
}


IMPATOM_END_NAMESPACE
