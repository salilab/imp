/**
 *  \file IMP/atom/pdb.h
 *  \brief Functions to read pdbs
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include <IMP/atom/atom_config.h>
#include "Hierarchy.h"
#include "Atom.h"
#include "element.h"
#include "internal/pdb.h"
#include "atom_macros.h"
#include <IMP/file.h>
#include "Selection.h"
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/OptimizerState.h>
#include <IMP/internal/utility.h>
#include <boost/format.hpp>

IMPATOM_BEGIN_NAMESPACE


//! Select which atoms to read from a PDB file
/** Selector is a general purpose class used to select records from a PDB
    file. Using descendants of this class one may implement arbitrary
    selection functions with operator() and pass them to PDB reading functions
    for object selection. Simple selectors can be used to build more complicated
    ones. Inheritence means "AND" unless otherwise noted (that is, the
    CAlphaPDBSelector takes all non-alternate C-alphas since it inherits from
    NonAlternativePDBSelector).

    \see read_pdb
*/
class IMPATOMEXPORT PDBSelector: public IMP::base::Object {
 public:
  PDBSelector(std::string name): Object(name){}
  //! Return true if the line should be processed
  virtual bool get_is_selected(const std::string& pdb_line) const=0;
  virtual ~PDBSelector();
};

IMP_OBJECTS(PDBSelector, PDBSelectors);

//! Select all ATOM and HETATM records which are not alternatives
class NonAlternativePDBSelector : public PDBSelector {
 public:
  IMP_PDB_SELECTOR(NonAlternativePDBSelector, PDBSelector,
                   return (internal::atom_alt_loc_indicator(pdb_line) == ' '
                           || internal::atom_alt_loc_indicator(pdb_line)
                           == 'A'),out << "");
};

//! Select all non-alternative ATOM records
class ATOMPDBSelector: public NonAlternativePDBSelector {
public:
  IMP_PDB_SELECTOR(ATOMPDBSelector,  NonAlternativePDBSelector,
                   return NonAlternativePDBSelector::get_is_selected(pdb_line)
                   && internal::is_ATOM_rec(pdb_line),out << "");
};


//! Select all CA ATOM records
class CAlphaPDBSelector : public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(CAlphaPDBSelector, NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
                     return false;
                   }
                   const std::string type = internal::atom_type(pdb_line);
                   return (type[1] == 'C' && type[2] == 'A'
                           && type[3] == ' '),out << "");
};

//! Select all CB ATOM records
class CBetaPDBSelector: public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(CBetaPDBSelector, NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)){
                     return false;
                   }
                   const std::string type = internal::atom_type(pdb_line);
                   return (type[1] == 'C' && type[2] == 'B'
                           && type[3] == ' '),out << "");
};

//! Select all C (not CA or CB) ATOM records
class CPDBSelector: public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(CPDBSelector,NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
                     return false;
                   }
                   const std::string type = internal::atom_type(pdb_line);
                   return (type[1] == 'C' && type[2] == ' ' && type[3] == ' '),
                   out << ""
                   );
};

//! Select all N ATOM records
class NPDBSelector: public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(NPDBSelector, NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
                     return false;
                   }
                   const std::string type = internal::atom_type(pdb_line);
                   return (type[1] == 'N' && type[2] == ' ' && type[3] == ' '),
                   out << ""
                   );
};

//! Defines a selector that will pick every ATOM and HETATM record
class AllPDBSelector : public PDBSelector {
public:
  IMP_PDB_SELECTOR(AllPDBSelector, PDBSelector,
                   return true || pdb_line.empty(),
                   out << "");
};

//! Select all ATOM and HETATMrecords with the given chain ids
class ChainPDBSelector : public NonAlternativePDBSelector {
 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    for(int i=0; i < (int)chains_.length(); i++) {
      if(internal::atom_chain_id(pdb_line) == chains_[i])
        return true;
    }
    return false;
  }
  IMP_OBJECT_INLINE(ChainPDBSelector,out << chains_,);
   //! The chain id can be any character in chains
  ChainPDBSelector(const std::string &chains,
                   std::string name="ChainPDBSelector%1%"):
    NonAlternativePDBSelector(name), chains_(chains) {}
 private:
  std::string chains_;
};

//! Select all non-water ATOM and HETATMrecords
class WaterPDBSelector : public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(WaterPDBSelector, NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
                     return false;
                   }
                   const std::string res_name
                      = internal::atom_residue_name(pdb_line);
                   return ((res_name[0]=='H' && res_name[1] =='O'
                            && res_name[2]=='H') ||
                           (res_name[0]=='D' && res_name[1] =='O'
                            && res_name[2]=='D')),
                   out << ""
                   );
};

