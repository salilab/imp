/**
 * \file RotamerLibrary.h
 * \brief Object representing rotamer library.
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPROTAMER_ROTAMER_LIBRARY_H
#define IMPROTAMER_ROTAMER_LIBRARY_H

#include <string>
#include <vector>
#include <boost/range/iterator_range.hpp>
#include <IMP/base/Object.h>
#include <IMP/atom/Residue.h>
#include <IMP/rotamer/rotamer_config.h>

IMPROTAMER_BEGIN_NAMESPACE

//! A simple class storing chi angles and their probability
class IMPROTAMEREXPORT RotamerAngleTuple
{
public:
  //! default constructor. Build identity rotations with zero probability
  RotamerAngleTuple()
    : chi1_(0)
    , chi2_(0)
    , chi3_(0)
    , chi4_(0)
    , probability_(0)
  {
  }

  //! constructor. build rotamer data corresponding to 1 line from
  //the library file
  RotamerAngleTuple(float chi1, float chi2, float chi3, float chi4,
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

  IMP_SHOWABLE_INLINE(RotamerAngleTuple, {
      out << "RotamerAngleTuple: " << chi1_ << ' '
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


IMP_VALUES(RotamerAngleTuple, RotamerAngleTuples);


//! A class storing a whole rotamer library read from a file
class IMPROTAMEREXPORT RotamerLibrary
  : public IMP::base::Object
{
public:
  //! constructor. Build an empty library object
  /** \param[in] angle_step bucket size in degrees */
  RotamerLibrary(unsigned angle_step = 10);

#ifndef SWIG
  typedef RotamerAngleTuples::const_iterator RotamerIterator;
  typedef boost::iterator_range<RotamerIterator> RotamerRange;

  //! query the rotamer library for the rotamer data
  /** This function returns a range of iterators to the queried contents.
      The range can be used in the following way:
      \code{.cpp}
      RotamerLibrary rl;
      // ....
      RotamerLibrary::RotamerRange r = rl.get_rotamers_fast(...);
      for ( RotamerLibrary::RotamerIterator p = r.begin(); p != r.end(); ++p )
      {
        const RotamerAngleTuple &ra = *p;
        // process ra ...
      }
      \endcode
      \param[in] residue the residue to query about
      \param[in] phi first backbone angle
      \param[in] psi second backbone angle
      \param[in] probability_thr threshold on the sum of probabilities.
  */
  RotamerRange get_rotamers_fast(
      IMP::atom::ResidueType residue, float phi, float psi,
      float probability_thr) const;
#endif

  //! query the rotamer library for the rotamer data
  /** This function returns a vector with the queried contents (and is
      therefore slower than get_rotamers_fast). It is however more useful in
      Python code
      \param[in] residue the residue to query about
      \param[in] phi first backbone angle
      \param[in] psi second backbone angle
      \param[in] probability_thr threshold on the sum of probabilities.
  */
  RotamerAngleTuples get_rotamers(
      IMP::atom::ResidueType residue, float phi, float psi,
      float probability_thr) const;

  //! load the library from file
  /** \param[in] lib_file_name file name */
  void read_library_file(const std::string &lib_file_name);

  IMP_OBJECT_METHODS(RotamerLibrary);

private:
  unsigned backbone_angle_to_index(float phi, float psi) const;

  typedef std::vector<RotamerAngleTuples> RotamerAngleTuplesByBackbone;
  typedef std::vector<RotamerAngleTuplesByBackbone> RotamersByResidue;
  RotamersByResidue library_;

  unsigned angle_step_;
  unsigned rotamers_by_backbone_size_;
};


IMPROTAMER_END_NAMESPACE

#endif /* IMPROTAMER_ROTAMER_LIBRARY_H */
