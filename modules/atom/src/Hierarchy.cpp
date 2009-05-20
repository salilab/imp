/**
 *  \file Hierarchy.cpp   \brief Decorator for helping deal
 *                                                 with a hierarchy.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Domain.h>
#include <IMP/core/LeavesRefiner.h>
#include <IMP/core/XYZR.h>
#include <IMP/atom/estimates.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/core/internal/Grid3D.h>

#include <boost/random/uniform_int.hpp>

#include <sstream>
#include <set>


IMPATOM_BEGIN_NAMESPACE

const IMP::core::HierarchyTraits&
Hierarchy::get_traits() {
  static IMP::core::HierarchyTraits ret("molecular_hierarchy");
  return ret;
}

IntKey Hierarchy::get_type_key() {
  static IntKey k("molecular_hierarchy_type");
  return k;
}

void Hierarchy::show(std::ostream &out,
                     std::string prefix) const
{
  if (*this == Hierarchy()) {
    out << "NULL Molecular Hierarchy node";
    return;
  }
  if (get_type() == ATOM && Atom::is_instance_of(get_particle())) {
    Atom ad(get_particle());
    ad.show(out, prefix);
  } else if ((get_type() == RESIDUE || get_type() == NUCLEICACID)
             && Residue::is_instance_of(get_particle())){
      Residue adt(get_particle());
      adt.show(out, prefix);
  } else if (get_type() == CHAIN
             && Chain::is_instance_of(get_particle())){
      Chain adt(get_particle());
      adt.show(out, prefix);
  } else if (Domain::is_instance_of(get_particle())) {
    Domain dd(get_particle());
    dd.show(out, prefix);
  } else {
    out << prefix << get_type_string() <<std::endl;
    out << prefix << "\"" <<  get_particle()->get_name() << "\"" << std::endl;
  }
}



namespace
{

struct MHDMatchingType
{
  MHDMatchingType(Hierarchy::Type t): t_(t){}

  bool operator()(Particle *p) const {
    Hierarchy mhd= Hierarchy::cast(p);
    if (mhd== Hierarchy()) {
      return false;
    } else {
      return mhd.get_type()==t_;
    }
  }

  Hierarchy::Type t_;
};

} // namespace

Particles get_by_type(Hierarchy mhd,
                      Hierarchy::Type t)
{
  Particles out;
  gather(mhd, MHDMatchingType(t),
         std::back_inserter(out));
  return out;
}


namespace
{

struct MatchResidueIndex
{
  int index_;
  MatchResidueIndex(int i): index_(i) {}
  bool operator()(Particle *p) const {
    Hierarchy mhd(p);
    if (mhd.get_type() == Hierarchy::RESIDUE
        || mhd.get_type() == Hierarchy::NUCLEICACID) {
      Residue rd(p);
      return (rd.get_index() == index_);
    } else {
      if (mhd.get_number_of_children()==0) {
        Domain dd= Domain::cast(p);
        return dd && dd.get_begin_index() <= index_
          && dd.get_end_index()> index_;
      } else {
        return false;
      }
    }
  }
};

} // namespace


Hierarchy
get_residue(Hierarchy mhd,
            unsigned int index)
{
  IMP_check(mhd.get_type() == Hierarchy::PROTEIN
            || mhd.get_type() == Hierarchy::CHAIN
            || mhd.get_type() == Hierarchy::NUCLEOTIDE,
            "Invalid type of Hierarchy passed to get_residue",
            ValueException);
  MatchResidueIndex mi(index);
  IMP::core::Hierarchy hd= breadth_first_find(mhd, mi);
  if (hd== IMP::core::Hierarchy()) {
    return Hierarchy();
  } else {
    return Hierarchy(hd.get_particle());
  }
}



Hierarchy
create_fragment(const Hierarchys &ps)
{
  IMP_check(!ps.empty(), "Need some particles",
            ValueException);
  Hierarchy parent= ps[0].get_parent();
  unsigned int index= ps[0].get_parent_index();
  IMP_IF_CHECK(CHEAP) {
    for (unsigned int i=0; i< ps.size(); ++i) {
      IMP_check(ps[i].get_parent() == parent,
                "Parents don't match",
                ValueException);
    }
  }

  Particle *fp= new Particle(parent.get_particle()->get_model());
  Hierarchy fd= Hierarchy::create(fp,
                                       Hierarchy::FRAGMENT);

  for (unsigned int i=0; i< ps.size(); ++i) {
    parent.remove_child(ps[i]);
    fd.add_child(ps[i]);
  }

  parent.add_child_at(fd, index);
  return fd;
}

Bonds get_internal_bonds(Hierarchy mhd)
{
  Particles ps= get_all_descendants(mhd);
  std::set<Particle*> sps(ps.begin(), ps.end());
  Bonds ret;
  for (Particles::iterator pit = ps.begin(); pit != ps.end(); ++pit) {
    Particle *p = *pit;
    if (Bonded::is_instance_of(p)) {
      Bonded b(p);
      for (unsigned int i=0; i< b.get_number_of_bonds(); ++i) {
        Particle *op = b.get_bonded(i).get_particle();
        if (op < p && sps.find(op) != sps.end()) {
          ret.push_back(b.get_bond(i));
        }
      }
    }
  }
  return ret;
}

namespace {
IMPATOMEXPORT
Hierarchy clone_internal(Hierarchy d,
                                           std::map<Particle*,
                                           Particle*> &map) {
  Particle *p= new Particle(d.get_model());
  map[d.get_particle()]=p;
  Hierarchy nd;
  if (Atom::is_instance_of(d.get_particle())) {
    nd= Atom::create(p, Atom(d.get_particle()));
  } else if (Residue::is_instance_of(d.get_particle())) {
    nd= Residue::create(p, Residue(d.get_particle()));
  } else if (Domain::is_instance_of(d.get_particle())) {
    nd= Domain::create(p, Domain(d.get_particle()));
  } else if (Chain::is_instance_of(d.get_particle())) {
    nd= Chain::create(p, Chain(d.get_particle()));
  } else {
    nd=Hierarchy::create(p, d.get_type());
  }
  using core::XYZ;
  using core::XYZR;
  if (XYZR::is_instance_of(d.get_particle())){
    XYZR::create(p,
        algebra::Sphere3D(XYZ(d.get_particle()).get_coordinates(),
                          XYZR(d.get_particle()).get_radius()));
  } else if (XYZ::is_instance_of(d.get_particle())) {
    XYZ::create(p,
                         XYZ(d.get_particle()).get_coordinates());
  }
  p->set_name(d.get_particle()->get_name());
  for (unsigned int i=0 ;i< d.get_number_of_children(); ++i) {
    Hierarchy nc= clone_internal(d.get_child(i), map);
    nd.add_child(nc);
  }
  return nd;
}
}


Hierarchy clone(Hierarchy d) {
  std::map<Particle*,Particle*> map;
  Hierarchy nh= clone_internal(d, map);
  Bonds bds= get_internal_bonds(d);
  for (unsigned int i=0; i< bds.size(); ++i) {
    Bonded e0= bds[i].get_bonded(0);
    Bonded e1= bds[i].get_bonded(1);
    Particle *np0= map[e0.get_particle()];
    Particle *np1= map[e1.get_particle()];
    Bonded ne0, ne1;
    if (Bonded::is_instance_of(np0)) {
      ne0=Bonded(np0);
    } else {
      ne0=Bonded::create(np0);
    }
    if (Bonded::is_instance_of(np1)) {
      ne1=Bonded(np1);
    } else {
      ne1=Bonded::create(np1);
    }
    copy_bond(ne0, ne1, bds[i]);
  }
  return nh;
}

/*
  Volume of two spheres overlap is
  Vi= pi*(r0+r1-d)^2*(d^2+2*d*r1-3*r1^2+2*d*r0+6*r0*r1-3*r0^2)/(12*d)

  r1=r0=r
  d=(1-f)*2*r
  v=4/3pir^3*n-(n-1)Vi

  n=.5*(3*V+2*PI*r^3*f^3-6*PI*r^3*f^2)/((-3*f^2+f^3+2)*r^3*PI)
 */