//! Select all hydrogen ATOM and HETATM records
class IMPATOMEXPORT HydrogenPDBSelector : public NonAlternativePDBSelector {
  bool is_hydrogen(std::string pdb_line) const;
 public:
  IMP_PDB_SELECTOR(HydrogenPDBSelector, NonAlternativePDBSelector,
                   return is_hydrogen(pdb_line);,
                   out << "");
};

//! Select non water and non hydrogen atoms
class NonWaterNonHydrogenPDBSelector : public NonAlternativePDBSelector {
  IMP::OwnerPointer<PDBSelector> ws_, hs_;
 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    return (! ws_->get_is_selected(pdb_line)
            && ! hs_->get_is_selected(pdb_line));
  }
  IMP_OBJECT_INLINE(NonWaterNonHydrogenPDBSelector,out << *ws_,);
  NonWaterNonHydrogenPDBSelector(std::string name):
    NonAlternativePDBSelector(name),
    ws_(new WaterPDBSelector()),
    hs_(new HydrogenPDBSelector()){}
  NonWaterNonHydrogenPDBSelector():
    NonAlternativePDBSelector("NonWaterPDBSelector%1%"),
    ws_(new WaterPDBSelector()),
    hs_(new HydrogenPDBSelector()){}
};

//! Select all non-water non-alternative ATOM and HETATM records
class NonWaterPDBSelector : public NonAlternativePDBSelector {
  IMP::OwnerPointer<PDBSelector> ws_;
 public:
  bool get_is_selected(const std::string &pdb_line) const {
    if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
      return false;
    }
    return( ! ws_->get_is_selected(pdb_line));
  }
  IMP_OBJECT_INLINE(NonWaterPDBSelector,out << *ws_,);
  NonWaterPDBSelector(std::string name): NonAlternativePDBSelector(name),
                                         ws_(new WaterPDBSelector()){}
  NonWaterPDBSelector(): NonAlternativePDBSelector("NonWaterPDBSelector%1%"),
                         ws_(new WaterPDBSelector()){}
};

//! Select all P ATOM records
class PPDBSelector : public NonAlternativePDBSelector {
 public:
  IMP_PDB_SELECTOR(PPDBSelector,
                   NonAlternativePDBSelector,
                   if (!NonAlternativePDBSelector::get_is_selected(pdb_line)) {
                     return false;
                   }
                   const std::string type = internal::atom_type(pdb_line);
                   return (type[1] == 'P' && type[2] == ' '),
                   out << "");
};

// these do not work in python as the wrapped selectors get cleaned up
//! Select atoms which are selected by both selectors
/** To use do something like
    \code
    read_pdb(name, m, AndPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode
 */
