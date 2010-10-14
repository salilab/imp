/**
 *  \file hierarchy_tools.h
 *  \brief functionality for defining rigid bodies
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_HIERARCHY_TOOLS_H
#define IMPATOM_HIERARCHY_TOOLS_H

#include "atom_config.h"
#include "Hierarchy.h"
#include "Residue.h"
#include "Atom.h"
#include <IMP/core/XYZR.h>


IMPATOM_BEGIN_NAMESPACE

//! Create a coarse grained molecule
/** The coarse grained model is created with a number of spheres
    based on the resolution and the volume. If the volume is not provided
    it is estimated based on the number of residues. The protein is
    created as a molecular hierarchy rooted at p. The leaves are Domain
    particles with appropriate residue indexes stored and are XYZR
    particles.

    Volume is, as usual, in cubic anstroms.

    Currently the function creates a set of balls with radii no greater
    than resolution which overlap by 20% and have a volume of their
    union equal to the passed volume. The balls are held together by
    a ConnectivityRestraint with the given spring constant.

    The coordinates of the balls defining the protein are optimized
    by default, and have garbage coordinate values.
    \untested{create_protein}
    \unstable{create_protein}
    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy create_protein(Model *m,
                                       double resolution,
                                       int number_of_residues,
                                       int first_residue_index=0,
                                       double volume=-1,
                                       double spring_strength=1);


/** \name Simplification along backbone

    These two methods create a simplified version of a molecule by
    merging residues sequentially. In one case every n residues are
    merged, in the other, the intervals are passed manually. The
    resulting molecule is not optimized by default and has no
    restraints automatically created.

    At the moment, the calls only support unmodified hierarchies loaded
    by read_pdb() which have only protein or DNA members.

    They return Hierarchy() if the input chain is empty.
    @{
*/
IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
                                                   int num_res);

IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
           const IntRanges& residue_segments);
/** @} */


/** \name Finding information
    Get the attribute of the given particle or throw a ValueException
    if it is not applicable. The particle with the given information
    must be above the passed node.
    @{
*/
IMPATOMEXPORT std::string get_molecule_name(Hierarchy h);
IMPATOMEXPORT Ints get_residue_indexes(Hierarchy h);
IMPATOMEXPORT ResidueType get_residue_type(Hierarchy h);
IMPATOMEXPORT char get_chain(Hierarchy h);
IMPATOMEXPORT AtomType get_atom_type(Hierarchy h);
IMPATOMEXPORT std::string get_domain_name(Hierarchy h);
/** @} */


/** A part of an atom.Hiearchy or atom.Hierarchies that is identified
    by the biological name. For example (in python)
    \code
    Named(hierarchy=h, molecule="myprotein", terminus=Named.C)
    Named(hierarchy=h, molecule="myprotein", residue=133)
    Named(hierarchy=h, molecule="myprotein", residues=[(133,134)])
    \endcode
    each get the C-terminus of the protein "myprotein" (assuming the last
    residue index is 133).
*/
class IMPATOMEXPORT Named {
 public:
  enum Terminus {NONE, C,N};
 private:
  Hierarchies h_;
  Strings molecules_;
  Ints residue_indices_;
  ResidueTypes residue_types_;
  std::string chains_;
  AtomTypes atom_types_;
  Strings domains_;
  double radius_;
  Terminus terminus_;
  bool check_nonradius(Hierarchy h) const;
  bool operator()(Hierarchy h) const;
 public:
#ifdef IMP_DOXYGEN
  /** When using python, you have much more control over
      construction due to the use of keyword arguments. You
      can provide any subset of the arguments (although one
      of hierarchy or hierarchies must be provided).
  */
  Named(Hierarchy hierarchy=None,
        Hierarchies hierarchies=[],
        Strings molecules=[],
        Ints residue_indexes=[],
        Strings chains=[],
        AtomTypes atom_types=[],
        ResidueTypes residue_types=[],
        Strings domains=[],
        double target_radius=-1,
        std::string molecule=None,
        int residue_index=None,
        char chain=None,
        AtomType atom_type=None,
        ResidueType residue_type=None,
        Terminus terminus=None,
        std::string domain=None);
#endif
  Named(Hierarchy h): h_(1, h){
    radius_=-1;
    terminus_=NONE;
  }
  Named(Hierarchies h): h_(h){
    radius_=-1;
    terminus_=NONE;
  }
  // for C++
  Named(Hierarchy h,
        std::string molname,
        int residue_index): h_(h), molecules_(1,molname),
    residue_indices_(1, residue_index),
    radius_(-1), terminus_(NONE){
  }
  Hierarchies get_hierarchies() const {
    return h_;
  }
  void set_molecules(Strings mols) {
    molecules_= mols;
    std::sort(molecules_.begin(), molecules_.end());
  }
  void set_target_radius(double r) {
    radius_=r;
  }
  void set_terminus(Terminus t) {
    terminus_=t;
  }
  void set_chains(std::string chains) {
    chains_= chains;
    std::sort(chains_.begin(), chains_.end());
  }
  void set_residue_indexes(Ints indexes) {
    residue_indices_= indexes;
    std::sort(residue_indices_.begin(), residue_indices_.end());
  }
  void set_atom_types(AtomTypes types) {
    atom_types_= types;
    std::sort(atom_types_.begin(), atom_types_.end());
  }
  void set_residue_types(ResidueTypes types) {
    residue_types_= types;
    std::sort(residue_types_.begin(), residue_types_.end());
  }
  void set_domains(Strings types) {
    domains_= types;
    std::sort(domains_.begin(), domains_.end());
  }
  void set_molecule(std::string mol) {
    molecules_= Strings(1,mol);
  }
  void set_chain(char c) {
    chains_= std::string(1,c);
  }
  void set_residue_index(int i) {
    residue_indices_= Ints(1,i);
  }
  void set_atom_type(AtomType types) {
    atom_types_= AtomTypes(1,types);
  }
  void set_residue_type(ResidueType type) {
    residue_types_= ResidueTypes(1,type);
  }
  void set_domain(std::string name) {
    domains_= Strings(1, name);
  }
  ParticlesTemp get_particles() const;
};

IMP_VALUES(Named, Nameds);

IMPATOMEXPORT Restraint* create_distance_restraint(const Named &n0,
                                                   const Named &n1,
                                                   double x0, double k);


IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_HIERARCHY_TOOLS_H */
