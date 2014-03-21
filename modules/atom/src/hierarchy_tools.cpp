/**
 *  \file atom/hierarchy_tools.cpp
 *  \brief A decorator for a point particle that has an electrostatic charge.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/atom/hierarchy_tools.h"
#include <IMP/SingletonContainer.h>
#include <IMP/algebra/Sphere3D.h>
#include <IMP/algebra/vector_generators.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Copy.h>
#include <IMP/atom/Domain.h>
#include <IMP/atom/Fragment.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Molecule.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/bond_decorators.h>
#include <IMP/atom/distance.h>
#include <IMP/atom/estimates.h>
#include <IMP/base/set_map_macros.h>
#include <IMP/kernel/constants.h>
#include <IMP/container/AllBipartitePairContainer.h>
#include <IMP/container/ConnectingPairContainer.h>
#include <IMP/container/ListSingletonContainer.h>
#include <IMP/container/generic.h>
#include <IMP/core/ClosePairsPairScore.h>
#include <IMP/core/ConnectivityRestraint.h>
#include <IMP/core/CoverRefined.h>
#include <IMP/core/DistancePairScore.h>
#include <IMP/core/ExcludedVolumeRestraint.h>
#include <IMP/core/Harmonic.h>
#include <IMP/core/PairRestraint.h>
#include <IMP/core/SphereDistancePairScore.h>
#include <IMP/core/TableRefiner.h>
#include <IMP/core/rigid_bodies.h>
#include <IMP/kernel/generic.h>
#include <algorithm>
IMPATOM_BEGIN_NAMESPACE

namespace {
std::pair<int, double> compute_n(double V, double r, double f) {
  double n =
      /*.5*(3*V+2*PI*cube(r*f)-6*PI*cube(r)*square(f))
        /((-3*square(f)+cube(f)+2)*cube(r)*PI);*/
      V / (4.0 / 3.0 * PI * cube(r));
  int in = static_cast<int>(std::ceil(n));
  double rr =
      /*std::pow(V/(.666*(2*in-3*square(f)*n+cube(f)*n
        +3*square(f)-cube(f))*PI), .333333);*/
      std::pow(V / (in * 4.0 / 3.0 * PI) / (1 - f), .3333);
  return std::make_pair(in, rr);
}

