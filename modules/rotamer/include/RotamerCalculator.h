/**
 * \file RotamerCalculator.h
 * \brief Object performing rotations by Chi angles.
 *
 * Copyright 2007-2012 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPROTAMER_ROTAMER_CALCULATOR_H
#define IMPROTAMER_ROTAMER_CALCULATOR_H

#include <vector>
#include <IMP/algebra/Vector3D.h>
#include <IMP/atom/Atom.h>
#include <IMP/rotamer/RotamerLibrary.h>

IMPROTAMER_BEGIN_NAMESPACE

//! A class storing the rotated coordinates of the atoms in the residue
class IMPROTAMEREXPORT ResidueRotamer
{
public:
  ResidueRotamer() {}

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

  IMP_SHOWABLE_INLINE(ResidueRotamer, {
      out << "ResidueRotamer"; });

private:
  void add_coordinates(const IMP::atom::AtomType &at,
      const IMP::algebra::Vector3D &coords);
  void push_coordinates();
  IMP::algebra::Vector3D &get_coordinates(const IMP::atom::AtomType &at);

  friend class RotamerCalculator;

  typedef std::vector<IMP::algebra::Vector3D> AtomCoordinates;
  typedef std::vector<AtomCoordinates> ResidueCoordinates;
  ResidueCoordinates residue_coordinates_;
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

  IMP_OBJECT_INLINE(RotamerCalculator, IMP_UNUSED(out),);

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

  IMP::Pointer<const RotamerLibrary> rl_;
  std::vector<ResidueData> residues_;
};

IMPROTAMER_END_NAMESPACE

#endif /* IMPROTAMER_ROTAMER_CALCULATOR_H */