IMPATOMEXPORT Particles
get_detailed_representation(Hierarchy hi){
  Particles ret;
  std::vector<Hierarchy> front;
  front.push_back(hi);
  do {
    Hierarchy h= front.back();
    front.pop_back();
    //IMP_LOG(VERBOSE, "Trying " << h << std::endl);
    if (h.get_number_of_children() > 0
        && core::XYZ::is_instance_of(h.get_child(0).get_particle())) {
      front.insert(front.end(),
                   h.children_begin(),
                   h.children_end());
    } else if (core::XYZ::is_instance_of(h.get_particle())) {
      ret.push_back(h.get_particle());
    } else {
       front.insert(front.end(),
                   h.children_begin(),
                   h.children_end());
    }
  } while (!front.empty());
  IMP_LOG(VERBOSE, "Detailed representation has size " << ret.size()
          << std::endl);
  return ret;
}

IMPATOMEXPORT Particles
get_simplified_representation(Hierarchy h) {
  Particles ret;
  std::vector<Hierarchy> front;
  front.push_back(h);
  do {
    Hierarchy h= front.back();
    front.pop_back();
    //IMP_LOG(VERBOSE, "Trying " << h << std::endl);
    if (core::XYZ::is_instance_of(h.get_particle())) {
      ret.push_back(h.get_particle());
    } else {
      front.insert(front.end(),
                   h.children_begin(),
                   h.children_end());
    }
  } while (!front.empty());
  return ret;
}

