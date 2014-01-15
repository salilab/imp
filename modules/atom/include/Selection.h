/**
 *  \file IMP/atom/Selection.h
 *  \brief A set of useful functionality on IMP::atom::Hierarchy decorators
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SELECTION_H
#define IMPATOM_SELECTION_H

#include <IMP/atom/atom_config.h>
#include "Atom.h"
#include "Hierarchy.h"
#include "Residue.h"
#include <IMP/SingletonPredicate.h>
#include <IMP/algebra/algebra_config.h>
#include <IMP/base/InputAdaptor.h>
#include <IMP/core/Typed.h>
#include <IMP/core/XYZR.h>
#include <IMP/display/declare_Geometry.h>

IMPATOM_BEGIN_NAMESPACE

/** Select a part of an atom.Hiearchy or atom.Hierarchies that is identified
    by the biological name.


    For example (in python)
    \code
    Selection(hierarchy=h, molecule="myprotein", terminus=Selection.C)
    Selection(hierarchy=h, molecule="myprotein", residue_index=133)
    Selection(hierarchy=h, molecule="myprotein", residue_indexes=range(133,138))
    \endcode
    each get the C-terminus of the protein "myprotein" (assuming the last
    residue index is 133).

    \note Only representational particles are selected. That is, ones
    with x,y,z coordinates. And the highest resolution representation
    that fits is returned. If you want lower resolution, use the
    resolution parameter to select the desired resolution (pass a very large
    number to get the coarsest representation).
*/
class IMPATOMEXPORT Selection :
#ifdef SWIG
    public kernel::ParticleIndexesAdaptor
#else
    public base::InputAdaptor
#endif
    {
 public:
  enum Terminus {
    NONE,
    C,
    N
  };

 private:
  SingletonPredicates predicates_;
  kernel::Model *m_;
  double resolution_;
  int state_;

  kernel::ParticleIndexes h_;
  IMP_NAMED_TUPLE_2(SearchResult, SearchResults, bool, match,
                    kernel::ParticleIndexes, indexes, );
  SearchResult search(kernel::Model *m, kernel::ParticleIndex pi,
                      boost::dynamic_bitset<> parent) const;
  void set_hierarchies(kernel::Model *m, const kernel::ParticleIndexes &pis);

 public:
#ifdef IMP_DOXYGEN
  /** When using python, you have much more control over
      construction due to the use of keyword arguments. You
      can provide any subset of the arguments (although one
      of hierarchy or hierarchies must be provided).
  */
  Selection(Hierarchy hierarchy = None, Hierarchies hierarchies = [],
            Strings molecules = [], Ints residue_indexes = [],
            Strings chains = [], AtomTypes atom_types = [],
            ResidueTypes residue_types = [], Strings domains = [],
            double resolution = 0, std::string molecule = None,
            int residue_index = None, std::string chain = None,
            AtomType atom_type = None, ResidueType residue_type = None,
            HierarchyType hierarchy_type = None, Terminus terminus = None,
            std::string domain = None, core::ParticleType particle_type = None,
            core::ParticleTypes particle_types = [], int copy_index = -1,
            Ints copy_indexs = [], int state_index = -1,
            Ints state_indexes = []);
#endif
  Selection();
  Selection(Hierarchy h);
  Selection(kernel::Particle *h);
  Selection(kernel::Model *m, const kernel::ParticleIndexes &pis);
#ifndef SWIG
  Selection(const Hierarchies &h);
#endif
  Selection(const kernel::ParticlesTemp &h);
// for C++
#if !defined(IMP_DOXYGEN) && !defined(SWIG)
  Selection(Hierarchy h, std::string molname, int residue_index);
#endif
  //! Return the hierarchies that the Selection was constructed with
  Hierarchies get_hierarchies() const;
  /** Select based on the molecule name.*/
  void set_molecules(Strings mols);

  /** Select at a Representation node with a resolution close to r.*/
  void set_resolution(double r) { resolution_ = r; }
  /** Select State with the passed index.*/
  void set_state_index(int state) { set_states(Ints(1, state)); }
  /** Select State with the passed index.*/
  void set_state_indexes(Ints states);
  /** Select the n or c terminus.*/
  void set_terminus(Terminus t);
  /** Select particles in chains whose id is
      in the passed string.*/
  void set_chains(Strings chains);
  /** Select residues whose indexes are in the passed list.*/
  void set_residue_indexes(Ints indexes);
  /** Select atoms whose types are in the list, eg AT_CA.*/
  void set_atom_types(AtomTypes types);
  /** Select residues whose types are in the list. Not sure
      why you would do this.*/
  void set_residue_types(ResidueTypes types);
  /** Select domains with the specificed names. */
  void set_domains(Strings names);
  /** Select a molecule with the passed name. */
  void set_molecule(std::string mol);
  /** Select a chain with the passed id*/
  void set_chain(std::string c);
#ifndef SWIG
  /** \deprecated_at{2.2} Pass a string */
  IMPATOM_DEPRECATED_FUNCTION_DECL(2.2)
  void set_chain(char c) { set_chain(std::string(1, c)); }
#endif
  /** Select only residues with the passed index.*/
  void set_residue_index(int i);
  /** Select atoms with only the passed type. */
  void set_atom_type(AtomType types);
  /** Select only residues with the passed type.*/
  void set_residue_type(ResidueType type);
  /** Select only the single domain with that name*/
  void set_domain(std::string name);
  /** Select elements with Copy::get_copy_index() that match.*/
  void set_copy_index(unsigned int copy);
  /** Select elements with Copy::get_copy_index() that are in the list.*/
  void set_copy_indexes(Ints copies);
  /** Select elements that match the core::ParticleType.*/
  void set_particle_type(core::ParticleType t);
  /** Select elements that match the core::ParticleType.*/
  void set_particle_types(core::ParticleTypes t);
  /** Select only particles whose type matches the passed type, eg
      Molecule, Fragment, Residue etc. See GetByType for how to
      specify the types. Ints are used to make swig happy.*/
  void set_hierarchy_types(Ints types);
  //! Get the selected particles
  kernel::ParticlesTemp get_selected_particles() const;
  //! Get the indexes of the selected particles
  kernel::ParticleIndexes get_selected_particle_indexes() const;

#ifndef SWIG
  operator ParticleIndexes() const { return get_selected_particle_indexes(); }
  operator ParticlesTemp() const { return get_selected_particles(); }
#endif

  IMP_SHOWABLE(Selection);
};

