/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_KEY_CATEGORY_H
#define IMPLIBRMF_KEY_CATEGORY_H

#include "RMF_config.h"
#include "infrastructure_macros.h"
#include <vector>

namespace RMF {

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
namespace internal {
  class SharedData;
}
#endif
class Category;

/** The category for a key. */
class RMFEXPORT Category {
  int i_;
  friend class RootHandle;
  friend class internal::SharedData;
  static Category get_category(std::string name);
  int compare(const Category &o) const {
    if (i_ < o.i_) return -1;
    else if (i_ > o.i_) return 1;
    else return 0;
  }
public:
#ifndef IMP_DOXYGEN
  Category(unsigned int i): i_(i){}
#endif
  Category(): i_(-1){}
  unsigned int get_index() const {
    IMP_RMF_USAGE_CHECK(i_ >=0, "Invalid Category used");
    return i_;
  }
  std::string get_name() const;
  unsigned int get_offset() {
    // int, string, float
    return 3*i_;
  }
  IMP_RMF_HASHABLE(Category, return i_);
  IMP_RMF_COMPARISONS(Category);
  void show(std::ostream &out) const {
    out << get_name();
  }
};

typedef std::vector<Category> Categories;
#ifndef SWIG
inline std::ostream &operator<<(std::ostream &out, const Category &nh) {
  nh.show(out);
  return out;
}
#endif

//! Keys for storing physics-based attributes of the node
/** \rmfattributetable_begin{Physics}
    \rmfattribute{cartesian x, Float, the x coordinate in angstrom, }
    \rmfattribute{cartesian y, Float, the y coordinate in angstrom,
    required if other cartesian coordinates are included}
    \rmfattribute{cartesian z, Float, the z coordinate in angstrom,
    required if other cartesian coordinates are included}
    \rmfattribute{radius, Float,
    the radius of the volume occupied by the entity in angstrom,
    positive; requires coordinates }
    \rmfattribute{mass, Float, the mass of the node in Daltons,
    positive; requires coordinates}
    \rmfattribute{diffusion coefficient, Float,
    the diffusion coefficient for the node in angstroms per squared femtosecond,
    positive}
    \rmfattributetable_end
 */
RMFEXPORT extern const Category physics;

//! Keys for storing information about protein and nucleotide sequence
/** \rmfattributetable_begin{Sequence}
    \rmfattribute{first residue index, Int,
    the first residue index included in this node,
    any descendent that has an index must be in the specified range}
    \rmfattribute{last residue index, Int,
    the last residue index included in this node,
    must be greater than begin residue index}
    \rmfattribute{element, Index, the atomic number of the element,
    nodes with elements cannot have children}
    \rmfattribute{secondary structure, Index,
    0 for unstructured; 1 for helix; 2 for strand,
    node must have a residue type}
    \rmfattribute{sequence, Strings,
    store the sequence as a list of three letter codes,
    if the index range is specified then the lengths must match;
    children with residue types must match}
    \rmfattribute{residue type, String,
    the three letter code for the residue type,
    this must match any sequence information higher in the tree and no
    descendent must have a residue type}
    \rmfattribute{chain id, Index,
    stores an integer for the chain identifier. When translating from
    pdb files the position in the alphabet of the chain should probably
    be used (eg chain 'A' is 0), }
    \rmfattributetable_end
*/
RMFEXPORT extern const Category sequence;

//! Information about geometric shapes
/**
    \rmfattributetable_begin{Shape}
    \rmfattribute{cartesian x, Float, the x coordinate,
    must have y and z and no coordinates fields}
    \rmfattribute{cartesian y, Float, the y coordinate,
    must have x and z and no coordinates fields}
    \rmfattribute{cartesian z, Float, the z coordinate,
    must have x and y and no coordinates fields}
    \rmfattribute{cartesian xs, Floats, the x coordinates,
    the number must match that of the y and z coordinates}
    \rmfattribute{cartesian ys, Floats, the y coordinates,
    the number must match that of the x and z coordinates}
    \rmfattribute{cartesian zs, Floats, the z coordinates,
    the number must match that of the x and y coordinates}
    \rmfattribute{radius, Float, the radius, positive}
    \rmfattribute{rgb red, Float, the red channel for the color,
    between 0 and 1; must have green and blue}
    \rmfattribute{rgb green, Float, the green channel for the color,
    between 0 and 1; must have red and blue}
    \rmfattribute{rgb blue, Float, the blue channel for the color,
    between 0 and 1; must have red and green}
    \rmfattribute{triangles, Indexes,
    indexes into the coordinates fields to define triangular faces,
    indexes must be less than the coordinates length;
    the number must be divisible by 3}
    \rmfattribute{shape, Index, 0 for sphere(s); 1 for cylinder(s);
    2 for surface,
    appropriate attributes must be there;
    if cylinders or spheres there must be a radius;
    if cylinder and the coordinates fields are used the size
    must be divisible by 2}
    \rmfattributetable_end
 */
RMFEXPORT extern const Category shape;

//! Keys for providing markup of parts of the structure
/** \rmfattributetable_begin{Feature}
    \rmfattribute{score, Float, the score, }
    \rmfattribute{representation, NodeIDs, the nodes involved in the feature, }
    \rmfattributetable_end
*/
RMFEXPORT extern const Category feature;

//! Store information about associated publications
/** \rmfattributetable_begin{Publication}
    \rmfattribute{title, String, article title, }
    \rmfattribute{book title, String, the book title, }
    \rmfattribute{chapter, String, the book chapter, must have book title}
    \rmfattribute{journal, String, the name of the journal, }
    \rmfattribute{url, String, a url for the publication, }
    \rmfattribute{pubmed id, Int, the pubmed id, }
    \rmfattribute{year, Int, the year of publication, }
    \rmfattribute{first page, Index, the first page number, }
    \rmfattribute{last page, Index, the last page number, }
    \rmfattribute{author, Strings,
    the author names as a list for "Firstname Lastname", }
    \rmfattributetable_end
*/
RMFEXPORT extern const Category publication;

#ifndef IMP_DOXYGEN
  RMFEXPORT extern const Category Physics;
  RMFEXPORT extern const Category Sequence;
  RMFEXPORT extern const Category Shape;
  RMFEXPORT extern const Category Feature;
  RMFEXPORT extern const Category Publication;
#endif

} /* namespace RMF */

#endif /* IMPLIBRMF_KEY_CATEGORY_H */
