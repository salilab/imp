/**
 *  \file IMP/atom/Selection.h
 *  \brief A set of useful functionality on IMP::atom::Hierarchy decorators
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPATOM_SELECTION_H
#define IMPATOM_SELECTION_H

#include <IMP/atom/atom_config.h>
#include <IMP/algebra/algebra_config.h>
#include "Hierarchy.h"
#include "Residue.h"
#include "Atom.h"
#include <IMP/base/InputAdaptor.h>
#include <IMP/display/declare_Geometry.h>
#include <IMP/core/Typed.h>
#include <IMP/core/XYZR.h>

IMPATOM_BEGIN_NAMESPACE



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
class IMPATOMEXPORT Selection: public base::InputAdaptor {
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
  Ints copies_;
  core::ParticleTypes types_;
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
            std::string domain=None,
            core::ParticleType particle_type=None,
            core::ParticleTypes particle_types=[],
            int copy_index=-1,
            Ints copy_indexs=[]
            );
#endif
  Selection(): radius_(-1), terminus_(NONE) {
  }
  Selection(Hierarchy h): h_(1, h), radius_(-1), terminus_(NONE) {
    IMP_USAGE_CHECK(h.get_is_valid(true), "Hierarchy " << h
                    << " is not valid.");
  }
  Selection(const Hierarchies& h): h_(h){
    for (unsigned int i=0; i< h.size(); ++i) {
      IMP_USAGE_CHECK(h[i].get_is_valid(true), "Hierarchy " << h[i]
                    << " is not valid.");
    }
    radius_=-1;
    terminus_=NONE;
  }
  Selection(const ParticlesTemp& h): h_(h.begin(), h.end()){
    for (unsigned int i=0; i< h.size(); ++i) {
      IMP_USAGE_CHECK(Hierarchy(h[i]).get_is_valid(true), "Hierarchy " << h[i]
                    << " is not valid.");
    }
    radius_=-1;
    terminus_=NONE;
  }
  // for C++
  Selection(Hierarchy h,
        std::string molname,
            int residue_index): h_(1,h), molecules_(1,molname),
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
  void set_copy_index(unsigned int copy) {
    copies_=Ints(1, copy);
  }
  void set_copy_indexes(const Ints &copies) {
    copies_=copies;
    std::sort(copies_.begin(), copies_.end());
  }
  void set_particle_type(core::ParticleType t) {
    types_= core::ParticleTypes(1,t);
  }
  void set_particle_types(core::ParticleTypes t) {
    types_= t;
    std::sort(types_.begin(), types_.end());
  }
  //! Get the selected particles
  ParticlesTemp get_selected_particles() const;
  IMP_SHOWABLE(Selection);
};

IMP_VALUES(Selection, Selections);

/** Create a distance restraint between the selections.

    This restraint applies a harmonic to the minimum distance
    between a particle in selection n0 and a particle in selection
    n1.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    \relatesalso Selection
 */
IMPATOMEXPORT Restraint* create_distance_restraint(const Selection &n0,
                                                   const Selection &n1,
                                                   double x0, double k);


/** Create a restraint connecting the selections.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
    \relatesalso Selection
*/
IMPATOMEXPORT Restraint* create_connectivity_restraint(const Selections &s,
                                                       double k);

/** Create a restraint connecting the selections. The particles are
 allowed to be appart by x0 and still count as connected.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.
     \relatesalso Selection
*/
IMPATOMEXPORT Restraint* create_connectivity_restraint(const Selections &s,
                                                       double x0, double k);

/** Create a restraint connecting the selection.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.

    \relatesalso Selection
*/
IMPATOMEXPORT Restraint*
create_internal_connectivity_restraint(const Selection &s,
                                       double k);

/** Create a restraint connecting the selection. The particles are
 allowed to be appart by x0 and still count as connected.

    If one or more of the selections is a rigid body, this will be used
    to accelerate the computation.

 \relatesalso Selection
*/
IMPATOMEXPORT Restraint*
create_internal_connectivity_restraint(const Selection &s,
                                       double x0, double k);




/** Create an XYZR particle which always includes the particles
    in the selection in its bounding volume. If all the particles
    in the selection are part of the same rigid body, then the
    created particle is added as part of that rigid body. Otherwise
    it uses an IMP::core::Cover to maintain the cover property.

    Doing this can be a useful way to accelerate computations
    when it is OK to replace a potential complicated set of
    geometry represented by the selection with a much simpler
    one.

    \relatesalso Selection
*/
IMPATOMEXPORT core::XYZR create_cover(const Selection &s,
                                      std::string name=std::string());


/** Get the total mass of a hierarchy. In daltons.
    \relatesalso Selection
 */
IMPATOMEXPORT double get_mass(const Selection &s);


#ifdef IMP_ALGEBRA_USE_IMP_CGAL
/** Get the total volume of a hierarchy. In cubic angstroms.
    \requires{get_volume, CGAL}
    \relatesalso Selection
*/
IMPATOMEXPORT double get_volume(const Selection &s);

/** Get the total surface area of a hierarchy. In square angstroms.
    \requires{get_surface_area, CGAL}
    \relatesalso Selection
*/
IMPATOMEXPORT double get_surface_area(const Selection &s);
#endif

/**     \relatesalso Selection
 */
IMPATOMEXPORT double get_radius_of_gyration(const Selection &s);

/** Create an excluded volume restraint for a list of selections.*/
IMPATOMEXPORT Restraint*
create_excluded_volume_restraint(const Selections &s);


/** \relatesalso Hierarchy */
IMPATOMEXPORT Hierarchies get_leaves(const Selection &h);



/** \class SelectionGeometry
    \brief Display a Selection.
*/
class IMPATOMEXPORT SelectionGeometry: public display::Geometry {
  atom::Selection res_;
  mutable IMP::compatibility::map<Particle*, Pointer<Geometry> > components_;
public:
  SelectionGeometry(atom::Selection d,
                    std::string name="Selection"):
      display::Geometry(name), res_(d) {}
  display::Geometries get_components() const;
  IMP_OBJECT_INLINE(SelectionGeometry,IMP_UNUSED(out);,);
};



IMPATOM_END_NAMESPACE

#endif  /* IMPATOM_SELECTION_H */