IMP_VALUES(Selection, Selections);

/** Create a distance restraint between the selections.

    This restraint applies a harmonic to the minimum distance
    between a particle in selection n0 and a particle in selection
    n1.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    See Selection
 */
IMPATOMEXPORT kernel::Restraint *create_distance_restraint(const Selection &n0,
                                                           const Selection &n1,
                                                           double x0, double k,
                                                           std::string name =
                                                               "Distance%1%");

/** Create a restraint connecting the selections.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    See Selection
*/
IMPATOMEXPORT kernel::Restraint *create_connectivity_restraint(
    const Selections &s, double k, std::string name = "Connectivity%1%");

/** Create a restraint connecting the selections. The particles are
 allowed to be appart by x0 and still count as connected.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
     See Selection
*/
IMPATOMEXPORT kernel::Restraint *create_connectivity_restraint(
    const Selections &s, double x0, double k,
    std::string name = "Connectivity%1%");

/** Create a restraint connecting the selection.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.

    See Selection
*/
IMPATOMEXPORT kernel::Restraint *create_internal_connectivity_restraint(
    const Selection &s, double k, std::string name = "Connectivity%1%");

/** Create a restraint connecting the selection. The particles are
 allowed to be appart by x0 and still count as connected.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.

 See Selection
*/
IMPATOMEXPORT kernel::Restraint *create_internal_connectivity_restraint(
    const Selection &s, double x0, double k,
    std::string name = "Connectivity%1%");

/** Create an XYZR particle which always includes the particles
    in the selection in its bounding volume. If all the particles
    in the selection are part of the same rigid body, then the
    created particle is added as part of that rigid body. Otherwise
    it uses an IMP::core::Cover to maintain the cover property.

    Doing this can be a useful way to accelerate computations
    when it is OK to replace a potential complicated set of
    geometry represented by the selection with a much simpler
    one.

    See Selection
*/
IMPATOMEXPORT core::XYZR create_cover(const Selection &s,
                                      std::string name = std::string());

/** Get the total mass of a hierarchy. In daltons.
    See Selection
 */
IMPATOMEXPORT double get_mass(const Selection &s);

#ifdef IMP_ALGEBRA_USE_IMP_CGAL
/** Get the total volume of a hierarchy. In cubic angstroms.
    \requires{get_volume, CGAL}
    See Selection
*/
IMPATOMEXPORT double get_volume(const Selection &s);

/** Get the total surface area of a hierarchy. In square angstroms.
    \requires{get_surface_area, CGAL}
    See Selection
*/
IMPATOMEXPORT double get_surface_area(const Selection &s);
#endif

/**     See Selection
 */
IMPATOMEXPORT double get_radius_of_gyration(const Selection &s);

/** Create an excluded volume restraint for a list of selections.*/
IMPATOMEXPORT kernel::Restraint *create_excluded_volume_restraint(
    const Selections &s);

/** See Hierarchy */
IMPATOMEXPORT Hierarchies get_leaves(const Selection &h);

/** \class SelectionGeometry
    \brief Display a Selection.
*/
class IMPATOMEXPORT SelectionGeometry : public display::Geometry {
  atom::Selection res_;
  mutable IMP::base::map<kernel::Particle *, base::Pointer<Geometry> >
      components_;

 public:
  SelectionGeometry(atom::Selection d, std::string name = "Selection")
      : display::Geometry(name), res_(d) {}
  display::Geometries get_components() const;
  IMP_OBJECT_METHODS(SelectionGeometry);
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SELECTION_H */
