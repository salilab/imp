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
  AnchorsData(algebra::Vector3Ds points, IntPairs edges){
    points_=points;edges_=edges;}
  AnchorsData(AnchorsData orig,Ints these_indexes, bool keep){
    //copy an AnchorsData object but ONLY/EXCLUDING a subset
    //for ONLY the subset, use keep==true
    //for EXCLUDING the subset, use keep==false
    Ints in_subset;
    Ints idx_in_new;
    bool broken=false;
    for (unsigned int op=0;op<orig.points_.size();op++) {
      for (unsigned int np=0;np<these_indexes.size();np++) {
        if (these_indexes[np]==op) {
          in_subset.push_back(1);
          broken=true;
          if (keep) {
            idx_in_new.push_back(points_.size());
            points_.push_back(orig.points_[op]);
          }
          else idx_in_new.push_back(-1);
          break;
        }
      }
      if (broken==false) {
        in_subset.push_back(0);
        if (!keep) {
          idx_in_new.push_back(points_.size());
          points_.push_back(orig.points_[op]);
        }
        else idx_in_new.push_back(-1);
      }
      else broken=false;
    }
    //now do edges...
    if (keep) {
      for (unsigned int oe=0;oe<orig.edges_.size();oe++){
        if (in_subset[orig.edges_[oe].first] &&
            in_subset[orig.edges_[oe].second]) {
          edges_.push_back(IntPair(idx_in_new[orig.edges_[oe].first],
                                   idx_in_new[orig.edges_[oe].second]));
        }
      }
    }
    else {
      for (unsigned int oe=0;oe<orig.edges_.size();oe++){
        if (!in_subset[orig.edges_[oe].first] &&
            !in_subset[orig.edges_[oe].second]) {
          edges_.push_back(IntPair(idx_in_new[orig.edges_[oe].first],
                                   idx_in_new[orig.edges_[oe].second]));
        }
      }
    }
  }
  int get_number_of_points() const {return points_.size();}
  int get_number_of_edges() const {return edges_.size();}
  void show(std::ostream& out=std::cout) const {
    out<<"==== "<<points_.size()<<" Anchors:"<<std::endl;
    for(int i=0;i<(int)points_.size();i++) {
      out<<points_[i]<<std::endl;
    }
    out<<"===="<<edges_.size()<<" Edges:"<<std::endl;
    for(int i=0;i<(int)edges_.size();i++) {
      out<<"("<<edges_[i].first<<","<<edges_[i].second<<") ";
    }
    out<<std::endl;
  }
  algebra::Vector3Ds points_;
  IntPairs edges_;
};

IMPMULTIFITEXPORT AnchorsData read_anchors_data(const char *txt_filename);
IMPMULTIFITEXPORT
void write_txt(const std::string &txt_filename,
               const AnchorsData &ad);
IMPMULTIFITEXPORT
void write_cmm(const std::string &cmm_filename,
               const std::string &marker_set_name,
               const AnchorsData &dpa);

IMPMULTIFIT_END_NAMESPACE
#endif  /* IMPMULTIFIT_ANCHORS_READER_H */
