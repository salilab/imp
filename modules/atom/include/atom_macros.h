/**
 *  \file IMP/atom/atom_macros.h    \brief Various important macros
 *                           for implementing decorators.
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_MACROS_H
#define IMPATOM_MACROS_H

#define IMP_ATOM_TYPE_INDEX 8974343
#define IMP_RESIDUE_TYPE_INDEX 90784334
#define IMP_HIERARCHY_TYPE_INDEX 90784335
//! Define the basic things you need for a ForceFieldParameters.
/** In addition to the methods done by all the macros, it declares
    - IMP::Restraint::evaluate()
    - IMP::Restraint::incremental_evaluate()
    and it defines
    - IMP::Restraint::get_is_incremental() to return true
*/
#define IMP_FORCE_FIELD_PARAMETERS(Name) IMP_OBJECT_NO_WARNING(Name)

//! Define the basic things you need for a pdb selector
/** In addition to the methods defined/declared by IMP_OBJECT,
    it defines:
    - IMP::PDBSelector::get_is_selected()
    The selected argument should return true or false and use
    a string called pdb_line.
 */
#define IMP_PDB_SELECTOR(Name, parent, selected, show)                  \
  IMPATOM_DEPRECATED_MACRO(2.1, "Expand the macro in place.");          \
  Name(std::string name) : parent(name) {}                              \
  Name() : parent(std::string(#Name) + "%1%") {}                        \
  bool get_is_selected(const std::string& pdb_line) const { selected; } \
  IMP_OBJECT_METHODS(Name)

//! Define the basic things you need for a mol2 selector
/** In addition to the methods defined/declared by IMP_OBJECT,
    it defines:
    - IMP::Mol2Selector::get_is_selected()
    The selected argument should return true or false and use
    a string called pdb_line.
 */
#define IMP_MOL2_SELECTOR(Name, selected, show)                          \
  bool get_is_selected(const std::string& mol2_line) const { selected; } \
  IMP_OBJECT_METHODS(Name)

/** Add the methods needed for a Simulator
 */
#define IMP_SIMULATOR(Name)                                               \
  IMPATOM_DEPRECATED_MACRO(2.1, "Expand the macro in place.");            \
                                                                          \
 private:                                                                 \
  virtual void setup(const kernel::ParticleIndexes& ps);                  \
  virtual double do_step(const kernel::ParticleIndexes& sc, double dt);   \
  virtual bool get_is_simulation_particle(kernel::ParticleIndex p) const; \
                                                                          \
 public:
#endif /* IMPATOM_MACROS_H */
