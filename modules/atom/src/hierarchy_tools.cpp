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
     //IMP_LOG(VERBOSE, "Approximating volume." << std::endl);
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

}


Hierarchy create_simplified_along_backbone(Hierarchy in,
                                           int num_res) {
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
    return create_simplified_along_backbone(Chain(chains[0]), num_res);
  } else {
    IMP_THROW("No chains to simplify", ValueException);
  }
}



Hierarchy
create_simplified_along_backbone(Chain in,
                            const IntRanges& residue_segments) {
  IMP_USAGE_CHECK(in.get_is_valid(true), "Chain " << in
                  << " is not valid.");
  if (in.get_number_of_children() ==0 || residue_segments.empty()) {
    IMP_LOG(TERSE, "Nothing to simplify in " << (in? in->get_name(): "nullptr")
            << " with " << residue_segments.size() << " segments.\n");
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
  Hierarchies cur;
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
      IMP_LOG(VERBOSE, "Added particle for "
              << residue_segments[cur_segment].first
              << "..." << residue_segments[cur_segment].second
              << std::endl);
      root.add_child(create_approximation_of_residues(cur));
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
    IMP_LOG(TERSE, "Bumping radius by " << f << std::endl);
    for (unsigned int i=0; i< rt.size(); ++i) {
      core::XYZR d(rt[i]);
      d.set_radius(radii[i]*f);
    }
    double nv=get_volume(root);
    IMP_LOG(TERSE, "Got volume " << nv << " " << ov << std::endl);
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

std::string get_molecule_name(Hierarchy h) {
  do {
    if (Molecule::particle_is_instance(h)) {
      return h->get_name();
    }
  } while ((h=h.get_parent()));
  IMP_THROW("Hierarchy " << h << " has no residue index.",
            ValueException);
}

Ints get_residue_indexes(Hierarchy h) {
    do {
      Ints ret= get_tree_residue_indexes(h);
      if (!ret.empty()) return ret;
    } while ((h=h.get_parent()));
    IMP_THROW("Hierarchy " << h << " has no residue index.",
              ValueException);
}
ResidueType get_residue_type(Hierarchy h) {
  do {
    if (Residue::particle_is_instance(h)) {
      return Residue(h).get_residue_type();
    }
  } while ((h=h.get_parent()));
    IMP_THROW("Hierarchy " << h << " has no residue type.",
              ValueException);
}
int get_chain_id(Hierarchy h) {
  Chain c= get_chain(h);
  if (!c) {
    IMP_THROW("Hierarchy " << h << " has no chain.",
              ValueException);
  } else {
    return c.get_id();
  }
}
AtomType get_atom_type(Hierarchy h) {
  do {
    if (Atom::particle_is_instance(h)) {
      return Atom(h).get_atom_type();
    }
  } while ((h=h.get_parent()));
    IMP_THROW("Hierarchy " << h << " has no atom type.",
              ValueException);
}
std::string get_domain_name(Hierarchy h) {
  do {
    if (Domain::particle_is_instance(h)) {
      return Domain(h)->get_name();
    }
  } while ((h=h.get_parent()));
    IMP_THROW("Hierarchy " << h << " has no domain name.",
              ValueException);
}

int get_copy_index(Hierarchy h) {
  do {
    if (Copy::particle_is_instance(h)) {
      return Copy(h).get_copy_index();
    }
  } while ((h=h.get_parent()));
    IMP_THROW("Hierarchy " << h << " has number.",
              ValueException);
}


bool Selection::check_nonradius(Hierarchy h) const {
  try {
    if (!molecules_.empty()) {
      std::string name= get_molecule_name(h);
      if (!std::binary_search(molecules_.begin(), molecules_.end(),
                              name)) return false;
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
      int chain= get_chain_id(h);
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
    if (!copies_.empty()) {
      int copy=get_copy_index(h);
      if (!std::binary_search(copies_.begin(), copies_.end(),
                              copy)) return false;
    }
    if (!types_.empty()) {
      if (!core::Typed::particle_is_instance(h)) {
        return false;
      } else {
        core::Typed d(h);
        if (!std::binary_search(types_.begin(), types_.end(), d.get_type())) {
          return false;
        }
      }
    }
  } catch (ValueException) {
    return false;
  }
  return true;
}

bool Selection::operator()(Hierarchy h) const
{
  try {
    if (!check_nonradius(h)) return false;
    if (!core::XYZ::particle_is_instance(h)) return false;
    bool found=false;
    for (unsigned int i=0; i< h.get_number_of_children(); ++i) {
      Hierarchy c= h.get_child(i);
      if (check_nonradius(c)) {
        if (core::XYZR::particle_is_instance(c)) {
          double r= core::XYZR(c).get_radius();
          if (r >radius_) {
            found=true;
            break;
          }
        } else {
          /*std::cout << "Child " << h.get_child(i)->get_name()
            << " fails radius check" << std::endl;*/
        }
      } else {
        /*std::cout << "Child " << h.get_child(i)->get_name()
          << " fails non-radius check" << std::endl;*/
      }
    }
    if (found) return false;
    // check terminus
    if (terminus_!= NONE) {
      Hierarchy cur=h;
      bool fail=false;
      // make sure we pick the right atom
      if (Atom::particle_is_instance(cur)) {
        Atom a(cur);
        if (terminus_==C && a.get_atom_type() != AT_C
            && get_atom(Residue(cur.get_parent()), AT_C) != Atom()) {
          fail=true;
        }
        if (terminus_==N && a.get_atom_type() != AT_N
            && get_atom(Residue(cur.get_parent()), AT_N) != Atom()) {
          fail=true;
        }
        // don't pay attention to my position
        cur= cur.get_parent();
      }
      if (!fail) {
        do {
          Hierarchy p= cur.get_parent();
          if (!p) break;
          unsigned int i= cur.get_child_index();
          if (terminus_==C && i+1 != p.get_number_of_children()){
            fail=true;
            break;
          } else if (terminus_==N && i != 0) {
            fail=true;
            break;
          }
          cur=p;
          if (!Fragment::particle_is_instance(cur)
              && !Domain::particle_is_instance(cur)
              && !Residue::particle_is_instance(cur)) break;
        } while (true);
      }
      if (fail) return false;
    }
  } catch (ValueException) {
    return false;
  }
  return true;
}
ParticlesTemp Selection::get_selected_particles() const {
  ParticlesTemp ret;
  for (unsigned int i=0; i< h_.size(); ++i) {
    core::gather_slice(h_[i], boost::bind(&IMP::atom::Selection::operator(),
                                       this, _1), std::back_inserter(ret));
  }
  return ret;
}


namespace {
  template <class PS>
  Restraint* create_distance_restraint(const Selection &n0,
                                       const Selection &n1,
                                       PS *ps) {
    ParticlesTemp p0= n0.get_selected_particles();
    ParticlesTemp p1= n1.get_selected_particles();
    IMP_IF_CHECK(USAGE) {
      IMP::compatibility::set<Particle*> all(p0.begin(), p0.end());
      all.insert(p1.begin(), p1.end());
      IMP_USAGE_CHECK(all.size() == p0.size()+p1.size(),
                      "The two selections cannot overlap.");
    }
    Pointer<Restraint> ret;
    IMP_USAGE_CHECK(!p0.empty(), "Selection " << n0
                    << " does not refer to any particles.");
    IMP_USAGE_CHECK(!p1.empty(), "Selection " << n1
                    << " does not refer to any particles.");
    if (p1.size() ==1 && p0.size()==1) {
      IMP_LOG(TERSE, "Creating distance restraint between "
              << p0[0]->get_name() << " and "
              << p1[0]->get_name() << std::endl);
      ret= IMP::create_restraint(ps,
                                 ParticlePair(p0[0], p1[0]),
                                  "Atom distance restraint %1%");
    } else {
      IMP_LOG(TERSE, "Creating distance restraint between "
              << n0 << " and "
              << n1 << std::endl);
      /*if (p0.size()+p1.size() < 100) {
        ret=new core::KClosePairsRestraint(ps,
                                           p0, p1, 1,
                                           "Atom k distance restraint %1%");
                                           } else {*/
      Pointer<core::TableRefiner> r= new core::TableRefiner();
      r->add_particle(p0[0], p0);
      r->add_particle(p1[0], p1);
      IMP_NEW(core::KClosePairsPairScore,  nps, (ps,
      r, 1));
      ret= IMP::create_restraint(nps.get(), ParticlePair(p0[0],
                                                   p1[0]),
                                 "Atom k distance restraint %1%");
      //}
    }
    return ret.release();
  }
}

Restraint* create_distance_restraint(const Selection &n0,
                                     const Selection &n1,
                                     double x0, double k) {
  IMP_NEW(core::HarmonicSphereDistancePairScore, ps, (x0, k));
  return create_distance_restraint(n0, n1, ps.get());
}



Restraint* create_connectivity_restraint(const Selections &s,
                                         double x0,
                                         double k) {
  if (s.size() < 2) return nullptr;
  if (s.size() ==2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    Restraint *r= create_distance_restraint(s[0], s[1], ps.get());
    return r;
  } else {
    unsigned int max=0;
    for (unsigned int i=0; i< s.size(); ++i) {
      max=
    std::max(static_cast<unsigned int>(s[i].get_selected_particles().size()),
                 max);
    }
    if (max==1) {
      // special case all singletons
      ParticlesTemp particles;
      for (unsigned int i=0; i< s.size(); ++i) {
        particles.push_back(s[i].get_selected_particles()[0]);
      }
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0,k));
      IMP_NEW(container::ListSingletonContainer, lsc, (particles));
      IMP_NEW(container::ConnectingPairContainer, cpc, (lsc, 0));
      Pointer<Restraint> cr= container::create_restraint(hdps.get(), cpc.get());
      return cr.release();
    } else {
      IMP_NEW(core::TableRefiner, tr, ());
      ParticlesTemp rps;
      for (unsigned int i=0; i< s.size(); ++i) {
        ParticlesTemp ps= s[i].get_selected_particles();
        IMP_USAGE_CHECK(!ps.empty(), "Selection " << s[i]
                        << " does not contain any particles.");
        tr->add_particle(ps[0], ps);
        rps.push_back(ps[0]);
      }
      IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0,k));
      Pointer<PairScore> ps;
      IMP_LOG(TERSE, "Using closest pair score." << std::endl);
      ps=new core::KClosePairsPairScore(hdps, tr);
      IMP_NEW(IMP::internal::InternalListSingletonContainer, lsc,
              (rps[0]->get_model(), "Connectivity particles"));
      lsc->set(IMP::internal::get_index(rps));
      IMP_NEW(core::ConnectivityRestraint, cr, (ps, lsc));
      return cr.release();
    }
  }
}

