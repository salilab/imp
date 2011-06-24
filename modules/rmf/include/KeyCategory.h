/**
 *  \file IMP/rmf/KeyCategory.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPRMF_KEY_CATEGORY_H
#define IMPRMF_KEY_CATEGORY_H

#include "rmf_config.h"
#include <IMP/macros.h>
#include <IMP/exception.h>
#include <vector>

IMPRMF_BEGIN_NAMESPACE
/** The category for a key. */
class IMPRMFEXPORT KeyCategory {
  int i_;
  KeyCategory(unsigned int i): i_(i){}
public:
  KeyCategory(): i_(-1){}
  unsigned int get_index() const {
    IMP_USAGE_CHECK(i_ >=0, "Invalid KeyCategory used");
    return i_;
  }
  static KeyCategory add_key_category(std::string name);
  std::string get_name() const;
  unsigned int get_offset() {
    // int, string, float
    return 3*i_;
  }
  IMP_HASHABLE_INLINE(KeyCategory, return i_);
  IMP_COMPARISONS_1(KeyCategory, i_);
  void show(std::ostream &out) const {
    out << get_name();
  }
};

IMP_VALUES(KeyCategory, KeyCategories);

/** Standard physical keys are
    - "cartesian_x", "cartesian_y", "cartesian_z" for
    cartesian coordinates in angstroms
    - "radius" for radius in angstroms
    - "mass" for mass in daltons
 */
IMPRMFEXPORT extern const KeyCategory Physics;

/** Standard sequence keys are:
    - "begin residue index", "beyond residue index" for one
    or more residue indexes eg [begin_residue_index, beyond_residue_index)
    - "element" for the element by atomic number
    - "seconard structure" 1 for helix, 2 for strand
    - "sequence" store the sequence as a series of space-deliminted 3-letter
    codes
    - "residue type" store the three letter code for the residue type
    - "chain id" stores an integer for the chain identifier. When translating
    from pdb files, the position in the alphabet of the chain should probably
    be used (eg, chain 'A' is 0)
*/
IMPRMFEXPORT extern const KeyCategory Sequence;


/** Float keys are
    - cartesian_x,cartesian_y,cartesian_z,radius
    - rgb_color_red, rgb_color_blue, rgb_color_green

    Int keys are
    - shape which is
        - 1 for sphere described by cartesian_x0,cartesian_y0,cartesian_z0,
        radius
        - 2 for cylinder, described by cartesian_x0,cartesian_y0,cartesian_z0,
        cartesian_x1,cartesian_y1,cartesian_z1, r
 */
IMPRMFEXPORT extern const KeyCategory Shape;


/** Float keys are
    - score
    Index keys are:
    - representation{i} for the representation nodes involved
*/
IMPRMFEXPORT extern const KeyCategory Feature;
IMPRMF_END_NAMESPACE

#endif /* IMPRMF_KEY_CATEGORY_H */
