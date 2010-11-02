/**
 *  \file atom/pdb.h
 *  \brief Functions to read pdbs
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPATOM_PDB_H
#define IMPATOM_PDB_H

#include "atom_config.h"
#include "Hierarchy.h"
#include "Atom.h"
#include "element.h"
#include "internal/pdb.h"
#include <IMP/file.h>
#include <IMP/Model.h>
#include <IMP/Particle.h>
#include <IMP/FailureHandler.h>
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

    PDBSelectors are designed to be temporary objects and should never be
    stored.
    \see read_pdb
*/
class IMPATOMEXPORT PDBSelector {
 public:
  //! Return true if the line should be processed
  virtual bool operator()(const std::string& pdb_line) const=0;
  virtual ~PDBSelector();
};

//! Select all ATOM and HETATM records which are not alternatives
class NonAlternativePDBSelector : public PDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    return ((internal::atom_alt_loc_indicator(pdb_line) == ' ') ||
            (internal::atom_alt_loc_indicator(pdb_line) == 'A'));
  }
};

//! Select all non-alternative ATOM records
class ATOMPDBSelector: public NonAlternativePDBSelector {
public:
  bool operator()(const std::string& pdb_line) const {
    return NonAlternativePDBSelector::operator()(pdb_line)
      && internal::is_ATOM_rec(pdb_line);
  }
};


//! Select all CA ATOM records
class CAlphaPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator() (const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'A' && type[3] == ' ');
  }
};

//! Select all CB ATOM records
class CBetaPDBSelector: public NonAlternativePDBSelector {
 public:
  bool operator() (const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'B' && type[3] == ' ');
  }
};

//! Select all C (not CA or CB) ATOM records
class CPDBSelector: public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == ' ' && type[3] == ' ');
  }
};

//! Select all N ATOM records
class NPDBSelector: public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'N' && type[2] == ' ' && type[3] == ' ');
  }
};

//! Defines a selector that will pick every ATOM and HETATM record
class AllPDBSelector : public PDBSelector {
 public:
  bool operator()(const std::string&) const { return true; }
};

//! Select all ATOM and HETATMrecords with the given chain ids
class ChainPDBSelector : public NonAlternativePDBSelector {
 public:
  //! The chain id can be any character in chains
  ChainPDBSelector(const std::string &chains): chains_(chains) {}
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    for(int i=0; i < (int)chains_.length(); i++) {
      if(internal::atom_chain_id(pdb_line) == chains_[i])
        return true;
    }
    return false;
  }
 private:
  std::string chains_;
};

//! Select all non-water ATOM and HETATMrecords
class WaterPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string res_name = internal::atom_residue_name(pdb_line);
    return ((res_name[0]=='H' && res_name[1] =='O' && res_name[2]=='H') ||
            (res_name[0]=='D' && res_name[1] =='O' && res_name[2]=='D'));
  }
};

//! Select all hydrogen ATOM and HETATM records
class HydrogenPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    std::string elem = internal::atom_element(pdb_line);
    boost::trim(elem);
    // determine if the line is hydrogen atom as follows:
    // 1. if the record has element field (columns 76-77),
    // check that it is indeed H. Note that it may be missing in some files.
    // some programms do not output element, so the ATOM line can be shorter.
    if(elem.length() == 1 && elem[0]=='H') return true;
    // 2. if no hydrogen is found in the element record, try atom type field.
    // some NMR structures have 'D' for labeled hydrogens
    std::string atom_name = internal::atom_type(pdb_line);
    return (// " HXX" or " DXX" or "1HXX" ...
            ((atom_name[0] == ' ' || isdigit(atom_name[0])) &&
             (atom_name[1] == 'H' || atom_name[1] == 'D')) ||
            // "HXXX" or "DXXX"
            (atom_name[0] == 'H' || atom_name[0] == 'D'));
  }
};

//! Select non water and non hydrogen atoms
class NonWaterNonHydrogenPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    WaterPDBSelector w;
    HydrogenPDBSelector h;
    return (! w(pdb_line) && ! h(pdb_line));
  }
};