Restraint* create_connectivity_restraint(const Selections &s,
                                         double k) {
  return create_connectivity_restraint(s, 0, k);
}


Restraint* create_internal_connectivity_restraint(const Selection &ss,
                                         double x0,
                                         double k) {
  ParticlesTemp s= ss.get_selected_particles();
  if (s.size() < 2) return nullptr;
  if (s.size() ==2) {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, ps, (x0, k));
    IMP_NEW(core::PairRestraint, r, (ps, ParticlePair(s[0], s[1])));
    return r.release();
  } else {
    IMP_NEW(core::HarmonicUpperBoundSphereDistancePairScore, hdps, (x0,k));
    IMP_NEW(container::ListSingletonContainer, lsc, (s));
    IMP_NEW(container::ConnectingPairContainer, cpc, (lsc, 0));
    Pointer<Restraint> cr= container::create_restraint(hdps.get(), cpc.get());
    return cr.release();
  }
}


Restraint* create_internal_connectivity_restraint(const Selection &s,
                                         double k) {
  return create_internal_connectivity_restraint(s, 0, k);
}


Restraint* create_excluded_volume_restraint(const Selections& ss) {
  ParticlesTemp ps;
  for (unsigned int i=0; i< ss.size(); ++i) {
    ParticlesTemp cps= ss[i].get_selected_particles();
    IMP_IF_LOG(TERSE) {
      IMP_LOG(TERSE, "Found ");
      for (unsigned int i=0; i< cps.size(); ++i) {
        IMP_LOG(TERSE, cps[i]->get_name() << " ");
      }
      IMP_LOG(TERSE, std::endl);
    }
    ps.insert(ps.end(), cps.begin(), cps.end());
  }
  IMP_NEW(IMP::internal::InternalListSingletonContainer, lsc,
          (ps[0]->get_model(), "Hierarchy EV particles"));
  lsc->set(IMP::internal::get_index(ps));
  IMP_NEW(core::ExcludedVolumeRestraint, evr, (lsc));
  evr->set_name("Hierarchy EV");
  return evr.release();
}


