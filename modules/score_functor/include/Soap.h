/**
 * \file score_functor/Soap.h
 * \brief  A Score on the distance between a pair of particles.
 *
 * \authors Dina Schneidman
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSCORE_FUNCTOR_SOAP_H
#define IMPSCORE_FUNCTOR_SOAP_H

#include <IMP/score_functor/score_functor_config.h>
#include "Dope.h"
#include "Statistical.h"

IMPSCOREFUNCTOR_BEGIN_NAMESPACE

class Soap : public Statistical<DopeType, false, false> {
  typedef Statistical<DopeType, false, false> P;

 public:
  Soap(double threshold = std::numeric_limits<double>::max())
      : P(get_soap_type_key(), threshold, get_data_path("soap_score.lib")) {}
  Soap(double threshold, base::TextInput data_file)
      : P(get_soap_type_key(), threshold, data_file) {}
  static IntKey get_soap_type_key() {
    static const IntKey ik("dope atom type");
    return ik;
  }
};

IMPSCOREFUNCTOR_END_NAMESPACE

#endif /* IMPSCORE_FUNCTOR_SOAP_H */
