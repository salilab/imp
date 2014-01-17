/**
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 */
#include <IMP/atom/hierarchy_tools.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/pdb.h>
#include <IMP/base/flags.h>
#include <IMP/rmf/atom_io.h>
#include <IMP/rmf/frames.h>
#include <RMF/FileHandle.h>

#if 0
namespace {

struct SphereTag {};
struct SPTag {};
typedef IMP::base::Index<SphereTag> SphereIndex;
typedef IMP::base::Index<SPTag> SPIndex;
typedef IMP::base::Vector<SphereIndex> SphereIndexes;
typedef IMP::base::Vector<SPIndex> SPIndexes;
typedef IMP_BASE_SMALL_UNORDERED_SET<SphereIndex> SphereIndexSet;
typedef IMP_BASE_SMALL_UNORDERED_SET<SPIndex> SPIndexSet;


IMP::algebra::Sphere3Ds get_surface_simplified(IMP::algebra::Sphere3Ds in,
                                               double resolution) {
  const double probe =  1.0 / resolution;
  double tot_rad = 0;
  IMP_FOREACH(IMP::algebra::Sphere3D &s, in) {
    tot_rad += s.get_radius();
    s = IMP::algebra::Sphere3D(s.get_center(), s.get_radius() + probe);
  }
  double average_rad = tot_rad/in.size();
  IMP_LOG_TERSE("Getting connolly surface with "
                << in.size() << " points and resolution " << average_rad
                << std::endl);
  IMP::algebra::Vector3Ds sps = IMP::algebra::get_uniform_surface_cover(
      in, 1.0 / IMP::algebra::get_squared(resolution));
  IMP_LOG_TERSE("Got " << sps.size() << " connolly points." << std::endl);
  {
    std::ofstream cpout("/tmp/connolly.bild");
    IMP_FOREACH(const IMP::algebra::Vector3D & sp, sps) {
      cpout << ".dotat " << sp[0] << " " << sp[1] << " " << sp[2] << std::endl;
    }
  }

  // which surface points support each sphere
  typedef std::pair<SphereIndex, SPIndexSet> SupportsPair;
  IMP_BASE_LARGE_UNORDERED_MAP<SphereIndex, SPIndexSet> supports;
  IMP_BASE_LARGE_UNORDERED_MAP<SphereIndex, double> radii;
  IMP_LOG_TERSE("Creating NN search structure." << std::endl);
  IMP_NEW(IMP::algebra::NearestNeighborD<3>, nns, (sps));

  IMP_LOG_TERSE("Searching for nearest neighbors." << std::endl);
  {
    std::ofstream cpout("/tmp/balls.bild");
    for (unsigned int i = 0; i < in.size(); ++i) {
      SphereIndex si(i);
      unsigned int nn = nns->get_nearest_neighbor(in[i].get_center());
      double r = IMP::algebra::get_distance(sps[nn], in[i].get_center());
      IMP_FOREACH(IMP::algebra::Vector3D v, sps) {
        IMP_INTERNAL_CHECK(
            IMP::algebra::get_distance(v, in[i].get_center()) > .9 * r,
            "Bad nearest neighbor. Found one at "
                << v << " with distance "
                << IMP::algebra::get_distance(v, in[i].get_center())
                << " as opposed to " << r << " from " << in[i]);
      }
      std::cout << "Searching at " << i << " with radius " << r << " "
                << 1.0 / resolution << std::endl;
      IMP::Ints support =
          nns->get_in_ball(in[i].get_center(), r + 1.0 / resolution);

      IMP_FOREACH(int i, support) { supports[si].insert(SPIndex(i)); }
      radii[si] = r + .5 / resolution;
      cpout << ".sphere " << in[i].get_center()[0] << " "
            << in[i].get_center()[1] << " " << in[i].get_center()[2] << " " << r
            << std::endl;
    }
  }

  for (unsigned int i = 0; i< in.size(); ++i) {
    SphereIndex si(i);
    IMP_FOREACH(IMP::algebra::Vector3D v, sps) {
      IMP_INTERNAL_CHECK(IMP::algebra::get_distance(v, in[i].get_center()) >=
                             radii.find(si)->second - 1.0 / resolution,
                         "Sphere too big at "
                             << in[i] << " with radius "
                             << radii.find(si)->second << " at surface point "
                             << v << " with distance "
                             << IMP::algebra::get_distance(v,
                                                           in[i].get_center()));
    }
  }

 IMP_LOG_TERSE("Distributing support." << std::endl);
  // which spheres are supported by each point
  typedef std::pair<SPIndex, SphereIndexSet> SupportedPair;
  IMP_BASE_LARGE_UNORDERED_MAP<SPIndex, SphereIndexSet> supported;
  IMP_FOREACH(const SupportsPair &ps, supports) {
    IMP_FOREACH(SPIndex spi, ps.second) {
      supported[spi].insert(ps.first);
    }
  }

  for (unsigned int i = 0; i< sps.size(); ++i) {
    SPIndex spi(i);
    IMP_INTERNAL_CHECK(supported.find(spi) != supported.end(),
                       "Point is not supported");
  }

  IMP_LOG_TERSE("Generating output." << std::endl);
  IMP::algebra::Sphere3Ds ret;
  while (!supported.empty()) {
    IMP_USAGE_CHECK(!supports.empty(), "Out of spheres");
    SphereIndex max;
    double max_score = 0;
    IMP_FOREACH(const SupportsPair &sp, supports) {
      double score = sp.second.size();
      if (score > max_score) {
        max_score = score;
        max = sp.first;
      }
    }
    std::cout << "Chose " << max << " with score " << max_score
              << " and radius " << radii.find(max)->second << std::endl;
    SPIndexSet max_supports = supports.find(max)->second;
    IMP_FOREACH(SPIndex spi, max_supports) {
      IMP_FOREACH(SphereIndex si, supported.find(spi)->second) {
        if (si != max) {
          supports[si].erase(spi);
          if (supports[si].empty()) supports.erase(si);
        }
      }
      supported.erase(spi);
    }
    supports.erase(max);
    ret.push_back(IMP::algebra::Sphere3D(in[max.get_index()].get_center(),
                                         radii.find(max)->second - probe));
  }
  return ret;
}

IMP::atom::Hierarchy create_hierarchy(const IMP::algebra::Sphere3Ds &ss,
                                      IMP::kernel::Model *m) {
  IMP::atom::Hierarchy root =
      IMP::atom::Hierarchy::setup_particle(m, m->add_particle("root"));
  IMP_FOREACH(IMP::algebra::Sphere3D s, ss) {
    IMP::kernel::ParticleIndex cur = m->add_particle("p");
    root.add_child(IMP::atom::Hierarchy::setup_particle(m, cur));
    IMP::core::XYZR::setup_particle(m, cur, s);
    IMP::atom::Mass::setup_particle(m, cur, 1);
  }
  return root;
}

void assign_residues(IMP::atom::Hierarchy in,
                     const IMP::atom::Hierarchies &out) {
  IMP_LOG_FUNCTION;
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
      for (unsigned int i = ir.first; i < ir.second; ++i) {
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
    IMP::atom::Mass(ip.first).set_mass(
        IMP::atom::get_mass_from_number_of_residues(ip.second.size()));
  }
}

void add_bonds(const IMP::atom::Hierarchies & out) {
  IMP_LOG_FUNCTION;
  IMP_FOREACH(IMP::atom::Hierarchy c, out) {
    IMP::atom::Bonded::setup_particle(c);
  }
  for (unsigned int i = 0; i < out.size(); ++i) {
    IMP::Ints ii = IMP::atom::Fragment(out[i]).get_residue_indexes();
    bool bonded = false;
    for (unsigned int j = 0; j < i && !bonded; ++j) {
      IMP::Ints ij = IMP::atom::Fragment(out[j]).get_residue_indexes();
      std::sort(ij.begin(), ij.end());
      IMP_FOREACH(int iic, ii) {
        if (std::binary_search(ij.begin(), ij.end(), iic + 1) ||
            std::binary_search(ij.begin(), ij.end(), iic - 1)) {
          IMP::atom::create_custom_bond(
              IMP::atom::Bonded(out[i]), IMP::atom::Bonded(out[j]),
              std::max<double>(
                  0, IMP::core::get_distance(IMP::core::XYZR(out[i]),
                                             IMP::core::XYZR(out[j]))));
          bonded = true;
          break;
        }
      }
    }
  }
}
IMP::atom::Hierarchy create_simplified(IMP::atom::Hierarchy in,
                                       double resolution) {
  IMP_LOG_FUNCTION;
  IMP::algebra::Sphere3Ds in_spheres;
  IMP_FOREACH(IMP::atom::Hierarchy h, IMP::atom::get_leaves(in)) {
    in_spheres.push_back(IMP::core::XYZR(h).get_sphere());
  }

  IMP::algebra::Sphere3Ds out_spheres =
      IMP::algebra::get_surface_simplified(in_spheres, 1.0 / resolution);

  IMP::atom::Hierarchy hn = create_hierarchy(out_spheres, in.get_model());
  assign_residues(in, IMP::atom::get_leaves(hn));
  add_bonds(IMP::atom::get_leaves(hn));
  // add bonds
  return hn;
}
}
#endif
int main(int argc, char *argv[]) {
  double resolution = 1;
  IMP::base::AddFloatFlag ra("resolution", "The resolution to use.",
                             &resolution);
  IMP::Strings args = IMP::base::setup_from_argv(
      argc, argv, "Create a simplified representation of a pdb",
      "input.pdb output.rmf", 2);

  IMP_NEW(IMP::kernel::Model, m, ());
  IMP::atom::Hierarchy hr = IMP::atom::read_pdb(args[0], m);

  IMP::atom::Hierarchy root =
      IMP::atom::Hierarchy::setup_particle(m, m->add_particle(hr->get_name()));
  IMP_FOREACH(IMP::atom::Hierarchy c,
              IMP::atom::get_by_type(hr, IMP::atom::CHAIN_TYPE)) {
    IMP::atom::Hierarchy cur = create_simplified_from_volume(hr, resolution);
    IMP::atom::Chain::setup_particle(cur, IMP::atom::Chain(c).get_id());
    root.add_child(cur);
  }

  RMF::FileHandle fh = RMF::create_rmf_file(args[1]);
  IMP::rmf::add_hierarchy(fh, root);
  IMP::rmf::save_frame(fh, "frame");
  return 0;
}
