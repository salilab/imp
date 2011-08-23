/**
 *  \file proteomics_reader.h
 *  \brief handles reading of proteomics data
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_PROTEOMICS_READER_H
#define IMPMULTIFIT_PROTEOMICS_READER_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/file.h>
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE
class ProteinRecordData {
public:
  void reset_all(){
    name_="";
    start_res_=0;
    end_res_=0;
    filename_="";
    surface_filename_="";
    ref_filename_="";
    }
  ProteinRecordData() {reset_all();
    }
    ProteinRecordData(const std::string &name){
      reset_all();
      name_=name;
    }
    ProteinRecordData(const std::string &name,const std::string fn){
      reset_all();
      name_=name;filename_=fn;
    }
    ProteinRecordData(const std::string &name,
                int start_res,int end_res,const std::string fn){
      reset_all();
      name_=name;
      start_res_=start_res;
      end_res_=end_res;
      filename_=fn;
    }
    ProteinRecordData(const std::string &name,
                int start_res,int end_res,const std::string &fn,
                      const std::string &surface_fn,
                const std::string &ref_fn){
      reset_all();
      name_=name;
      start_res_=start_res;
      end_res_=end_res;
      filename_=fn;
      surface_filename_=surface_fn;
      ref_filename_=ref_fn;
    }
    std::string name_;
    int start_res_,end_res_;
    std::string filename_;
    std::string surface_filename_;
    std::string ref_filename_;
  };
class IMPMULTIFITEXPORT ProteomicsData {//: public Object {
 protected:
 public:
  ProteomicsData() {}//: Object("ProteomicsData%1%") {}
  /** return the assigned index
   */
  int add_protein(std::string name,int start_res,
                  int end_res,const std::string &mol_fn,
                  const std::string &surface_fn,
                  const std::string &ref_fn){
    prot_data_.push_back(ProteinRecordData(name,start_res,end_res,mol_fn,
                                           surface_fn,ref_fn));
    prot_map_[name]=prot_data_.size()-1;
    return prot_map_[name];
  }
  /** return the assigned index
   */
  int add_protein(const ProteinRecordData &rec) {
    IMP_INTERNAL_CHECK(prot_map_.find(rec.name_)==prot_map_.end(),
                       "protein with name"<<rec.name_<<" was added already");
    prot_data_.push_back(rec);
    prot_map_[rec.name_]=prot_data_.size()-1;
    return prot_map_[rec.name_];
  }
  //if not found -1 is returned
  int find(const std::string &name) const {
    if (prot_map_.find(name) == prot_map_.end()) return -1;
    return prot_map_.find(name)->second;
  }
  void add_interaction(const Ints &ii) {
    interactions_.push_back(ii);
  }
  void add_xlink_interaction(Int prot1,Int res1,Int prot2,Int res2){
    xlinks_.push_back(std::make_pair(IntPair(prot1,res1),IntPair(prot2,res2)));
  }
  void add_ev_pair(Int prot1,Int prot2){
    ev_.push_back(std::make_pair(prot1,prot2));
  }
  int get_number_of_proteins() const {return prot_data_.size();}
  int get_number_of_interactions() const {return interactions_.size();}
  Ints get_interaction(int interaction_ind) const {
    IMP_USAGE_CHECK(interaction_ind<(int)interactions_.size(),
                    "index out of range\n");
    return interactions_[interaction_ind];}

  int get_number_of_xlinks() const {return xlinks_.size();}
  std::pair<IntPair,IntPair> get_xlink(int xlink_ind) const {
    IMP_USAGE_CHECK(xlink_ind<(int)xlinks_.size(),
                    "index out of range\n");
    return xlinks_[xlink_ind];
  }


  int get_number_of_ev_pairs() const {return ev_.size();}
  IntPair get_ev_pair(int ev_ind) const {
    IMP_USAGE_CHECK(ev_ind<(int)ev_.size(),
                    "index out of range\n");
    return ev_[ev_ind];
  }


  std::string get_protein_name(int protein_ind) const {
        IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                        "index out of range\n");
        return prot_data_[protein_ind].name_;
  }
  int get_end_res(int protein_ind) const {
        IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                        "index out of range\n");
        return prot_data_[protein_ind].end_res_;
  }
  int get_start_res(int protein_ind) const {
        IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                        "index out of range\n");
        return prot_data_[protein_ind].start_res_;
  }
  std::string get_protein_filename(int protein_ind) const {
        IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                        "index out of range\n");
        return prot_data_[protein_ind].filename_;
  }
  std::string get_reference_filename(int protein_ind) const {
        IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                        "index out of range\n");
        return prot_data_[protein_ind].ref_filename_;
  }
  std::string get_surface_filename(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].surface_filename_;
  }
  ProteinRecordData get_protein_data(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),"index out of range\n");
    return prot_data_[protein_ind];}
  void show(std::ostream &out=std::cout) {
    out<<"Proteins:";
    for(std::vector<ProteinRecordData>::const_iterator
          it = prot_data_.begin(); it != prot_data_.end();it++){
      out<<it->name_<<",";
    }
    out<<std::endl;
    out<<"Interactions:"<<std::endl;
    for(std::vector<Ints >::const_iterator
          it = interactions_.begin();it != interactions_.end();it++){
      for(Ints::const_iterator it1 = it->begin();
          it1 != it->end();it1++){
        out<<prot_data_[*it1].name_<<",";
      }
      out<<std::endl;
    }
  }
  /*IMP_OBJECT_INLINE(ProteomicsData, {
        out<<"Proteins:";
        for(std::vector<ProteinRecordData>::const_iterator
            it = prot_data_.begin(); it != prot_data_.end();it++){
          out<<it->name_<<",";
        }
        out<<std::endl;
        out<<"Interactions:"<<std::endl;
        for(std::vector<std::vector<int> >::const_iterator
              it = interactions_.begin();it != interactions_.end();it++){
          for(std::vector<int>::const_iterator it1 = it->begin();
              it1 != it->end();it1++){
            out<<prot_data_[*it1].name_<<",";
          }
          out<<std::endl;
          } },);*/

 protected:
  std::vector<ProteinRecordData> prot_data_;
  std::map<std::string,int> prot_map_;
  domino::IntsList interactions_;
  std::vector<std::pair<IntPair,IntPair> > xlinks_;
  IntPairs ev_; //pairs of proteins to calcualte EV between
};

//! Proteomics reader
/**
\todo consider using TextInput
 */
IMPMULTIFITEXPORT ProteomicsData read_proteomics_data(
  const char *proteomics_fn);
IMPMULTIFITEXPORT
ProteomicsData get_partial_proteomics_data(
                       const ProteomicsData &pd,
                       const Strings &prot_names);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_PROTEOMICS_READER_H */
