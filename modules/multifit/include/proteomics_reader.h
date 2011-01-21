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
#include <IMP/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/file.h>
#include "multifit_config.h"
IMPMULTIFIT_BEGIN_NAMESPACE

class IMPMULTIFITEXPORT ProteomicsData {
 protected:
  class ProteinData {
  public:
    void reset_all(){
      name_="";
      start_res_=0;
      end_res_=0;
      filename_="";
      ref_filename_="";
    }
    ProteinData() {reset_all();
    }
    ProteinData(const std::string &name){
      reset_all();
      name_=name;
    }
    ProteinData(const std::string &name,const std::string fn){
      reset_all();
      name_=name;filename_=fn;
    }
    ProteinData(const std::string &name,
                int start_res,int end_res,const std::string fn){
      reset_all();
      name_=name;
      start_res_=start_res;
      end_res_=end_res;
      filename_=fn;
    }
    ProteinData(const std::string &name,
                int start_res,int end_res,const std::string fn,
                const std::string &ref_fn){
      reset_all();
      name_=name;
      start_res_=start_res;
      end_res_=end_res;
      filename_=fn;
      ref_filename_=ref_fn;
    }
    std::string name_;
    int start_res_,end_res_;
    std::string filename_;
    std::string ref_filename_;
  };
 public:
  ProteomicsData(){}
  void add_protein(std::string name,int start_res,
                   int end_res,const std::string &mol_fn,
                   const std::string &ref_fn){
    prot_data_.push_back(ProteinData(name,start_res,end_res,mol_fn,ref_fn));
    prot_map_[name]=prot_data_.size()-1;
  }
  //if not found -1 is returned
  int find(const std::string &name) const {
    if (prot_map_.find(name) == prot_map_.end()) return -1;
    return prot_map_.find(name)->second;
  }
  void add_interaction(const std::vector<int> &ii) {
    interactions_.push_back(ii);
  }
  void show(std::ostream& out=std::cout) const {
    out<<"Proteins:";
    for(std::vector<ProteinData>::const_iterator it = prot_data_.begin();
        it != prot_data_.end();it++){
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
    }
  }
  int get_number_of_proteins() const {return prot_data_.size();}
  int get_number_of_interactions() const {return interactions_.size();}

  Ints get_interaction(int interaction_ind) const {
    IMP_USAGE_CHECK(interaction_ind<(int)interactions_.size(),
                    "index out of range\n");
    return interactions_[interaction_ind];}
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
 protected:
  ProteinData get_protein_data(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind<(int)prot_data_.size(),"index out of range\n");
    return prot_data_[protein_ind];}
  std::vector<ProteinData> prot_data_;
  std::map<std::string,int> prot_map_;
  std::vector<Ints>interactions_;
};

//! Proteomics reader
/**
\todo consider using TextInput
 */
IMPMULTIFITEXPORT ProteomicsData read_proteomics_data(
  const char *proteomics_fn);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_PROTEOMICS_READER_H */
