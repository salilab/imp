/**
 *  \file hierarchy_tools.h
 *  \brief A set of useful functionality on IMP::atom::Hierarchy decorators
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
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
                                       std::string name,
                                       double resolution,
                                       int number_of_residues,
                                       int first_residue_index=0,
                                       double volume=-1
#ifndef IMP_DOXYGEN
                                       , bool ismol=true
#endif
);
/** Like the former create_protein(), but it enforces domain splits
    at the provide domain boundairs. The domain boundaries should be
    the start of the first domain, any boundies, and then one past
    the end of the last domain.
 */
IMPATOMEXPORT Hierarchy create_protein(Model *m,
                                       std::string name,
                                       double resolution,
                                       const Ints domain_boundaries);


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
/** Simplify every num_res into one particle.*/
IMPATOMEXPORT Hierarchy create_simplified_along_backbone(Chain in,
                                                         int num_res);
/** Simplify by breaking at the boundaries provided.*/
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


/** Select a part of an atom.Hiearchy or atom.Hierarchies that is identified
    by the biological name.


    For example (in python)
    \code
    Selection(hierarchy=h, molecule="myprotein", terminus=Selection.C)
    Selection(hierarchy=h, molecule="myprotein", residue_index=133)
    Selection(hierarchy=h, molecule="myprotein", residue_indexes=[(133,134)])
    \endcode
    each get the C-terminus of the protein "myprotein" (assuming the last
    residue index is 133).

    \note Only representational particles are selected. That is, ones
    with x,y,z coordinates. And the highest resolution representation
    that fits is returned. If you want lower resolution, use the
    target_radius parameter to select the desired radius (pass a very large
    number to get the coarsest representation).
*/
class IMPATOMEXPORT Selection {
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
  Selection(Hierarchy hierarchy=None,
        Hierarchies hierarchies=[],
        Strings molecules=[],
        Ints residue_indexes=[],
        Strings chains=[],
        AtomTypes atom_types=[],
        ResidueTypes residue_types=[],
        Strings domains=[],
        double target_radius=0,
        std::string molecule=None,
        int residue_index=None,
        char chain=None,
        AtomType atom_type=None,
        ResidueType residue_type=None,
        Terminus terminus=None,
        std::string domain=None);
#endif
  Selection(){
    radius_=-1;
    terminus_=NONE;
  }
  Selection(Hierarchy h): h_(1, h){
    IMP_USAGE_CHECK(h.get_is_valid(true), "Hierarchy " << h
                    << " is not valid.");
    radius_=-1;
    terminus_=NONE;
  }
  Selection(Hierarchies h): h_(h){
    for (unsigned int i=0; i< h.size(); ++i) {
      IMP_USAGE_CHECK(h[i].get_is_valid(true), "Hierarchy " << h[i]
                    << " is not valid.");
    }
    radius_=-1;
    terminus_=NONE;
  }
  // for C++
  Selection(Hierarchy h,
        std::string molname,
        int residue_index): h_(h), molecules_(1,molname),
    residue_indices_(1, residue_index),
    radius_(-1), terminus_(NONE){
    IMP_USAGE_CHECK(h.get_is_valid(true), "Hierarchy " << h
                    << " is not valid.");
  }
  //! Return the hierarchies that the Selection was constructed with
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
  //! Get the selected particles
  ParticlesTemp get_selected_particles() const;
  IMP_SHOWABLE(Selection);
};

IMP_VALUES(Selection, Selections);

/** Create a distance restraint between the selections.
 */
IMPATOMEXPORT Restraint* create_distance_restraint(const Selection &n0,
                                                   const Selection &n1,
                                                   double x0, double k);


/** Create a restraint connecting the selections.*/
IMPATOMEXPORT Restraint* create_connectivity_restraint(const Selections &s,
                                                       double k);


/** Create an excluded volume restraint for the included molecules. If a
    value is provided for resolution, then something less than the full
    resolution representation will be used.
 */
IMPATOMEXPORT Restraint* create_excluded_volume_restraint(const Hierarchies &hs,
                                                          double resolution=-1);




/** Set the mass, radius, residues, and coordinates to approximate the passed
    particles.
 */
IMPATOMEXPORT void setup_as_approximation(Particle *h,
                                          const ParticlesTemp &other
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution=-1
#endif
);

/** Set the mass, radius, residues, and coordinates to approximate the passed
    particle based on the leaves of h.
 */
IMPATOMEXPORT void setup_as_approximation(Hierarchy h
#ifndef IMP_DOXYGEN
                                          ,
                                          double resolution =-1
#endif
);


/** Transform a hierarchy. This is aware of rigid bodies.
 */
IMPATOMEXPORT void transform(Hierarchy h, const algebra::Transformation3D &tr);

IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_HIERARCHY_TOOLS_H */
