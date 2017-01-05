/**
 * \file IMP/multi_state/stat_helpers.h
 * \brief
 *
 * \authors Dina Schneidman
 * Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTI_STATE_STAT_HELPERS_H
#define IMPMULTI_STATE_STAT_HELPERS_H

#include <vector>
#include <utility>

IMPMULTISTATE_BEGIN_NAMESPACE

double get_average(const std::vector<double>& v);

std::pair<double, double> get_average_and_stdev(const std::vector<double>& v);

IMPMULTISTATE_END_NAMESPACE

#endif /* IMPMULTI_STATE_STAT_HELPERS_H */