Restraint* create_excluded_volume_restraint(const Hierarchies &hs,
                                            double resolution) {
  Selections ss;
  for (unsigned int i=0; i< hs.size(); ++i) {
    Selection s(hs[i]);
    s.set_target_radius(resolution);
    ss.push_back(s);
  }
  return create_excluded_volume_restraint(ss);
}


core::XYZR create_cover(const Selection& s,
                        std::string name) {
  if (name.empty()) {
    name="atom cover";
  }
  ParticlesTemp ps= s.get_selected_particles();
  IMP_USAGE_CHECK(!ps.empty(),
                  "No particles selected.");
  Particle *p= new Particle(ps[0]->get_model());
  p->set_name(name);
  core::RigidBody rb;
  for (unsigned int i=0; i< ps.size(); ++i) {
    if (core::RigidMember::particle_is_instance(ps[i])) {
      if (!rb) {
        rb= core::RigidMember(ps[i]).get_rigid_body();
      } else {
        if (rb != core::RigidMember(ps[i]).get_rigid_body()) {
          rb= core::RigidBody();
          break;
        }
      }
    }
  }

  if (rb) {
    algebra::Sphere3Ds ss;
    for (unsigned int i=0; i< ps.size(); ++i) {
      ss.push_back(core::XYZR(ps[i]).get_sphere());
    }
    algebra::Sphere3D s= algebra::get_enclosing_sphere(ss);
    core::XYZR d= core::XYZR::setup_particle(p, s);
    rb.add_member(d);
    return d;
  } else {
    core::Cover c=core::Cover::setup_particle(p, core::XYZRs(ps));
    return c;
  }
}


