/**
 *  \file IMP/multifit/proteomics_reader.h
 *  \brief handles reading of proteomics data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_PROTEOMICS_READER_H
#define IMPMULTIFIT_PROTEOMICS_READER_H

#include <IMP/algebra/Transformation3D.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/kernel/Model.h>
#include <IMP/core/Hierarchy.h>
#include <IMP/base/file.h>
#include <IMP/multifit/multifit_config.h>
IMPMULTIFIT_BEGIN_NAMESPACE
class ProteinRecordData {
 public:
  void reset_all() {
    name_ = "";
    start_res_ = 0;
    end_res_ = 0;
    filename_ = "";
    surface_filename_ = "";
    ref_filename_ = "";
  }
  ProteinRecordData() { reset_all(); }
  ProteinRecordData(const std::string &name) {
    reset_all();
    name_ = name;
  }
  ProteinRecordData(const std::string &name, const std::string fn) {
    reset_all();
    name_ = name;
    filename_ = fn;
  }
  ProteinRecordData(const std::string &name, int start_res, int end_res,
                    const std::string fn) {
    reset_all();
    name_ = name;
    start_res_ = start_res;
    end_res_ = end_res;
    filename_ = fn;
  }
  ProteinRecordData(const std::string &name, int start_res, int end_res,
                    const std::string &fn, const std::string &surface_fn,
                    const std::string &ref_fn) {
    reset_all();
    name_ = name;
    start_res_ = start_res;
    end_res_ = end_res;
    filename_ = fn;
    surface_filename_ = surface_fn;
    ref_filename_ = ref_fn;
  }

  IMP_SHOWABLE_INLINE(ProteinRecordData, { out << name_; });

  std::string name_;
  int start_res_, end_res_;
  std::string filename_;
  std::string surface_filename_;
  std::string ref_filename_;
};

IMP_VALUES(ProteinRecordData, ProteinRecordDataList);

//! Storage of proteomics data.
class IMPMULTIFITEXPORT ProteomicsData : public base::Object {
 protected:
 public:
  ProteomicsData() : Object("ProteomicsData%1%") {}
  /** return the assigned index
   */
  int add_protein(std::string name, int start_res, int end_res,
                  const std::string &mol_fn, const std::string &surface_fn,
                  const std::string &ref_fn) {
    prot_data_.push_back(ProteinRecordData(name, start_res, end_res, mol_fn,
                                           surface_fn, ref_fn));
    prot_map_[name] = prot_data_.size() - 1;
    return prot_map_[name];
  }
  /** return the assigned index
   */
  int add_protein(const ProteinRecordData &rec) {
    IMP_INTERNAL_CHECK(prot_map_.find(rec.name_) == prot_map_.end(),
                       "protein with name" << rec.name_
                                           << " was added already");
    prot_data_.push_back(rec);
    prot_map_[rec.name_] = prot_data_.size() - 1;
    return prot_map_[rec.name_];
  }
  // if not found -1 is returned
  int find(const std::string &name) const {
    if (prot_map_.find(name) == prot_map_.end()) return -1;
    return prot_map_.find(name)->second;
  }
  void add_interaction(const Ints &ii, bool used_for_filter, float linker_len) {
    interactions_.push_back(ii);
    interaction_in_filter_.push_back(used_for_filter);
    interaction_linker_len_.push_back(linker_len);
  }
  void add_cross_link_interaction(Int prot1, Int res1, Int prot2, Int res2,
                                  bool used_in_filter, Float linker_len) {
    xlinks_.push_back(
        std::make_pair(IntPair(prot1, res1), IntPair(prot2, res2)));
    xlink_in_filter_.push_back(used_in_filter);
    xlink_len_.push_back(linker_len);
  }
  void add_ev_pair(Int prot1, Int prot2) {
    ev_.push_back(std::make_pair(prot1, prot2));
  }
  int get_number_of_proteins() const { return prot_data_.size(); }
  int get_number_of_interactions() const { return interactions_.size(); }
  Ints get_interaction(int interaction_ind) const {
    IMP_USAGE_CHECK(interaction_ind < (int)interactions_.size(),
                    "index out of range\n");
    return interactions_[interaction_ind];
  }
  bool get_interaction_part_of_filter(int interaction_ind) const {
    IMP_USAGE_CHECK(interaction_ind < (int)interaction_in_filter_.size(),
                    "index out of range\n");
    return interaction_in_filter_[interaction_ind];
  }
  int get_interaction_linker_length(int interaction_ind) const {
    IMP_USAGE_CHECK(interaction_ind < (int)interaction_linker_len_.size(),
                    "index out of range\n");
    return interaction_linker_len_[interaction_ind];
  }
  int get_number_of_cross_links() const { return xlinks_.size(); }
  std::pair<IntPair, IntPair> get_cross_link(int xlink_ind) const {
    IMP_USAGE_CHECK(xlink_ind < (int)xlinks_.size(), "index out of range\n");
    return xlinks_[xlink_ind];
  }
  bool get_cross_link_part_of_filter(int xlink_ind) const {
    IMP_USAGE_CHECK(xlink_ind < (int)xlinks_.size(), "index out of range\n");
    return xlink_in_filter_[xlink_ind];
  }
  float get_cross_link_length(int xlink_ind) const {
    IMP_USAGE_CHECK(xlink_ind < (int)xlinks_.size(), "index out of range\n");
    return xlink_len_[xlink_ind];
  }

  //======== ev access functions
  int get_number_of_ev_pairs() const { return ev_.size(); }
  IntPair get_ev_pair(int ev_ind) const {
    IMP_USAGE_CHECK(ev_ind < (int)ev_.size(), "index out of range\n");
    return ev_[ev_ind];
  }

  std::string get_protein_name(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].name_;
  }
  int get_end_res(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].end_res_;
  }
  int get_start_res(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].start_res_;
  }
  std::string get_protein_filename(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].filename_;
  }
  std::string get_reference_filename(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].ref_filename_;
  }
  std::string get_surface_filename(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind].surface_filename_;
  }
  ProteinRecordData get_protein_data(int protein_ind) const {
    IMP_USAGE_CHECK(protein_ind < (int)prot_data_.size(),
                    "index out of range\n");
    return prot_data_[protein_ind];
  }

  IMP_OBJECT_METHODS(ProteomicsData);
  /*IMP_OBJECT_INLINE(ProteomicsData, {
    out<<"Proteins:";
    for(std::vector<ProteinRecordData>::const_iterator
          it = prot_data_.begin(); it != prot_data_.end();it++){
      out<<it->name_<<",";
    }
    out<<std::endl;
    out<<"Interactions:"<<std::endl;
    for(IntsList::const_iterator
          it = interactions_.begin();it != interactions_.end();it++){
      for(Ints::const_iterator it1 = it->begin();
          it1 != it->end();it1++){
        out<<prot_data_[*it1].name_<<",";
      }
      out<<std::endl;
      } }, {});*/
  int get_num_allowed_violated_interactions() const {
    return num_allowed_violated_interactions_;
  }
  void set_num_allowed_violated_interactions(int n) {
    num_allowed_violated_interactions_ = n;
  }
  int get_num_allowed_violated_cross_links() const {
    return num_allowed_violated_xlinks_;
  }
  void set_num_allowed_violated_cross_links(int n) {
    num_allowed_violated_xlinks_ = n;
  }
  int get_num_allowed_violated_ev() const { return num_allowed_violated_ev_; }
  void set_num_allowed_violated_ev(int n) { num_allowed_violated_ev_ = n; }

 protected:
  std::vector<ProteinRecordData> prot_data_;
  std::map<std::string, int> prot_map_;
  IntsList interactions_;
  std::vector<bool> interaction_in_filter_;  // for each interaction
  Floats interaction_linker_len_;  // for each interaction in interactions_
  int num_allowed_violated_interactions_;
  // decide if it used to build the JT or just for scoring
  //=========residue cross links
  std::vector<std::pair<IntPair, IntPair> > xlinks_;
  std::vector<bool> xlink_in_filter_;
  Floats xlink_len_;
  int num_allowed_violated_xlinks_;
  //=========
  IntPairs ev_;  // pairs of proteins to calcualte EV between
  int num_allowed_violated_ev_;
};

//! Proteomics reader
/**
\todo consider using TextInput
 */
IMPMULTIFITEXPORT ProteomicsData *read_proteomics_data(
    const char *proteomics_fn);
IMPMULTIFITEXPORT
ProteomicsData *get_partial_proteomics_data(const ProteomicsData *pd,
                                            const Strings &prot_names);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_PROTEOMICS_READER_H */
