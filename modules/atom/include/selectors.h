/**
 * \file selectors
 * \brief Selectors to extract predefined subsets of ATOM records from PDB files
 *
 * Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
#ifndef IMPATOM_SELECTORS_H
#define IMPATOM_SELECTORS_H

#include "config.h"
#include "internal/pdb.h"

#include <IMP/base_types.h>

IMPATOM_BEGIN_NAMESPACE

/** Selector is a general purpose class used to select records from a PDB
    file. Using descendants of this class one may implement arbitrary
    selection functions with operator() and pass them to PDB reading functions
    for object selection. Simple selectors can be used to build more compilated
    ones.
*/
class IMPATOMEXPORT Selector {
 public:
  virtual bool operator()(const String& pdb_line) const { return true; }
  virtual ~Selector();
};

//! Defines a selector that will pick only C-alpha atoms.
class CAlphaSelector : public Selector {
 public:
  bool operator() (const String& pdb_line) const {
    const String type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'A' && type[3] == ' ');
  }
};

//! Defines a selector that will pick only C-beta atoms.
class CBetaSelector: public Selector {
 public:
  bool operator() (const String& pdb_line) const {
    const String type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == 'B' && type[3] == ' ');
  }
};

//! Defines a selector that will pick only C atoms. (not Ca or Cb)
class CSelector: public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    const String type = internal::atom_type(pdb_line);
    return (type[1] == 'C' && type[2] == ' ' && type[3] == ' ');
  }
};

//! Defines a selector that will pick only N atoms.
class NSelector: public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    const String type = internal::atom_type(pdb_line);
    return (type[1] == 'N' && type[2] == ' ' && type[3] == ' ');
  }
};

//! Defines a selector that will pick every atom.
class AllSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const { return true; }
};

//! Selector that picks atoms of given chains.
class ChainSelector : public Selector {
 public:
  ChainSelector(const String &chains): chains_(chains) {}
  virtual ~ChainSelector() {}
  bool operator()(const String& pdb_line) const {
    for(int i=0; i < (int)chains_.length(); i++) {
      if(internal::atom_chain_id(pdb_line) == chains_[i])
        return true;
    }
    return false;
  }
 private:
  String chains_;
};

//! Selector that check if the line is water record
class WaterSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    const String res_name = internal::atom_residue_name(pdb_line);
    return ((res_name[0]=='H' && res_name[1] =='O' && res_name[2]=='H') ||
            (res_name[0]=='D' && res_name[1] =='O' && res_name[2]=='D'));
  }
};

//! Selector that check if the line is hydrogen record
class HydrogenSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    return (pdb_line[internal::atom_type_field_+1] == 'H' ||
            pdb_line[internal::atom_type_field_+1] == 'D');
  }
};

//! Selector that picks non water and non hydrogen atoms
class NonWaterNonHydrogenSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    WaterSelector w;
    HydrogenSelector h;
    return (! w(pdb_line) && ! h(pdb_line));
  }
};

//! Selector that picks non water atoms
class NonWaterSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    WaterSelector w;
    return( ! w(pdb_line));
  }
};

//! A PDB Selector that picks only Phosphate atoms.
class PSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    const String type = internal::atom_type(pdb_line);
    return (type[1] == 'P' && type[2] == ' ');
  }
};

//! A PDB Selector that ignores all alternative location atoms.
class IgnoreAlternativesSelector : public Selector {
 public:
  bool operator()(const String& pdb_line) const {
    return ((internal::atom_alt_loc_indicator(pdb_line) == ' ') ||
            (internal::atom_alt_loc_indicator(pdb_line) == 'A'));
  }
};

IMPATOM_END_NAMESPACE

#endif /* IMPATOM_SELECTORS_H */
