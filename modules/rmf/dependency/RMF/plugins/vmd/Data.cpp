/**
 *  \file rmfplugin.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2016 IMP Inventors. All rights reserved.
 *
 */

#include "Data.h"
#include <RMF/utility.h>
#include <boost/range/iterator_range.hpp>

namespace RMF_vmd {

namespace {
template <class R, class OIt>
void my_copy_n(const R &range, std::size_t n, OIt out) {
  if (static_cast<std::size_t>(boost::distance(range)) <= n) {
    // older boost doesn't have range copy
    std::copy(range.begin(), range.end(), out);
  } else {
    std::copy(range.begin(), range.begin() + n, out);
  }
}
}

Data::Data(std::string name, int *num_atoms)
    : file_(RMF::open_rmf_file_read_only(name)),
      af_(file_),
      rf_(file_),
      chf_(file_),
      pf_(file_),
      rff_(file_),
      bf_(file_),
      sf_(file_),
      cf_(file_),
      bdf_(file_),
      scf_(file_),
      rcf_(file_),
      df_(file_),
      ff_(file_),
      cpf_(file_),
      tf_(file_),
      altf_(file_),
      stf_(file_),
      resolution_(get_resolution()),
      show_restraints_(get_show_restraints()),
      max_radius_(0),
      done_(false) {
  if (file_.get_number_of_frames() > 0) {
    file_.set_current_frame(RMF::FrameID(0));
  }

  bodies_.push_back(Body());

  boost::array<char, 2> default_chain = {{0}};
  boost::array<char, 8> default_resname = {{0}};
  boost::array<char, 2> default_altid = {{0}};
  boost::array<char, 8> default_segment = {{0}};
  boost::array<int, 2> na =
      fill_bodies(file_.get_root_node(), 0, default_chain, -1, default_resname,
                  default_altid, default_segment, resolution_);
  fill_index();
  *num_atoms = na[0] + na[1];
  if (*num_atoms == 0) {
    *num_atoms = MOLFILE_NUMATOMS_NONE;
  }

  bounds_ = RMF::get_bounding_box(file_.get_root_node());
  std::cout << "RMF: found " << *num_atoms << " atoms." << std::endl;
}

double Data::get_resolution() {
  RMF::Floats resolutions = RMF::decorator::get_resolutions(
      file_.get_root_node(), RMF::PARTICLE, .01);
  if (resolutions.size() > 1) {
    std::cout << "RMF: Resolutions are " << RMF::Showable(resolutions)
              << ".\nPlease enter desired resolution (or -1 for all): "
              << std::flush;
    double r = -1;
    std::cin >> r;
    std::cout << "Using resolution " << r << std::endl;
    // fix divide by zero
    if (r == 0)
      return .0001;
    else
      return r;
  }
  return 1;
}

int Data::get_show_restraints() {
  bool has_restraints = false;
  RMF_FOREACH(RMF::NodeID n, file_.get_node_ids()) {
    if (rcf_.get_is(file_.get_node(n))) {
      has_restraints = true;
      break;
    }
  }
  if (has_restraints) {
    std::cout << "RMF: File has restraints. Please choose what to display.\n"
              << "0 for just bonds, 1 for just restraints or 2 for both: "
              << std::flush;
    int r = -1;
    std::cin >> r;
    switch (r) {
      case 0:
        return BONDS;
      case 1:
        return RESTRAINTS;
      default:
        return BONDS | RESTRAINTS;
    }
  } else {
    std::cout << "No restraints found in file." << std::endl;
    return BONDS;
  }
}

int Data::handle_reference_frame(int body, RMF::NodeConstHandle cur) {
  bodies_.push_back(Body());
  bodies_.back().frames = bodies_[body].frames;
  bodies_.back().frames.push_back(rff_.get(cur));
  bodies_.back().state = bodies_[body].state;
  return bodies_.size() - 1;
}

int Data::handle_state(int body, RMF::NodeConstHandle cur) {
  int state_index = stf_.get(cur).get_state_index();
  // don't create duplicate bodies for 0
  if (state_index != bodies_[body].state) {
    if (rff_.get_is(cur)) {
      bodies_.back().state = state_index;
      return body;
    } else {
      bodies_.push_back(Body());
      bodies_.back().frames = bodies_[body].frames;
      bodies_.back().state = state_index;
      return bodies_.size() - 1;
    }
  } else {
    return body;
  }
}

boost::tuple<RMF::NodeConstHandle, boost::array<char, 2>, boost::array<int, 2> >
Data::handle_alternative(RMF::NodeConstHandle cur, int body,
                         boost::array<char, 2> chain, int resid,
                         boost::array<char, 8> resname,
                         boost::array<char, 2> altid,
                         boost::array<char, 8> segment, double resolution) {
  boost::array<int, 2> count = {{0}};
  if (resolution >= 0) {
    RMF::NodeConstHandle alt =
        altf_.get(cur).get_alternative(RMF::PARTICLE, resolution);
    return boost::make_tuple(alt, altid, count);
  } else {
    RMF::NodeConstHandles alts = altf_.get(cur).get_alternatives(RMF::PARTICLE);
    int alt = 1;
    RMF_FOREACH(RMF::NodeConstHandle c,
                boost::make_iterator_range(alts.begin() + 1, alts.end())) {
      altid[0] = 'A' + alt;
      boost::array<int, 2> cur = fill_bodies(c, body, chain, resid, resname,
                                             altid, segment, resolution);
      for (unsigned int i = 0; i < 2; ++i) {
        count[i] += cur[i];
      }
      ++alt;
    }
    altid[0] = 'A';
    return boost::make_tuple(alts.front(), altid, count);
  }
}

boost::array<int, 2> Data::fill_bodies(RMF::NodeConstHandle cur, int body,
                                       boost::array<char, 2> chain, int resid,
                                       boost::array<char, 8> resname,
                                       boost::array<char, 2> altid,
                                       boost::array<char, 8> segment,
                                       double resolution) {
  boost::array<int, 2> ret = {{0}};
  // must be firest due to ret
  if (altf_.get_is(cur))
    boost::tie(cur, altid, ret) = handle_alternative(
        cur, body, chain, resid, resname, altid, segment, resolution);

  if (cur.get_type() == RMF::ALIAS) return ret;
  if (cur.get_type() == RMF::REPRESENTATION && segment[0] == '\0') {
    my_copy_n(cur.get_name(), 8, segment.begin());
  }

  if (rff_.get_is(cur)) body = handle_reference_frame(body, cur);

  if (stf_.get_is(cur)) body = handle_state(body, cur);

  if (chf_.get_is(cur)) {
    my_copy_n(chf_.get(cur).get_chain_id(), 2, chain.begin());
    //    chain = chf_.get(cur).get_chain_id();
  }
  if (rf_.get_is(cur)) {
    resid = rf_.get(cur).get_residue_index();
    my_copy_n(rf_.get(cur).get_residue_type(), 8, resname.begin());
  }

  RMF_FOREACH(RMF::NodeConstHandle c, cur.get_children()) {
    boost::array<int, 2> count =
        fill_bodies(c, body, chain, resid, resname, altid, segment, resolution);
    for (unsigned int i = 0; i < 2; ++i) {
      ret[i] += count[i];
    }
  }

  if (ret[0] == 0 && pf_.get_is(cur)) {
    AtomInfo ai = {chain, resid, resname, altid, segment, cur.get_id()};
    bodies_[body].particles.push_back(ai);
    ++ret[0];
  }
  if (bf_.get_is(cur)) {
    AtomInfo ai = {chain, resid, resname, altid, segment, cur.get_id()};
    bodies_[body].balls.push_back(ai);
    ++ret[1];
  }
  return ret;
}

void Data::fill_index() {
  int cur_index = -1;
  RMF_FOREACH(const Body & body, bodies_) {
    RMF_FOREACH(const AtomInfo & n, body.particles) {
      index_[n.node_id] = ++cur_index;
    }
    RMF_FOREACH(const AtomInfo & n, body.balls) {
      index_[n.node_id] = ++cur_index;
    }
  }
}

void Data::fill_graphics(RMF::NodeConstHandle cur,
                         RMF::CoordinateTransformer tr) {
  if (rff_.get_is(cur)) {
    tr = RMF::CoordinateTransformer(tr, rff_.get(cur));
  }
  if (sf_.get_is_static(cur)) {
    RMF::decorator::SegmentConst s = sf_.get(cur);
    RMF::Vector3s coords = s.get_static_coordinates_list();
    RMF_INTERNAL_CHECK(coords.size() > 0, "Empty coordinates");
    int type = MOLFILE_LINE;
    double size = 0;
    if (cf_.get_is(cur)) {
      type = MOLFILE_CYLINDER;
      size = cf_.get(cur).get_radius();
    }
    RMF::Vector3 last_coords = tr.get_global_coordinates(coords[0]);
    RMF_FOREACH(const RMF::Vector3 & cc,
                boost::make_iterator_range(coords.begin() + 1, coords.end())) {
      molfile_graphics_t g;
      g.type = type;
      g.size = size;
      g.style = 0;
      std::copy(last_coords.begin(), last_coords.end(), g.data);
      last_coords = tr.get_global_coordinates(cc);
      std::copy(last_coords.begin(), last_coords.end(), g.data + 3);
      graphics_.push_back(g);
    }
  }
  RMF_FOREACH(RMF::NodeConstHandle c, cur.get_children()) {
    fill_graphics(c, tr);
  }
}

void Data::handle_bond(RMF::NodeConstHandle cur) {
  RMF::NodeID bonded0(bdf_.get(cur).get_bonded_0());
  RMF::NodeID bonded1(bdf_.get(cur).get_bonded_1());

  if (index_.find(bonded0) != index_.end() &&
      index_.find(bonded1) != index_.end()) {
    bond_from_.push_back(index_.find(bonded0)->second + 1);
    bond_to_.push_back(index_.find(bonded1)->second + 1);
    bond_type_.push_back(0);
  }
}

void Data::handle_restraint(RMF::NodeConstHandle cur) {
  bool child_feature = false;
  RMF_FOREACH(RMF::NodeConstHandle c, cur.get_children()) {
    if (c.get_type() == RMF::FEATURE) {
      child_feature = true;
      break;
    }
  }
  if (!child_feature && rcf_.get_is(cur)) {
    RMF::NodeConstHandles reps = rcf_.get(cur).get_representation();
    for (unsigned int i = 0; i < reps.size(); ++i) {
      RMF::NodeID bonded0(reps[i]);
      for (unsigned int j = 0; j < i; ++j) {
        RMF::NodeID bonded1(reps[j]);
        if (index_.find(bonded0) != index_.end() &&
            index_.find(bonded1) != index_.end()) {
          bond_from_.push_back(index_.find(bonded0)->second + 1);
          bond_to_.push_back(index_.find(bonded1)->second + 1);
          bond_type_.push_back(1);
        }
      }
    }
  }
}

void Data::fill_bonds(RMF::NodeConstHandle cur) {
  if (show_restraints_ & BONDS && bdf_.get_is(cur)) {
    handle_bond(cur);
  }
  if (show_restraints_ & RESTRAINTS && cur.get_type() == RMF::FEATURE) {
    handle_restraint(cur);
  }
  RMF_FOREACH(RMF::NodeConstHandle c, cur.get_children()) { fill_bonds(c); }
}

void Data::copy_basics(const AtomInfo &n, molfile_atom_t *out) {
  out->resid = n.residue_index;
  std::copy(n.chain_id.begin(), n.chain_id.end(), out->chain);
  my_copy_n(n.altid, 2, out->altloc);
  my_copy_n(n.segment, 8, out->segid);
  my_copy_n(n.residue_name, 8, out->resname);
  RMF::NodeConstHandle cur = file_.get_node(n.node_id);
  my_copy_n(cur.get_name(), 16, out->name);
}

molfile_atom_t *Data::copy_particles(const std::vector<AtomInfo> &atoms,
                                     molfile_atom_t *out) {
  RMF_FOREACH(const AtomInfo & n, atoms) {
    copy_basics(n, out);

    RMF::NodeConstHandle cur = file_.get_node(n.node_id);
    std::string at;
    if (af_.get_is(cur)) {
      at = cur.get_name();
    } else if (ff_.get_is(cur)) {
      at = "FRAGMENT";
    } else if (df_.get_is(cur)) {
      at = "DOMAIN";
    } else if (tf_.get_is(cur)) {
      at = tf_.get(cur).get_type_name();
    }
    my_copy_n(at, 16, out->type);

    out->mass = pf_.get(cur).get_mass();
    out->radius = pf_.get(cur).get_radius();

    ++out;
  }
  return out;
}

molfile_atom_t *Data::copy_balls(const std::vector<AtomInfo> &balls,
                                 molfile_atom_t *out) {
  RMF_FOREACH(AtomInfo n, balls) {
    copy_basics(n, out);

    RMF::NodeConstHandle cur = file_.get_node(n.node_id);
    my_copy_n(std::string("ball"), 16, out->type);
    out->mass = 0;
    out->radius = bf_.get(cur).get_radius();
    ++out;
  }
  return out;
}

}  // namespace RMF_vmd