void Selection::show(std::ostream &out) const {
  out << "Selection on \n";
  out << "  Hierarchies: ";
  for (unsigned int i=0; i< h_.size(); ++i) {
    out << h_[i]->get_name() << " ";
  }
  out << std::endl;
  if (!molecules_.empty()) {
    out << "  Molecules: ";
    for (unsigned int i=0; i< molecules_.size(); ++i) {
      out << molecules_[i] << " ";
    }
    out << std::endl;
  }
  if (!residue_indices_.empty()) {
    out << "  Residues: ";
    int start_run=-1000;
    int last_run = -1000;
    for (unsigned int i=0; i< residue_indices_.size(); ++i) {
      int c= residue_indices_[i];
      if (c== last_run+1) {
      } else {
        if (start_run != -1000) {
          if (last_run != start_run) {
            out << "[" << start_run << "..." << last_run << "] ";
          } else {
            out << start_run << " ";
          }
        }
        start_run=c;
      }
      last_run=c;
    }
    if (last_run != start_run) {
      out << "[" << start_run << "..." << last_run << "] ";
    } else {
      out << start_run;
    }
    out << std::endl;
  }
  if (!chains_.empty()) {
    out << "  Chains: " << chains_ << std::endl;
  }
  if (!atom_types_.empty()) {
    out << "  Atoms: ";
    for (unsigned int i=0; i< atom_types_.size(); ++i) {
      out << atom_types_[i] << " ";
    }
    out << std::endl;
  }
  if (!residue_types_.empty()) {
    out << "  Residues: ";
    for (unsigned int i=0; i< residue_types_.size(); ++i) {
      out << residue_types_[i] << " ";
    }
    out << std::endl;
  }
  if (radius_ >=0) {
    out << "  Radius: " << radius_;
    out << std::endl;
  }
}




 void setup_as_approximation(Particle* p,
                             const ParticlesTemp &other,
                             double resolution) {
   setup_as_approximation_internal(p, other, resolution);
 }