class AndPDBSelector: public PDBSelector {
  const IMP::OwnerPointer<PDBSelector> a_, b_;
public:
  bool get_is_selected(const std::string &pdb_line) const {
   return a_->get_is_selected(pdb_line)
     && b_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_INLINE(AndPDBSelector,out << *a_ << " and " << *b_,);
  AndPDBSelector( PDBSelector *a, PDBSelector *b):
    PDBSelector("AndPDBSelector%1%"), a_(a), b_(b){}
};

//! Select atoms which are selected by either selector
/** To use do something like
    \code
    read_pdb(name, m, OrPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode
 */
class OrPDBSelector: public PDBSelector {
  const IMP::OwnerPointer<PDBSelector> a_, b_;
public:
  bool get_is_selected(const std::string &pdb_line) const {
   return a_->get_is_selected(pdb_line)
     || b_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_INLINE(OrPDBSelector,out << *a_ << " or " << *b_,);
  OrPDBSelector( PDBSelector *a, PDBSelector *b):
    PDBSelector("OrPDBSelector%1%"), a_(a), b_(b){}
};

//! Select atoms which not selected by a given selector
/** To use do something like
    \code
    read_pdb(name, m, NotPDBSelector(PPDBSelector()));
    \endcode
 */
class NotPDBSelector: public PDBSelector {
  const IMP::OwnerPointer<PDBSelector> a_;
public:
  bool get_is_selected(const std::string &pdb_line) const {
    return !a_->get_is_selected(pdb_line);
  }
  IMP_OBJECT_INLINE(NotPDBSelector,out << "not" << *a_,);
  NotPDBSelector( PDBSelector *a): PDBSelector("NotPDBSelector%1%"),
                                   a_(a){}
};


/** @name PDB Reading
    \anchor pdb_in
   The read PDB methods produce a hierarchy that looks as follows:
    - One Atom per ATOM or HETATM record in the PDB.
    - All Atom particles have a parent which is a Residue.
    - All Residue particles have a parent which is a Chain.

    Waters are currently dropped if they are ATOM records. This can be fixed.

    The read_pdb() functions should successfully parse all valid pdb files. It
    can produce warnings on files which are not valid. It will attempt to read
    such files, but all bets are off.

    When reading PDBs, PDBSelector objects can be used to choose to only process
    certain record types. See the class documentation for more information.
    When no PDB selector is supplied for reading, the
    NonWaterPDBSelector is used.

    Set the IMP::LogLevel to VERBOSE to see details of parse errors.
*/
//!@{

/** Read a all the molecules in the first model of the
    pdb file.
 */
IMPATOMEXPORT Hierarchy read_pdb(base::TextInput input,
                                 Model* model);

/** Rewrite the coordinates of the passed hierarchy based
    on the contents of the first model in the pdb file.

    The hierarchy must have been created by reading from a pdb
    file and the atom numbers must correspond between the files.
    These are not really checked.

    A ValueException is thrown if there are insufficient models
    in the file.

    core::RigidMember particles are handled by updating the
    core::RigidBody algebra::ReferenceFrame3D to align with the
    loaded particles. Bad things will happen if the loaded coordinates
    are not a rigid transform of the prior coordinates.
 */
IMPATOMEXPORT void read_pdb(base::TextInput input,
                            int model,
                            Hierarchy h);


/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
read_pdb(base::TextInput input,
         Model* model,
         PDBSelector* selector,
         bool select_first_model = true
#ifndef IMP_DOXYGEN
         , bool no_radii=false
#endif
         );



/** Read all models from the pdb file.
 */
IMPATOMEXPORT Hierarchies read_multimodel_pdb(base::TextInput input,
                                              Model *model,
                                              PDBSelector* selector
#ifndef IMP_DOXYGEN
                                              , bool noradii=false
#endif
                                              );
/** Read all models from the pdb file.
 */
IMPATOMEXPORT Hierarchies read_multimodel_pdb(base::TextInput input,
                                              Model *model);
/** @} */

/** @name PDB Writing
    \anchor pdb_out
    The methods to write a PDBs expects a Hierarchy that looks as follows:
    - all leaves are Atom particles
    - all Atom particles have Residue particles as parents

    All Residue particles that have a Chain particle as an ancestor
    are considered part of a protein, DNA or RNA, ones without are
    considered heterogens.

    The functions produce files that are not valid PDB files,
    eg only ATOM/HETATM lines are printed for all Atom particles
    in the hierarchy. Complain if your favorite program can't read them and
    we might fix it.
*/
//!@{

/** Write some atoms to a PDB.
*/
IMPATOMEXPORT void write_pdb(const Selection& mhd,
                             base::TextOutput out,
                             unsigned int model=0);

/** \brief Write a hierarchy to a pdb as C_alpha atoms.

    This method is used to write a non-atomic hierarchy into a pdb in a way
    that can be read by most programs. If the leaves are Residue particles
    then the index and residue type will be read from them. Otherwise default
    values will be used so that each leaf ends up in a separate residue.
*/
IMPATOMEXPORT void write_pdb_of_c_alphas(const Selection& mhd,
                                         base::TextOutput out,
                                         unsigned int model=0);

/** Write the hierarchies one per frame.
*/
IMPATOMEXPORT void write_multimodel_pdb(const Hierarchies& mhd,
                                        base::TextOutput out);
/** @} */




#ifndef IMP_DOXYGEN

/**
   This function returns a string in PDB ATOM format
*/
IMPATOMEXPORT std::string get_pdb_string(const algebra::Vector3D& v,
                                     int index = -1,
                                     AtomType at = AT_C,
                                     ResidueType rt = atom::ALA,
                                     char chain = ' ',
                                     int res_index = 1,
                                     char res_icode = ' ',
                                     double occpancy = 1.00,
                                     double tempFactor = 0.00,
                                     Element e = C);

/**
   This function returns a connectivity string in PDB format
  /note The CONECT records specify connectivity between atoms for which
      coordinates are supplied. The connectivity is described using
      the atom serial number as found in the entry.
  /note http://www.bmsc.washington.edu/CrystaLinks/man/pdb/guide2.2_frame.html
*/
IMPATOMEXPORT std::string get_pdb_conect_record_string(int,int);
#endif



/** \class WritePDBOptimizerState
    This writes a PDB file at the specified interval during optimization.
    If the file name contains %1% then a new file is written each time
    with the %1% replaced by the index. Otherwise a new model is written
    each time to the same file.
*/
IMP_MODEL_SAVE(WritePDB, (const atom::Hierarchies& mh, std::string file_name),
               atom::Hierarchies mh_;,
               mh_=mh;,
               ,
               {
                 base::TextOutput to(file_name, append);
                 IMP_LOG_TERSE( "Writing pdb file " << file_name << std::endl);
                 atom::write_pdb(mh_,to, append?call:0);
               });


IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
