/**
 *  \file rmfplugin.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_RMFPLUGIN_DATA_H
#define RMF_RMFPLUGIN_DATA_H

#include "RMF/CoordinateTransformer.h"
#include "RMF/FileConstHandle.h"
#include "RMF/ID.h"
#include "RMF/NodeConstHandle.h"
#include "RMF/Vector.h"
#include "RMF/decorator/alternatives.h"
#include "RMF/decorator/bond.h"
#include "RMF/decorator/feature.h"
#include "RMF/decorator/physics.h"
#include "RMF/decorator/representation.h"
#include "RMF/decorator/sequence.h"
#include "RMF/decorator/shape.h"
#include "RMF/enums.h"
#include "RMF/infrastructure_macros.h"
#include "RMF/types.h"
#include "molfile_plugin.h"
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/scoped_array.hpp>
#include <string>
#include <vector>

namespace RMF_vmd {

class Data {
  RMF::FileConstHandle file_;
  RMF::decorator::AtomFactory af_;
  RMF::decorator::ResidueFactory rf_;
  RMF::decorator::ChainFactory chf_;
  RMF::decorator::ParticleFactory pf_;
  RMF::decorator::ReferenceFrameFactory rff_;
  RMF::decorator::BallFactory bf_;
  RMF::decorator::SegmentFactory sf_;
  RMF::decorator::CylinderFactory cf_;
  RMF::decorator::BondFactory bdf_;
  RMF::decorator::ScoreFactory scf_;
  RMF::decorator::RepresentationFactory rcf_;
  RMF::decorator::DomainFactory df_;
  RMF::decorator::FragmentFactory ff_;
  RMF::decorator::CopyFactory cpf_;
  RMF::decorator::TypedFactory tf_;
  RMF::decorator::AlternativesFactory altf_;
  RMF::decorator::StateFactory stf_;
  struct AtomInfo {
    // can precompute the actual molfile_atom_t data to simplify things
    boost::array<char, 2> chain_id;
    int residue_index;
    boost::array<char, 8> residue_name;
    boost::array<char, 2> altid;
    boost::array<char, 8> segment;
    RMF::NodeID node_id;
  };
  struct Body {
    std::vector<RMF::decorator::ReferenceFrameConst> frames;
    std::vector<AtomInfo> particles;
    std::vector<AtomInfo> balls;
    int state;
    Body() : state(0) {}
  };
  std::vector<Body> bodies_;
  boost::unordered_map<RMF::NodeID, int> index_;
  std::vector<molfile_graphics_t> graphics_;
  std::vector<int> bond_to_, bond_from_, bond_type_;
  boost::scoped_array<char> bond_type_name_, restraint_bond_type_name_;
  std::vector<char *> bond_type_names_;
  double resolution_;
  enum ShowRestraints {
    BONDS = 1,
    RESTRAINTS = 2
  };
  int show_restraints_;
  boost::array<RMF::Vector3, 2> bounds_;
  double max_radius_;
  bool done_;

  // find nodes to push to VMD
  boost::array<int, 2> fill_bodies(RMF::NodeConstHandle cur, int body,
                                   boost::array<char, 2> chain, int resid,
                                   boost::array<char, 8> resname,
                                   boost::array<char, 2> altid,
                                   boost::array<char, 8> segment,
                                   double resolution);
  void fill_index();
  void fill_graphics(RMF::NodeConstHandle cur, RMF::CoordinateTransformer tr);
  void fill_bonds(RMF::NodeConstHandle cur);
  int handle_reference_frame(int body, RMF::NodeConstHandle cur);
  int handle_state(int body, RMF::NodeConstHandle cur);
  boost::tuple<RMF::NodeConstHandle, boost::array<char, 2>,
               boost::array<int, 2> >
      handle_alternative(RMF::NodeConstHandle cur, int body,
                         boost::array<char, 2> chain, int resid,
                         boost::array<char, 8> resname,
                         boost::array<char, 2> altid,
                         boost::array<char, 8> segment, double resolution);
  void handle_bond(RMF::NodeConstHandle cur);
  void handle_restraint(RMF::NodeConstHandle cur);
  double get_resolution();
  int get_show_restraints();

  void copy_basics(const AtomInfo &ai, molfile_atom_t *out);
  molfile_atom_t *copy_particles(const std::vector<AtomInfo> &atoms,
                                 molfile_atom_t *out);
  molfile_atom_t *copy_balls(const std::vector<AtomInfo> &balls,
                             molfile_atom_t *out);

 public:
  Data(std::string name, int *num_atoms);
  int read_structure(int *optflags, molfile_atom_t *atoms);
  int read_timestep(molfile_timestep_t *frame);
  int read_graphics(int *nelem, const molfile_graphics_t **gdata);
  int read_bonds(int *nbonds, int **fromptr, int **toptr, float **bondorderptr,
                 int **bondtype, int *nbondtypes, char ***bondtypename);
  int read_timestep_metadata(molfile_timestep_metadata_t *data);
};
}  // namespace RMF_vmd
#endif /* RMF_RMFPLUGIN_DATA_H */
