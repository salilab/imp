/**
 * \file RotamerCalculator.h
 * \brief Object performing rotations by Chi angles.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPROTAMER_ROTAMER_CALCULATOR_H
#define IMPROTAMER_ROTAMER_CALCULATOR_H

#include <vector>
#include <limits>
#include <IMP/PairScore.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Residue.h>
#include <IMP/rotamer/RotamerLibrary.h>

IMPROTAMER_BEGIN_NAMESPACE

//! A class storing the rotated coordinates of the atoms in the residue
class IMPROTAMEREXPORT ResidueRotamer
{
public:
  ResidueRotamer(const IMP::atom::ResidueType &rt = IMP::atom::UNK)
    : size_(0)
    , residue_type_(rt)
  {}

  //! get coordinates of the specified atom
  /** \param[in] index the version of the coordinates (index 0 gives the
       original coordinates, index 1 the most probable coordinates, index 2
       the second most probable coordinates, etc)
      \param[in] at the requested atom
  */
  const IMP::algebra::Vector3D &get_coordinates(unsigned index,
      const IMP::atom::AtomType &at) const;

  //! get number of coordinate sets for a given atom
  /** \param[in] at the requested atom
  */
  unsigned get_number_of_cases(const IMP::atom::AtomType &at) const;

  //! check if a given atom is present in the data structure
  /** \param[in] at the requested atom
  */
  bool get_atom_exists(const IMP::atom::AtomType &at) const;

  //! get number of rotamers
  unsigned get_size() const
  {
    return size_;
  }


  //! get probability associated with the given rotamer
  /** \param[in] index the version of the coordinates (index 1 gives
      the coordinates of the first rotamer, etc)
  */
  double get_probability(unsigned index) const;

  IMP_SHOWABLE_INLINE(ResidueRotamer, {
      out << "ResidueRotamer"; });

private:
  struct Box3D
  {
    Box3D()
    {
      clear();
    }
    void clear()
    {
      xmin = ymin = zmin = std::numeric_limits<double>::max();
      xmax = ymax = zmax = -std::numeric_limits<double>::max();
    }
    double xmin, xmax;
    double ymin, ymax;
    double zmin, zmax;
  };

  static bool intersect(const Box3D &b1, const Box3D &b2);

  typedef std::vector<Box3D> Boxes3D;

  void create_bounding_boxes(Box3D &bb_box, Box3D &sc_box, Boxes3D &rot_boxes);

  void add_coordinates(const IMP::atom::AtomType &at,
      const IMP::algebra::Vector3D &coords);
  void push_coordinates();
  IMP::algebra::Vector3D &get_coordinates(const IMP::atom::AtomType &at);
  void set_coordinates(unsigned index,
      IMP::atom::Residue &rd) const;

  friend class RotamerCalculator;

  typedef std::vector<IMP::algebra::Vector3D> AtomCoordinates;
  typedef std::vector<AtomCoordinates> ResidueCoordinates;
  ResidueCoordinates residue_coordinates_;
  std::vector<double> probabilities_;
  unsigned size_;
  IMP::atom::ResidueType residue_type_;
};


IMP_VALUES(ResidueRotamer, ResidueRotamers);


//! A class performing the rotations of atoms in the residues
class IMPROTAMEREXPORT RotamerCalculator
  : public IMP::base::Object
{
public:
  //! constructor
  /** \param[in] rl an instance of RotamerLibrary
  */
  RotamerCalculator(const RotamerLibrary *rl);

  //! get the coordinates of the residue atoms
  /** this function performs the rotations of the atoms in the given residue
      with the cumulative probability thr
      \param[in] rd the residue to be rotated
      \param[in] thr probability threshold
  */
  ResidueRotamer get_rotamer(const IMP::atom::Residue &rd, double thr) const;

  IMP_OBJECT_METHODS(RotamerCalculator);

  //! set coordinates of side chains of the given protein
  /** \param[in] protein the protein to set coordinates based on most likely
      rotamers
      \param[in] score scoring function to use
      \param[in] thr query threshold
      \param[in] K parameter in equation (42)
      \param[in] num_iter maximum number of iterations (suggested: 6)
  */
  void transform(const IMP::atom::Hierarchy &protein,
      const IMP::PairScore *score, double thr,
      double K, int num_iter) const;

private:
  struct ResidueData
  {
    ResidueData()
      : n_angles(0)
    {}

    bool empty() const
    {
      return n_angles == 0;
    }

    int n_angles;
    std::vector<IMP::atom::AtomType> at_axes;
    std::vector<char> rot_atoms;
  };

  IMP::OwnerPointer<const RotamerLibrary> rl_;
  std::vector<ResidueData> residues_;
};

IMPROTAMER_END_NAMESPACE

#endif /* IMPROTAMER_ROTAMER_CALCULATOR_H */
