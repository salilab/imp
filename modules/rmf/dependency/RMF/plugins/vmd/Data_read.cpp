/**
 *  \file rmfplugin.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#include "Data.h"
#include <RMF/log.h>
#include "molfile_plugin.h"

namespace RMF_vmd {

int Data::read_structure(int *optflags, molfile_atom_t *out) {
  RMF_INFO("Reading structure");
  RMF_FOREACH(const Body & body, bodies_) {
    out = copy_particles(body.particles, out);
    out = copy_balls(body.balls, out);
  }
  *optflags = MOLFILE_RADIUS | MOLFILE_MASS | MOLFILE_ALTLOC;
  return VMDPLUGIN_SUCCESS;
}

int Data::read_timestep(molfile_timestep_t *frame) {
  if (done_) return MOLFILE_EOF;
  RMF::FrameID curf = file_.get_current_frame();
  RMF_INFO("Reading next frame at " << curf);
  frame->physical_time = curf.get_index();
  float *coords = frame->coords;
  file_.set_current_frame(RMF::FrameID(curf.get_index()));
  RMF_FOREACH(const Body & body, bodies_) {
    RMF::CoordinateTransformer tr;
    double offset =
        (bounds_[1][0] - bounds_[0][0] + max_radius_ * 3) * body.state;
    RMF_FOREACH(RMF::decorator::ReferenceFrameConst rf, body.frames) {
      tr = RMF::CoordinateTransformer(tr, rf);
    }
    RMF_FOREACH(const AtomInfo & n, body.particles) {
      RMF::Vector3 cc = pf_.get(file_.get_node(n.node_id)).get_coordinates();
      cc = tr.get_global_coordinates(cc);
      cc[0] += offset;
      std::copy(cc.begin(), cc.end(), coords);
      coords += 3;
    }
    RMF_FOREACH(const AtomInfo & n, body.balls) {
      RMF::Vector3 cc = bf_.get(file_.get_node(n.node_id)).get_coordinates();
      cc = tr.get_global_coordinates(cc);
      cc[0] += offset;
      std::copy(cc.begin(), cc.end(), coords);
      coords += 3;
    }
  }
  unsigned int next = curf.get_index() + 1;
  if (next >= file_.get_number_of_frames()) {
    done_ = true;
  } else {
    file_.set_current_frame(RMF::FrameID(next));
  }
  return VMDPLUGIN_SUCCESS;
}

int Data::read_graphics(int *nelem, const molfile_graphics_t **gdata) {
  RMF_INFO("Reading graphics");
  fill_graphics(file_.get_root_node(), RMF::CoordinateTransformer());
  *nelem = graphics_.size();
  *gdata = &graphics_[0];
  return VMDPLUGIN_SUCCESS;
}

int Data::read_bonds(int *nbonds, int **fromptr, int **toptr,
                     float **bondorderptr, int **bondtype, int *nbondtypes,
                     char ***bondtypename) {
  RMF_INFO("Reading bonds");
  fill_bonds(file_.get_root_node());
  RMF_INTERNAL_CHECK(bond_type_.size() == bond_from_.size(),
                     "Sizes don't match");
  RMF_INTERNAL_CHECK(bond_type_.size() == bond_to_.size(), "Sizes don't match");
  *nbonds = bond_type_.size();
  RMF_TRACE("Found " << *nbonds << " bonds.");
  *fromptr = &bond_from_[0];
  *toptr = &bond_to_[0];
  *bondtype = &bond_type_[0];
  *bondorderptr = NULL;
  *nbondtypes = 2;
  bond_type_name_.reset(new char[2]);
  bond_type_name_[0] = 'B';
  bond_type_name_[1] = '\0';
  restraint_bond_type_name_.reset(new char[2]);
  restraint_bond_type_name_[0] = 'R';
  restraint_bond_type_name_[1] = '\0';
  bond_type_names_.push_back(bond_type_name_.get());
  bond_type_names_.push_back(restraint_bond_type_name_.get());
  *bondtypename = &bond_type_names_[0];

  RMF_FOREACH(int bt,
              boost::make_iterator_range(*bondtype, *bondtype + *nbonds)) {
    RMF_UNUSED(bt);
    RMF_INTERNAL_CHECK(bt >= 0 && bt < *nbondtypes, "Invalid bond type ");
  }
  return VMDPLUGIN_SUCCESS;
}

int Data::read_timestep_metadata(molfile_timestep_metadata_t *data) {
  RMF_INFO("Reading timestep data");
  data->count = file_.get_number_of_frames();
  int na = 0;
  RMF_FOREACH(const Body & bd, bodies_) {
    na += bd.particles.size() + bd.balls.size();
  }
  data->avg_bytes_per_timestep = sizeof(float) * 3 * na;
  data->has_velocities = 0;
  return VMDPLUGIN_SUCCESS;
}
}  // namespace RMF_vmd
