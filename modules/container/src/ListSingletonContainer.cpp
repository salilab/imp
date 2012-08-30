/**
 *  \file ListSingletonContainer.cpp   \brief A list of ParticlesTemp.
 *
 *  WARNING This file was generated from ListNAMEContainer.cc
 *  in tools/maintenance/container_templates/container
 *  by tools/maintenance/make-container.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/container/ListSingletonContainer.h"
#include "IMP/SingletonModifier.h"
#include "IMP/SingletonScore.h"
#include <IMP/internal/InternalListSingletonContainer.h>
#include <algorithm>


IMPCONTAINER_BEGIN_NAMESPACE


ListSingletonContainer
::ListSingletonContainer(const ParticlesTemp &ps):
  P(IMP::internal::get_model(ps[0]),
    "ListSingletonContainer%1%")
{
  set_particles(ps);
}

ListSingletonContainer
::ListSingletonContainer(const ParticlesTemp &ps,
                         std::string name):
  P(IMP::internal::get_model(ps[0]), name)
{
  set_particles(ps);
}

ListSingletonContainer
::ListSingletonContainer(Model *m, std::string name):
  P(m, name){
}

ListSingletonContainer
::ListSingletonContainer(Model *m, const char *name):
  P(m, name){
}

void ListSingletonContainer::do_show(std::ostream &out) const {
  IMP_CHECK_OBJECT(this);
  out <<  get_number_of_particles()
      << " Singletons" << std::endl;
}

IMPCONTAINER_END_NAMESPACE