void setup_as_approximation(Hierarchy h, double resolution) {
  setup_as_approximation_internal(h, get_leaves(h), resolution);
}


void transform(Hierarchy h, const algebra::Transformation3D &tr) {
  base::Vector<Hierarchy> stack;
  stack.push_back(h);
  do {
    Hierarchy c= stack.back();
    stack.pop_back();
    if (core::RigidBody::particle_is_instance(c)) {
      core::transform(core::RigidBody(c), tr);
    } else if (core::XYZ::particle_is_instance(c)) {
      core::transform(core::XYZ(c), tr);
    }
    for (unsigned int i=0; i< c.get_number_of_children(); ++i) {
      stack.push_back(c.get_child(i));
    }
  } while (!stack.empty());
}


double get_mass(const Selection& h) {
  IMP_FUNCTION_LOG;
  double ret=0;
  ParticlesTemp ps=h.get_selected_particles();
  for (unsigned int i=0; i< ps.size(); ++i) {
    ret+= Mass(ps[i]).get_mass();
  }
  return ret;
}


#ifdef IMP_ALGEBRA_USE_IMP_CGAL

namespace {
  algebra::Sphere3Ds get_representation(Selection h) {
    ParticlesTemp leaves=h.get_selected_particles();
    algebra::Sphere3Ds ret(leaves.size());
    for (unsigned int i=0; i< leaves.size(); ++i) {
      ret[i] = core::XYZR(leaves[i]).get_sphere();
    }
    return ret;
  }
}

double get_volume(const Selection& h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return algebra::get_surface_area_and_volume(get_representation(h)).second;
}

double get_surface_area(const Selection& h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return algebra::get_surface_area_and_volume(get_representation(h)).first;
}
#endif

double get_radius_of_gyration(const Selection& h) {
  IMP_FUNCTION_LOG;
  IMP_USAGE_CHECK(!h.get_selected_particles().empty(),
                  "No particles selected.");
  return get_radius_of_gyration(h.get_selected_particles());
}

HierarchyTree get_hierarchy_tree(Hierarchy h) {
  HierarchyTree ret;
  typedef boost::property_map<HierarchyTree,
                      boost::vertex_name_t>::type VM;
  VM vm= boost::get(boost::vertex_name, ret);
  base::Vector<std::pair<int, Hierarchy> > queue;
  int v= boost::add_vertex(ret);
  vm[v]= h;
  queue.push_back(std::make_pair(v, h));
  do {
    int v= queue.back().first;
    Hierarchy c= queue.back().second;
    queue.pop_back();
    for (unsigned int i=0; i< c.get_number_of_children(); ++i) {
      int vc= boost::add_vertex(ret);
      vm[vc]= c.get_child(i);
      boost::add_edge(v, vc, ret);
      queue.push_back(std::make_pair(vc, c.get_child(i)));
    }
  } while (!queue.empty());
  return ret;
}



display::Geometries SelectionGeometry::get_components() const {
    display::Geometries ret;
    ParticlesTemp ps= res_.get_selected_particles();
    for (unsigned int i=0; i< ps.size(); ++i) {
      if (components_.find(ps[i]) == components_.end()) {
        IMP_NEW(HierarchyGeometry, g, (atom::Hierarchy(ps[i])));
        components_[ps[i]]= g;
        g->set_name(get_name());
        if (get_has_color()) {
          components_[ps[i]]->set_color(get_color());
        }
      }
      ret.push_back(components_.find(ps[i])->second);
    }
    return ret;
  }


Hierarchies get_leaves(const Selection &h) {
  Hierarchies ret;
  ParticlesTemp ps= h.get_selected_particles();
  for (unsigned int i=0; i< ps.size(); ++i) {
    ret+=get_leaves(Hierarchy(ps[i]));
  }
  return ret;
}
IMPATOM_END_NAMESPACE
