/**
 *  \file NeighborsTable.cpp
 * \brief
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/core/NeighborsTable.h>
#include <IMP/PairModifier.h>
#include <IMP/container_macros.h>
#include <IMP/PairContainer.h>

IMPCORE_BEGIN_NAMESPACE

void NeighborsTable::do_before_evaluate() {
  std::size_t new_hash = input_->get_contents_hash();
  if (new_hash == input_version_) return;
  input_version_ = new_hash;

  data_.clear();

  IMP_CONTAINER_FOREACH(PairContainer, input_, {
    data_[std::get<0>(_1)].push_back(std::get<1>(_1));
    data_[std::get<1>(_1)].push_back(std::get<0>(_1));
  });
}

NeighborsTable::NeighborsTable(PairContainer *input, std::string name)
    : ScoreState(input->get_model(), name),
      input_(input),
      input_version_(input->get_contents_hash()) {}

IMPCORE_END_NAMESPACE