//! Select all non-water non-alternative ATOM and HETATM records
class NonWaterPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    WaterPDBSelector w;
    return( ! w(pdb_line));
  }
};

//! Select all P ATOM records
class PPDBSelector : public NonAlternativePDBSelector {
 public:
  bool operator()(const std::string& pdb_line) const {
    if (!NonAlternativePDBSelector::operator()(pdb_line)) return false;
    const std::string type = internal::atom_type(pdb_line);
    return (type[1] == 'P' && type[2] == ' ');
  }
};

#if 0
// these do not work in python as the wrapped selectors get cleaned up
//! Select atoms which are selected by both selectors
/** To use do something like
    \code
    read_pdb(name, m, AndPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode
 */
class AndPDBSelector: public PDBSelector {
  const PDBSelector &a_, &b_;
public:
  AndPDBSelector(const PDBSelector &a, PDBSelector &b): a_(a), b_(b){}
  bool operator()(const std::string &pdb_line) const {
    return a_(pdb_line) && b_(pdb_line);
  }
};

//! Select atoms which are selected by either selector
/** To use do something like
    \code
    read_pdb(name, m, OrPDBSelector(PPDBSelector(), WaterPDBSelector()));
    \endcode
 */
class OrPDBSelector: public PDBSelector {
  const PDBSelector &a_, &b_;
public:
  OrPDBSelector(const PDBSelector &a, PDBSelector &b): a_(a), b_(b){}
  bool operator()(const std::string &pdb_line) const {
    return a_(pdb_line) || b_(pdb_line);
  }
};

//! Select atoms which not selected by a given selector
/** To use do something like
    \code
    read_pdb(name, m, NotPDBSelector(PPDBSelector()));
    \endcode
 */
class NotPDBSelector: public PDBSelector {
  const PDBSelector &a_;
public:
  NotPDBSelector(const PDBSelector &a): a_(a){}
  bool operator()(const std::string &pdb_line) const {
    return !a_(pdb_line);
  }
};
#endif


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

    Set the IMP::LogLevel to IMP::VERBOSE to see details of parse errors.
*/
//!@{

/** Read a all the molecules in the first model of the
    pdb file.

    \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy read_pdb(TextInput in,
                                 Model* model);

/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchy
read_pdb(TextInput in,
         Model* model,
         const PDBSelector& selector,
         bool select_first_model = true
#ifndef IMP_DOXYGEN
         , bool no_radii=false
#endif
         );



/** \relatesalso Hierarchy
 */
IMPATOMEXPORT Hierarchies read_multimodel_pdb(TextInput in,
                                              Model *model,
                                              const PDBSelector& selector);
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

/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(Hierarchy mhd,
                             TextOutput out);
/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_pdb(const Hierarchies &mhd,
                             TextOutput out);

/** \relatesalso Hierarchy
*/
IMPATOMEXPORT void write_multimodel_pdb(
                        const Hierarchies& mhd, TextOutput out);

/** @} */


#ifndef IMP_DOXYGEN

/**
   This function returns a string in PDB ATOM format
*/
IMPATOMEXPORT std::string pdb_string(const algebra::VectorD<3>& v,
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
IMPATOMEXPORT std::string conect_record_string(int,int);
#endif



/** \class WritePDBOptimizerState
    This writes a PDB file at the specified interval during optimization.
    The file name should not contain
    %1% (if it does not, the same file will be overwritten each time).

    \class WritePDBFailureHandler
    Write a PDB when an error occurs.

    \requires{class WriteBinaryFailureHandler, NetCDF}
*/
IMP_MODEL_SAVE(WritePDB, (const atom::Hierarchies& mh, std::string file_name),
               atom::Hierarchies mh_;,
               mh_=mh;,
               ,
               {
                 IMP_LOG(TERSE, "Writing pdb file " << file_name << std::endl);
                 atom::write_pdb(mh_,file_name);
               });


IMPATOM_END_NAMESPACE

#endif /* IMPATOM_PDB_H */