algebra::BoundingBox3D bounding_box(const Hierarchy &h,
                                    FloatKey r) {
  Particles rep= get_simplified_representation(h);
  algebra::BoundingBox3D bb;
  for (unsigned int i=0; i< rep.size(); ++i) {
    core::XYZR xyzr= core::XYZR::cast(rep[i], r);
    if (xyzr) {
      bb+= algebra::bounding_box(xyzr.get_sphere());
    } else if (core::XYZ::is_instance_of(rep[i])) {
      bb+= algebra::BoundingBox3D(core::XYZ(rep[i]).get_coordinates());
    }
  }
  IMP_LOG(VERBOSE, "Bounding box is " << bb << std::endl);
  return bb;
}


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

IMPATOMEXPORT Restraint* create_protein(Particle *p,
                                        double resolution,
                                        int number_of_residues,
                                        int first_residue_index,
                                        double volume,
                                        double spring_strength) {
  if (volume < 0) {
    double mass= mass_in_kDa_from_number_of_residues(number_of_residues);
    volume= volume_from_mass_in_kDa(mass);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac=.2;
  std::pair<int, double> nr= compute_n(volume, resolution, overlap_frac);
  Hierarchy pd
    =Hierarchy::create(p,
                              Hierarchy::PROTEIN);
  Particles ps;
  for (int i=0; i< nr.first; ++i) {
    Particle *pc= new Particle(p->get_model());
    Hierarchy pcd
      =Hierarchy::create(pc,
                              Hierarchy::FRAGMENT);
    pd.add_child(pcd);
    core::XYZR xyzd=core::XYZR::create(pc);
    xyzd.set_radius(nr.second);
    xyzd.set_coordinates_are_optimized(true);
    ps.push_back(pc);
  }
  IMP_NEW(core::Harmonic, h, ((1-overlap_frac)*2*nr.second, spring_strength));
  IMP_NEW(core::DistancePairScore, dps, (h));
  core::ConnectivityRestraint* cr= new core::ConnectivityRestraint(dps);
  cr->set_particles(ps);
  return cr;
}


namespace {
  typedef std::vector<algebra::Sphere3D> Spheres;
  typedef std::vector<algebra::Vector3D> Vectors;
  typedef std::vector<unsigned int> Indexes;


  double cell_size(double resolution) {
    return resolution/6;
  }

  algebra::Sphere3D get_sphere(Particle *p) {
    if (core::XYZR::is_instance_of(p)) {
      return core::XYZR(p).get_sphere();
    } else {
      return algebra::Sphere3D(core::XYZ(p).get_coordinates(),
                               1);
    }
  }


  void teleport(Spheres &centers, double resolution) {
    double minr= std::numeric_limits<double>::max();
      double maxr=-minr;
      int minc=-1;
      int maxc=-1;
      for (unsigned int i=0; i< centers.size(); ++i) {
        if (centers[i].get_radius() < minr) {
          minr= centers[i].get_radius();
          minc= i;
        }
        if (centers[i].get_radius() > maxr) {
          maxr= centers[i].get_radius();
          maxc=i;
        }
      }
      IMP_LOG(VERBOSE, "Teleporting " << centers[minc] << " to ");
      // move minr to near maxr
      algebra::Vector3D offset(0,0, cell_size(resolution));
      centers[minc]=algebra::Sphere3D(centers[maxc].get_center()
                                      +offset,
                                      centers[maxc].get_radius());
      centers[maxc]=algebra::Sphere3D(centers[maxc].get_center()
                                      -offset,
                                      centers[maxc].get_radius());
      IMP_LOG(VERBOSE, centers[minc] << std::endl);
  }

  void rasterize_io(Hierarchy in, double res,
                    Vectors &inside,
                    Vectors &outside) {

    algebra::BoundingBox3D bb= bounding_box(in);
    bb+= res;
    typedef IMP::core::internal::Grid3D<bool> Grid;
    Grid grid(cell_size(res), bb.get_corner(0), bb.get_corner(1),
              false);
    Particles leaves= get_detailed_representation(in);
    algebra::Vector3D ones(1,1,1);
    for (unsigned int i=0; i< leaves.size(); ++i) {
      algebra::Sphere3D s= get_sphere(leaves[i]);
      //s= algebra::Sphere3D(s.get_center(), s.get_radius());
      algebra::Vector3D lb= s.get_center()-s.get_radius()*ones;
      algebra::Vector3D ub= s.get_center()+s.get_radius()*ones;
      Grid::VirtualIndex lbi= grid.get_virtual_index(lb);
      Grid::VirtualIndex ubt=grid.get_virtual_index(ub);
      Grid::VirtualIndex ubi(ubt[0]+1, ubt[1]+1, ubt[2]+1);
      for (Grid::IndexIterator it= grid.indexes_begin(lbi, ubi);
           it != grid.indexes_end(lbi, ubi); ++it) {
        if (!grid.get_voxel(*it)) {
          algebra::Vector3D center= grid.get_center(*it);
          if (s.get_contains(center)) {
            grid.get_voxel(*it)=true;
          }
        }
      }
      grid.get_voxel(grid.get_index(s.get_center()))=true;
    }
    for (Grid::IndexIterator it= grid.all_indexes_begin();
           it != grid.all_indexes_end(); ++it) {
      if (grid.get_voxel(*it)) {
        inside.push_back(grid.get_center(*it));
      } else {
        outside.push_back(grid.get_center(*it));
      }
    }
  }

  void sort_outside(const Vectors &outside,
                    const Spheres &centers,
                    std::vector< std::vector<float> > &dists) {
    for (unsigned int i=0; i< centers.size(); ++i) {
      dists[i].resize(outside.size());
      for (unsigned int j=0; j< outside.size(); ++j) {
        dists[i][j]= distance(outside[j], centers[i].get_center());
      }
      std::sort(dists[i].begin(), dists[i].end());
      /*IMP_LOG(VERBOSE, "Sphere " << i << " has lb of " << dists[i].front()
        << " and ub of " << dists[i].back() << std::endl);*/
    }
  }

  int assign(const std::vector< std::vector<float> > &dists,
             const Spheres &centers,
             const Indexes &offsets,
             const algebra::Vector3D &v) {
    double best_score=std::numeric_limits<double>::max();
    int best_center=-1;
    unsigned int best_offset=-1;
    for (unsigned int j=0; j< centers.size(); ++j) {
      float dist = distance(v, centers[j].get_center());
      std::vector<float>::const_iterator it= std::upper_bound(dists[j].begin(),
                                                              dists[j].end(),
                                                              dist);
      /*IMP_LOG(VERBOSE, "Point " << i << " center " << j
        << " dist " << dist << " bound " << *it << std::endl);*/
      int score;
      /*if (it -dists[j].begin() < offsets[j]) {
        score=0;
        } else {*/
      score= std::distance(dists[j].begin(), it);
      if (score < offsets[j]) score=0;
      else score= score-offsets[j];
      //}
      if (score < best_score) {
        best_score=score;
        best_center=j;
        best_offset= score;
      }
    }
    return best_center;
  }


  void compute_assignments(const Vectors &inside,
                           const std::vector< std::vector<float> > &dists,
                           const Spheres &centers,
                           Indexes &offsets,
                           Indexes &assignments) {
     for (unsigned int i=0; i< inside.size(); ++i) {
       assignments[i]= assign(dists, centers, offsets, inside[i]);
       float dist= distance(centers[assignments[i]].get_center(), inside[i]);
       unsigned int score= std::upper_bound(dists[assignments[i]].begin(),
                                            dists[assignments[i]].end(),
                                            dist)
         - dists[assignments[i]].begin();
       offsets[assignments[i]]= std::max(offsets[assignments[i]], score);
    }
  }
  void recompute_centers(const Vectors &inside, const Indexes &assignments,
                         double resolution,
                         Spheres &centers) {
    boost::uniform_int<int> r(0, inside.size()-1);
    for (unsigned int i=0; i< centers.size(); ++i) {
      algebra::Sphere3Ds cur;
      for (unsigned int j=0; j< inside.size(); ++j) {
        if (assignments[j]==i) {
          cur.push_back(algebra::Sphere3D(inside[j], 0));
        }
      }
      if (cur.empty()) {
        int index= r(random_number_generator);
        centers[i]= algebra::Sphere3D(inside[index],
                                      0);
      } else if (cur.size() ==1) {
        centers[i]= algebra::Sphere3D(cur[0].get_center(),
                                      cell_size(resolution));
      } else {
        centers[i]= enclosing_sphere(cur);
      }
    }
  }

  void write_spheres(Spheres &centers,
                     unsigned int index) {
    std::ostringstream oss;
    oss << "Spheres." << index << ".bild";
    std::ofstream out(oss.str().c_str());
    for (unsigned int i=0; i< centers.size(); ++i) {
      out << ".sphere " << spaces_io(centers[i].get_center())
          << " " << centers[i].get_radius() << std::endl;
    }
  }

  void write_outside(const Vectors &vs) {
    std::ostringstream oss;
    oss << "Outside"
        << ".bild";
    std::ofstream out(oss.str().c_str());
    out << ".color 10\n";
    for (unsigned int i=0; i< vs.size(); ++i) {
      out << ".sphere " << spaces_io(vs[i])
          << " " << .5 << std::endl;
    }
  }

  void write_assignments(const Vectors &inside,
                         const Indexes &assignments,
                         unsigned int index) {
    std::ostringstream oss;
    oss << "Assignments." << index << ".bild";
    std::ofstream out(oss.str().c_str());
    for (unsigned int i=0; i< assignments.size(); ++i) {
      out << ".color " << assignments[i]*5 << std::endl;
      out << ".sphere " << spaces_io(inside[i])
          << " " << .5 << std::endl;
    }
  }

  double compute_score(const Spheres &centers,
                       const std::vector<std::vector<float> > &dists) {
    double sum=0;
    for (unsigned int i=0; i< centers.size(); ++i) {
      double score= std::upper_bound(dists[i].begin(),
                                     dists[i].end(),
                                     centers[i].get_radius())-dists[i].begin();
      sum+= score;
    }
    return sum;
  }
}

/** Simplify a molecular hierarchy to be represented by fewer
    balls.
    given current center locations, perform iterations of:
    - assign each point in order of distance to the nearest center to the
    center where addition minimizes outside volume (nkE)
    - recenter centers (nk)
    - if no improvement, move a center to split the largest sphere

    To measure error:
    - sort all outside points based on all their distances to each center
      (knlogn)
    - see how many new ones are needed, E=(1)

    Then:
    - take each amino acid, figure out which sphere it is closest to
    add it as a child of that sphere with no coordinates
 */
Hierarchy simplify_protein(Hierarchy in,
                           double resolution) {
  // compute outside points
  Vectors outside;
  Vectors inside;
  IMP_LOG(VERBOSE, "Rasterizing protein " << std::flush);
  rasterize_io(in, resolution, inside, outside);
  //write_outside(outside);
  IMP_LOG(VERBOSE, inside.size() << " inside and " << outside.size()
          << " outside " << std::endl);
  std::random_shuffle(inside.begin(), inside.end());
  // compute initial number of centers
  int num_centers=inside.size()/40;
  IMP_LOG(VERBOSE, num_centers << " centers" << std::endl);

  // produce random initial centers
  Spheres centers(num_centers);
  boost::uniform_int<int> r(0, inside.size()-1);
  for (unsigned int i=0; i< centers.size(); ++i) {
    int index= r(random_number_generator);
    centers[i]= algebra::Sphere3D(inside[index], 10000);
  }
  //write_spheres(centers, 0);
  Indexes assignments(inside.size());
  double old_score= std::numeric_limits<double>::max();
  std::vector< std::vector<float> > dists(centers.size());
  for (unsigned int i=0; i<100; ++i) {
  // generate sorted lists of distance

    IMP_LOG(VERBOSE, "Sorting outside " << std::endl);
    sort_outside(outside, centers, dists);
    Indexes offsets(centers.size());
    double new_score= compute_score(centers, dists);
    IMP_LOG(VERBOSE, "Score is " << new_score << std::endl);

    if (new_score==0) break;
    if (old_score <= new_score) {
      IMP_LOG(VERBOSE, "teleporting " << std::endl);
      teleport(centers, resolution);
      sort_outside(outside, centers, dists);
    }
    old_score=new_score;
    IMP_LOG(VERBOSE, "Computing assignments " << std::endl);
    compute_assignments(inside, dists, centers, offsets,  assignments);
    //write_assignments(inside, assignments, i);
    IMP_LOG(VERBOSE, "Recomputing centers " << std::endl);
    recompute_centers(inside, assignments, resolution, centers);
    //write_spheres(centers, i+1);
  }

  IMP_NEW(Particle, root, (in.get_particle()->get_model()));
  Hierarchy h= Hierarchy::create(root, in.get_type());
  for (unsigned int i=0; i< centers.size(); ++i) {
    IMP_NEW(Particle, c, (in.get_particle()->get_model()));
    Hierarchy hc= Hierarchy::create(c, Hierarchy::FRAGMENT);
    h.add_child(hc);
    core::XYZR::create(c, centers[i]);
  }

  Particles l= get_detailed_representation(in);
  for (unsigned int j=0; j < l.size(); ++j) {
    Particle *rp=NULL;
    Atom ad= Atom::cast(l[j]);
    if (ad && ad.get_atom_type() == AT_CA) {
      rp= get_residue(Atom(l[j])).get_particle();
    } else if (Residue::is_instance_of(l[j])) {
      rp=l[j];
    }
    if (rp) {
      IMP_NEW(Particle, res, (rp->get_model()));
      Residue residue= Residue::create(res, Residue(rp));
      double close_distance=10000;
      int close_center=-1;
      for (unsigned int i=0; i< centers.size(); ++i) {
        algebra::Sphere3D s= get_sphere(l[j]);
        double d= algebra::distance(s, centers[i]);
        if (d < close_distance) {
          close_distance=d;
          close_center=i;
        }
      }
      h.get_child(close_center).add_child(residue);
    }
  }
  //write_spheres(centers, 1000);

  return h;
}


IMPATOM_END_NAMESPACE
