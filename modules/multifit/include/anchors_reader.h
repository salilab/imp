/**
 *  \file IMP/multifit/anchors_reader.h
 *  \brief handles reading of anchors data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_ANCHORS_READER_H
#define IMPMULTIFIT_ANCHORS_READER_H

#include <IMP/atom/SecondaryStructureResidue.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/file.h>
#include <IMP/multifit/multifit_config.h>
#include "DataPointsAssignment.h"
IMPMULTIFIT_BEGIN_NAMESPACE


class IMPMULTIFITEXPORT AnchorsData {
 public:
  AnchorsData(){}
  AnchorsData(algebra::Vector3Ds points, IntPairs edges){
    points_=points;edges_=edges;
    secondary_structure_ps_=Particles();
    //set true to consider all points
    for (unsigned int i=0;i<points_.size();i++) {
      consider_point_.push_back(true);
    }
  }

  void remove_edges_for_node(int node_ind) {
    //iterate over edges and remove edges including node_ind
    IntPairs new_edges;
    for(int i=0;i<(int)edges_.size();i++) {
      if (! ((edges_[i].first==node_ind) || (edges_[i].second==node_ind))) {
        new_edges.push_back(edges_[i]);
      }
    }
    consider_point_[node_ind]=false;
    edges_=new_edges;
  }
  int get_number_of_points() const {return points_.size();}
  int get_number_of_edges() const {return edges_.size();}
  bool get_is_point_considered(int node_ind) const {
    return consider_point_[node_ind];}
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


  //!Check if secondary structure is setup
  bool get_secondary_structure_is_set(){
    return (secondary_structure_ps_.size()==points_.size());
  }

  //!Sets up default secondary structure particles for every anchor (run first)
  void setup_secondary_structure(Model *mdl);

  //!Assign secondary structure particles. Provide indices if out of order.
  /**
     \param[in] ssres_ps The particles which you will be assigning to anchors
     \param[in] indices (Optional) List of which anchor point numbers the
                provided ssres_ps belong to.
  */
  void set_secondary_structure_probabilities(const Particles &ssres_ps,
                                             const Ints &indices=Ints());

  inline Particles get_secondary_structure_particles() const {
    return secondary_structure_ps_;
  }
  algebra::Vector3Ds points_;
  std::vector<bool> consider_point_;
  IntPairs edges_;
 protected:
  Particles secondary_structure_ps_;
};
IMP_VALUES(AnchorsData, AnchorsDataList);

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
