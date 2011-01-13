/**
 *  \file anchors_reader.h
 *  \brief handles reading of anchors data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPMULTIFIT_ANCHORS_READER_H
#define IMPMULTIFIT_ANCHORS_READER_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/file.h>
#include "multifit_config.h"
#include "DataPointsAssignment.h"
IMPMULTIFIT_BEGIN_NAMESPACE


class IMPMULTIFITEXPORT AnchorsData {
 public:
  AnchorsData(){}
  int get_number_of_points() const {return points_.size();}
  int get_number_of_edges() const {return edges_.size();}
  algebra::Vector3Ds points_;
  IntPairs edges_;
};

IMPMULTIFITEXPORT AnchorsData read_anchors_data(const char *txt_filename);
IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_ANCHORS_READER_H */