/*
// ignores overlap
double get_volume(Hierarchy h) {
  if (core::XYZR::get_is_setup(h)) {
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
  if (Residue::get_is_setup(h)) {
    int i = Residue(h).get_index();
    inds.push_back(i);
  } else if (Fragment::get_is_setup(h) && h.get_number_of_children() != 0) {
    Ints v = Fragment(h).get_residue_indexes();
    inds.insert(inds.end(), v.begin(), v.end());
  } else if (Domain::get_is_setup(h) && h.get_number_of_children() == 0) {
    Domain d(h);
    IntRange ir = d.get_index_range();
    for (int i = ir.first; i != ir.second; ++i) {
      inds.push_back(i);
    }
  } else if (Atom::get_is_setup(h)) {
    Residue r = get_residue(Atom(h));
    inds.push_back(r.get_index());
  } else {
    for (unsigned int i = 0; i < h.get_number_of_children(); ++i) {
      gather_residue_indices(h.get_child(i), inds);
    }
  }
}

double get_volume_measurement(algebra::Sphere3Ds ss, double resolution) {
#ifdef IMP_ATOM_USE_IMP_CGAL
  // smooth
  for (unsigned int i = 0; i < ss.size(); ++i) {
    ss[i] =
        algebra::Sphere3D(ss[i].get_center(), ss[i].get_radius() + resolution);
  }
  double v = algebra::get_surface_area_and_volume(ss).second;
  return 4.0 / 3.0 * PI *
         cube(algebra::get_ball_radius_from_volume_3d(v) - resolution);
#else
  IMP_UNUSED(resolution);
  double v = 0;
  for (unsigned int i = 0; i < ss.size(); ++i) {
    v += algebra::get_volume(ss[i]);
  }
  return v;
#endif
}

void setup_as_approximation_internal(kernel::Particle *p,
                                     const kernel::ParticlesTemp &other,
                                     double resolution = -1, double volume = -1,
                                     double mass = -1) {
  IMP_USAGE_CHECK(volume == -1 || volume > 0,
                  "Volume must be positive if specified");
  IMP_USAGE_CHECK(other.size() > 0, "Must pass particles to approximate");
  double m = 0;
  algebra::Sphere3Ds ss;
  Ints inds;
  algebra::Vector3D vv(0, 0, 0);
  for (unsigned int i = 0; i < other.size(); ++i) {
    m += get_mass(Selection(Hierarchy(other[i])));
    gather_residue_indices(Hierarchy(other[i]), inds);
    core::XYZR d(other[i]);
    if (volume < 0) {
      ss.push_back(d.get_sphere());
    }
    vv += d.get_coordinates();
  }
  if (resolution < 0) {
    algebra::BoundingBox3D bb;
    for (unsigned int i = 0; i < ss.size(); ++i) {
      bb += get_bounding_box(ss[i]);
    }
    resolution = (bb.get_corner(0) - bb.get_corner(1)).get_magnitude() / 2.0;
  }
  if (mass >= 0) {
    m = mass;
  }
  if (!Residue::get_is_setup(p) && !Fragment::get_is_setup(p) &&
      !Domain::get_is_setup(p)) {
    Fragment f = Fragment::setup_particle(p);
    f.set_residue_indexes(inds);
  }
  if (!Mass::get_is_setup(p)) {
    Mass::setup_particle(p, m);
  } else {
    Mass(p).set_mass(m);
  }
  algebra::Sphere3D s;
  algebra::Vector3D center = vv / other.size();
  if (volume >= 0) {
    s = algebra::Sphere3D(center,
                          algebra::get_ball_radius_from_volume_3d(volume));
  } else {
    // IMP_LOG_VERBOSE( "Approximating volume." << std::endl);
    double v = get_volume_measurement(ss, resolution);
    s = algebra::Sphere3D(center, algebra::get_ball_radius_from_volume_3d(v));
  }

  if (core::XYZR::get_is_setup(p)) {
    core::XYZR(p).set_sphere(s);
  } else {
    core::XYZR::setup_particle(p, s);
  }
}
}

Hierarchy create_protein(kernel::Model *m, std::string name, double resolution,
                         int number_of_residues, int first_residue_index,
                         double volume, bool ismol) {
  double mass =
      atom::get_mass_from_number_of_residues(number_of_residues) / 1000;
  if (volume < 0) {
    volume = atom::get_volume_from_mass(mass * 1000);
  }
  // assume a 20% overlap in the beads to make the protein not too bumpy
  double overlap_frac = .2;
  std::pair<int, double> nr = compute_n(volume, resolution, overlap_frac);
  Hierarchy pd = Hierarchy::setup_particle(new kernel::Particle(m));
  Ints residues;
  for (int i = 0; i < number_of_residues; ++i) {
    residues.push_back(i + first_residue_index);
  }
  atom::Fragment::setup_particle(pd, residues);
  if (ismol) Molecule::setup_particle(pd);
  pd->set_name(name);
  for (int i = 0; i < nr.first; ++i) {
    kernel::Particle *pc;
    if (nr.first > 1) {
      pc = new kernel::Particle(m);
      std::ostringstream oss;
      oss << name << "-" << i;
      pc->set_name(oss.str());
      atom::Fragment pcd = atom::Fragment::setup_particle(pc);
      Ints indexes;
      for (int j = i * (number_of_residues / nr.first) + first_residue_index;
           j < (i + 1) * (number_of_residues / nr.first) + first_residue_index;
           ++j) {
        indexes.push_back(j);
      }
      pcd.set_residue_indexes(indexes);
      pd.add_child(pcd);
    } else {
      pc = pd;
    }

    core::XYZR xyzd = core::XYZR::setup_particle(pc);
    xyzd.set_radius(nr.second);
    atom::Mass::setup_particle(pc, mass / nr.first);
  }
  IMP_INTERNAL_CHECK(pd.get_is_valid(true), "Invalid hierarchy produced "
                                                << pd);
  return pd;
}

Hierarchy create_protein(kernel::Model *m, std::string name, double resolution,
                         const Ints db) {
  Hierarchy root = Hierarchy::setup_particle(new kernel::Particle(m));
  Domain::setup_particle(root, IntRange(db.front(), db.back()));
  for (unsigned int i = 1; i < db.size(); ++i) {
    std::ostringstream oss;
    oss << name << "-" << i - 1;
    Hierarchy cur = create_protein(
        m, oss.str(), resolution, db[i] - db[i - 1], db[i - 1],
        atom::get_volume_from_mass(
            atom::get_mass_from_number_of_residues(db[i] - db[i - 1])),
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
  static base::WarningContext wc;
  IMP_IF_CHECK(USAGE) {
    for (unsigned int i = 0; i < t.size(); ++i) {
      IMP_USAGE_CHECK(Residue::get_is_setup(t[i]),
                      "The residue is not a residue, it is " << t[i]);
    }
  }
  if (t.empty()) {
    return Hierarchy();
  }
  double v = 0;
  for (unsigned int i = 0; i < t.size(); ++i) {
    ResidueType rt = Residue(t[i]).get_residue_type();
    try {
      v += get_volume_from_residue_type(rt);
    }
    catch (base::ValueException) {
      IMP_WARN_ONCE(rt.get_string(),
                    "Computing volume for non-standard residue " << rt, wc);
      algebra::Sphere3Ds ss;
      Hierarchies gl = get_leaves(t[i]);
      for (unsigned int i = 0; i < gl.size(); ++i) {
        ss.push_back(core::XYZR(gl[i]).get_sphere());
      }
      v += get_volume_measurement(ss, 5.0);
    }
  }
  kernel::Model *mm = t[0]->get_model();
  kernel::Particle *p = new kernel::Particle(mm);
  kernel::ParticlesTemp children;
  for (unsigned int i = 0; i < t.size(); ++i) {
    Hierarchies cur = t[i].get_children();
    children.insert(children.end(), cur.begin(), cur.end());
  }
  setup_as_approximation_internal(p, children, -1, v);
  std::ostringstream oss;
  Ints rids = Fragment(p).get_residue_indexes();
  std::sort(rids.begin(), rids.end());
  oss << "Fragment";
  if (!rids.empty()) {
    oss << " [" << rids.front() << "-" << rids.back() + 1 << ")";
  }
  p->set_name(oss.str());
  wc.dump_warnings();
  return Hierarchy(p);
}
}

namespace {
Hierarchy create_simplified_along_backbone(Chain in, int num_res,
                                           bool keep_detailed) {
  if (in.get_number_of_children() == 0) {
    return Hierarchy();
  }
  IntRanges rs;
  int ib = Residue(in.get_child(0)).get_index();
  int ie =
      Residue(in.get_child(in.get_number_of_children() - 1)).get_index() + 1;
  int i;
  for (i = ib; i < ie; i += num_res) {
    rs.push_back(IntRange(i, std::min(i + num_res, ie)));
  }
  return create_simplified_along_backbone(in, rs, keep_detailed);
}
}

Hierarchy create_simplified_along_backbone(Hierarchy in, int num_res,
                                           bool keep_detailed) {
  Hierarchies chains = get_by_type(in, CHAIN_TYPE);
  if (chains.size() > 1) {
    Hierarchy root = Hierarchy::setup_particle(
        new kernel::Particle(in->get_model(), in->get_name()));
    for (unsigned int i = 0; i < chains.size(); ++i) {
      Chain chain(chains[i].get_particle());
      root.add_child(
          create_simplified_along_backbone(chain, num_res, keep_detailed));
    }
    return root;
  } else if (chains.size() == 1) {
    // make sure to cast it to chain to get the right overload
    return create_simplified_along_backbone(Chain(chains[0]), num_res,
                                            keep_detailed);
  } else {
    IMP_THROW("No chains to simplify", ValueException);
  }
}

Hierarchy create_simplified_along_backbone(Chain in,
                                           const IntRanges &residue_segments,
                                           bool keep_detailed) {
  IMP_USAGE_CHECK(in.get_is_valid(true), "Chain " << in << " is not valid.");
  if (in.get_number_of_children() == 0 || residue_segments.empty()) {
    IMP_LOG_TERSE("Nothing to simplify in "
                  << (in ? in->get_name() : "nullptr") << " with "
                  << residue_segments.size() << " segments.\n");
    return Hierarchy();
  }
  for (unsigned int i = 0; i < residue_segments.size(); ++i) {
    IMP_USAGE_CHECK(residue_segments[i].first < residue_segments[i].second,
                    "Residue intervals must be non-empty");
  }
  IMP_IF_LOG(VERBOSE) {
    for (unsigned int i = 0; i < residue_segments.size(); ++i) {
      IMP_LOG_VERBOSE("[" << residue_segments[i].first << "..."
                          << residue_segments[i].second << ") ");
    }
    IMP_LOG_VERBOSE(std::endl);
  }
  unsigned int cur_segment = 0;
  Hierarchies cur;
  Hierarchy root = create_clone_one(in);
  for (unsigned int i = 0; i < in.get_number_of_children(); ++i) {
    Hierarchy child = in.get_child(i);
    int index = Residue(child).get_index();
    IMP_LOG_VERBOSE("Processing residue "
                    << index << " with range "
                    << residue_segments[cur_segment].first << " "
                    << residue_segments[cur_segment].second << std::endl);
    if (index >= residue_segments[cur_segment].first &&
        index < residue_segments[cur_segment].second) {
    } else if (!cur.empty()) {
      IMP_LOG_VERBOSE("Added particle for "
                      << residue_segments[cur_segment].first << "..."
                      << residue_segments[cur_segment].second << std::endl);
      Hierarchy cur_approx = create_approximation_of_residues(cur);
      root.add_child(cur_approx);
      if (keep_detailed) {
        for (unsigned int j = 0; j < cur.size(); ++j) {
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
  kernel::ParticlesTemp rt= get_by_type(root, XYZR_TYPE);
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
  IMP_INTERNAL_CHECK(root.get_is_valid(true), "Invalid hierarchy produced "
                                                  << root);
  return root;
}

void setup_as_approximation(kernel::Particle *p,
                            const kernel::ParticlesTemp &other,
                            double resolution) {
  setup_as_approximation_internal(p, other, resolution);
}

void setup_as_approximation(Hierarchy h, double resolution) {
  setup_as_approximation_internal(h, get_leaves(h), resolution);
}

namespace {
void transform_impl(
    kernel::Model *m, kernel::ParticleIndex cur,
    const algebra::Transformation3D &tr,
    boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes> &
        rigid_bodies) {
  if (core::RigidBody::get_is_setup(m, cur)) {
    core::transform(core::RigidBody(m, cur), tr);
    return;
  }
  if (core::RigidBodyMember::get_is_setup(m, cur)) {
    kernel::ParticleIndex rb =
        core::RigidBodyMember(m, cur).get_rigid_body().get_particle_index();
    rigid_bodies[rb].push_back(cur);
  } else if (core::XYZ::get_is_setup(m, cur)) {
    core::transform(core::XYZ(m, cur), tr);
  }
  IMP_FOREACH(kernel::ParticleIndex pi,
              atom::Hierarchy(m, cur).get_children_indexes()) {
    transform_impl(m, pi, tr, rigid_bodies);
  }
}
}

void transform(atom::Hierarchy h, const algebra::Transformation3D &tr) {
  kernel::Model *m = h.get_model();
  typedef std::pair<kernel::ParticleIndex, kernel::ParticleIndexes> RBP;
  boost::unordered_map<kernel::ParticleIndex, kernel::ParticleIndexes>
      rigid_bodies;
  transform_impl(m, h.get_particle_index(), tr, rigid_bodies);
  IMP_FOREACH(const RBP & rbp, rigid_bodies) {
    core::RigidBody rb(m, rbp.first);
    kernel::ParticleIndexes members = rb.get_member_indexes();
    if (rbp.second.size() != members.size()) {
      IMP_USAGE_CHECK(
          rbp.second.size() == members.size(),
          "Hierarchy contains rigid body members of incomplete rigid bodies. "
          "It must contain all members of a rigid body or no members. Rigid "
          "body is "
              << m->get_particle_name(rbp.first));
    }
    core::transform(rb, tr);
  }
}

namespace {

void assign_residues(IMP::atom::Hierarchy in,
                     const IMP::atom::Hierarchies &out) {
  IMP_FUNCTION_LOG;
  IMP_BASE_LARGE_UNORDERED_MAP<IMP::atom::Hierarchy,
                               IMP_BASE_SMALL_UNORDERED_SET<int> > indexes;
  IMP_FOREACH(IMP::atom::Hierarchy l, IMP::atom::get_leaves(in)) {
    IMP::core::XYZR d(l);
    IMP::Ints cur;
    if (IMP::atom::Atom::get_is_setup(l) &&
        IMP::atom::Atom(l).get_atom_type() == IMP::atom::AT_CA) {
      l = l.get_parent();
    }

    if (IMP::atom::Residue::get_is_setup(l)) {
      cur.push_back(IMP::atom::Residue(l).get_index());
    } else if (IMP::atom::Fragment::get_is_setup(l)) {
      cur = IMP::atom::Fragment(l).get_residue_indexes();
    } else if (IMP::atom::Domain::get_is_setup(l)) {
      IMP::IntRange ir = IMP::atom::Domain(l).get_index_range();
      for (int i = ir.first; i < ir.second; ++i) {
        cur.push_back(i);
      }
    }
    IMP::atom::Hierarchy min;
    double min_distance = std::numeric_limits<double>::max();
    IMP_FOREACH(IMP::atom::Hierarchy c, out) {
      double cur_dist = IMP::core::get_distance(IMP::core::XYZR(c), d);
      if (cur_dist < min_distance) {
        min_distance = cur_dist;
        min = c;
      }
    }
    indexes[min].insert(cur.begin(), cur.end());
  }
  typedef std::pair<IMP::atom::Hierarchy, IMP_BASE_SMALL_UNORDERED_SET<int> >
      IP;
  IMP_FOREACH(IP ip, indexes) {
    IMP::atom::Fragment::setup_particle(
        ip.first, IMP::Ints(ip.second.begin(), ip.second.end()));
    IMP::atom::Mass::setup_particle(
        ip.first,
        IMP::atom::get_mass_from_number_of_residues(ip.second.size()));
  }
}

void add_bonds(const IMP::atom::Hierarchies &out) {
  IMP_FUNCTION_LOG;
  IMP_FOREACH(IMP::atom::Hierarchy c, out) {
    IMP::atom::Bonded::setup_particle(c);
  }
  base::set_progress_display("adding bonds", out.size());
  for (unsigned int i = 0; i < out.size(); ++i) {
    IMP::Ints ii = IMP::atom::Fragment(out[i]).get_residue_indexes();
    std::sort(ii.begin(), ii.end());
    for (unsigned int j = 0; j < i; ++j) {
      IMP::Ints ij = IMP::atom::Fragment(out[j]).get_residue_indexes();
      std::sort(ij.begin(), ij.end());
      IMP_FOREACH(int iic, ii) {
        if (std::binary_search(ij.begin(), ij.end(), iic + 1) ||
            std::binary_search(ij.begin(), ij.end(), iic - 1) ||
            std::binary_search(ij.begin(), ij.end(), iic)) {
          IMP::atom::create_custom_bond(
              IMP::atom::Bonded(out[i]), IMP::atom::Bonded(out[j]),
              std::max<double>(
                  0, IMP::core::get_distance(IMP::core::XYZR(out[i]),
                                             IMP::core::XYZR(out[j]))));
          break;
        }
      }
    }
    base::add_to_progress_display(1);
  }
}
}

Hierarchy create_simplified_from_volume(Hierarchy h, double resolution) {
  IMP_FUNCTION_LOG;
  Model *m = h.get_model();
  IMP::algebra::Sphere3Ds in_spheres;
  IMP_FOREACH(IMP::atom::Hierarchy child, IMP::atom::get_leaves(h)) {
    in_spheres.push_back(IMP::core::XYZR(child).get_sphere());
  }

  IMP::algebra::Sphere3Ds out_spheres =
      IMP::algebra::get_simplified_from_volume(in_spheres, 1.0 / resolution);

  IMP_LOG_TERSE("Input hierarchy has " << in_spheres.size()
                                       << " balls and output has "
                                       << out_spheres.size() << std::endl);

  IMP::atom::Hierarchies leaves;
  IMP_FOREACH(IMP::algebra::Sphere3D s, out_spheres) {
    IMP::kernel::ParticleIndex cur = m->add_particle("fragment");
    leaves.push_back(IMP::atom::Hierarchy::setup_particle(m, cur));
    IMP::core::XYZR::setup_particle(m, cur, s);
  }
  assign_residues(h, leaves);
  add_bonds(leaves);
  Hierarchy ret = Hierarchy::setup_particle(m, m->add_particle(h->get_name()));
  IMP_FOREACH(Hierarchy c, leaves) { ret.add_child(c); }
  return ret;
}

Hierarchy create_simplified_assembly_from_volume(Hierarchy h,
                                                 double resolution) {
  IMP_FUNCTION_LOG;
  if (Chain::get_is_setup(h)) {
    Hierarchy ret = create_simplified_from_volume(h, resolution);
    Chain::setup_particle(ret, Chain(h).get_id());
    return ret;
  } else if (Molecule::get_is_setup(h)) {
    Hierarchy ret = create_simplified_from_volume(h, resolution);
    Molecule::setup_particle(ret);
    return ret;
  } else {
    Hierarchy ret = Hierarchy::setup_particle(
        h.get_model(), h.get_model()->add_particle(h->get_name()));
    IMP_FOREACH(Hierarchy c, h.get_children()) {
      ret.add_child(create_simplified_assembly_from_volume(c, resolution));
    }
    return ret;
  }
}

IMPATOM_END_NAMESPACE
