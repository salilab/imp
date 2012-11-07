/**
 * \file RotamerLibrary.h
 * \brief Object representing rotamer library.
 *
 * Copyright 2007-2012 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPROTAMER_ROTAMER_LIBRARY_H
#define IMPROTAMER_ROTAMER_LIBRARY_H

#include <string>
#include <vector>
#include <IMP/base/Object.h>
#include <IMP/atom/Residue.h>
#include "rotamer_config.h"

IMPROTAMER_BEGIN_NAMESPACE

//! A simple class storing chi angles and their probability
class IMPROTAMEREXPORT RotamerAngles
{
public:
  //! default constructor. Build identity roattions with zero prbability
  RotamerAngles()
    : chi1_(0)
    , chi2_(0)
    , chi3_(0)
    , chi4_(0)
    , probability_(0)
  {
  }

  //! constructor. build rotamer data corresponding to 1 line from
  //the library file
  RotamerAngles(float chi1, float chi2, float chi3, float chi4,
      float probability)
    : chi1_(chi1)
    , chi2_(chi2)
    , chi3_(chi3)
    , chi4_(chi4)
    , probability_(probability)
  {
  }

  //! query the chi1 angle
  float get_chi1() const
  {
    return chi1_;
  }

  //! query the chi2 angle
  float get_chi2() const
  {
    return chi2_;
  }

  //! query the chi3 angle
  float get_chi3() const
  {
    return chi3_;
  }

  //! query the chi4 angle
  float get_chi4() const
  {
    return chi4_;
  }

  //! query the probability
  float get_probability() const
  {
    return probability_;
  }

  IMP_SHOWABLE_INLINE(RotamerAngles, {
      out << "RotamerAngles: " << chi1_ << ' '
        << chi2_ << ' '
        << chi3_ << ' ' << chi4_ << ' '
        << probability_; });

private:
  float chi1_;
  float chi2_;
  float chi3_;
  float chi4_;
  float probability_;
};


IMP_VALUES(RotamerAngles, ResidueRotamers);


//! A class storing a whole rotamer library read from a file
class IMPROTAMEREXPORT RotamerLibrary
  : public IMP::base::Object
{
public:
  //! constructor. Build an empty library object
  /** \param[in] angle_step bucket size in degrees */
  RotamerLibrary(unsigned angle_step = 10);

  typedef ResidueRotamers::const_iterator RotamerIterator;

  //! query the rotamer library for the rotamer data
  /** \param[in] residue the residue to query about
      \param[in] phi first backbone angle
      \param[in] psi second backbone angle
      \param[in] probability_thr threshold on the sum of probabilities.
      This function returns a pair of iterators to the queried contents.
      The iterators can be used in the following way:
      \code{.cpp}
      RotamerLibrary rl;
      // ....
      RotamerLibrary::RotamerIterator begin, end;
      boost::tie(begin, end) = rl.get_rotamers(...);
      while ( begin != end )
      {
        const RotamerAngles &ra = *begin++;
        // process ra ...
      }
      \endcode
  */
  std::pair<RotamerIterator, RotamerIterator> get_rotamers(
      IMP::atom::ResidueType residue, float phi, float psi,
      float probability_thr) const;


  //! query the rotamer library for the rotamer data
  /** \param[in] residue the residue to query about
      \param[in] phi first backbone angle
      \param[in] psi second backbone angle
      \param[in] probability_thr threshold on the sum of probabilities.
      This function returns a vector with the queried contents (and is
      therefore slower than get_rotamers). It is however more useful in
      Python code
  */
  ResidueRotamers get_rotamer_vector(
      IMP::atom::ResidueType residue, float phi, float psi,
      float probability_thr) const;

  //! load the library from file
  /** \param[in] lib_file_name file name */
  void read_library_file(const std::string &lib_file_name);

  IMP_OBJECT_INLINE(RotamerLibrary, IMP_UNUSED(out),);

private:
  unsigned backbone_angle_to_index(float phi, float psi) const;

  typedef std::vector<ResidueRotamers> ResidueRotamersByBackbone;
  typedef std::vector<ResidueRotamersByBackbone> RotamersByResidue;
  RotamersByResidue library_;

  unsigned angle_step_;
  unsigned rotamers_by_backbone_size_;
};


IMPROTAMER_END_NAMESPACE

#endif /* IMPROTAMER_ROTAMER_LIBRARY_H */
