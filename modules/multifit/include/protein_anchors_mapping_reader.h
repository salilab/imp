/**
 *  \file protein_anchors_mapping_reader.h
 *  \brief handles reading matches between a protein and its anchors
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_PROTEIN_ANCHORS_MAPPING_READER_H
#define IMPMULTIFIT_PROTEIN_ANCHORS_MAPPING_READER_H

#include <IMP/multifit/multifit_config.h>
#include <IMP/multifit/anchors_reader.h>
#include <IMP/domino/DominoSampler.h>
#include <IMP/multifit/SettingsData.h>
#include <IMP/multifit/proteomics_reader.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! stores the anchors sampling space for each protein
class IMPMULTIFITEXPORT ProteinsAnchorsSamplingSpace {
 public:
  ProteinsAnchorsSamplingSpace(multifit::ProteomicsData *prots = 0)
      : prots_(prots) {}
  void set_anchors(const multifit::AnchorsData &data) { anchors_data_ = data; }
  multifit::AnchorsData get_anchors() const { return anchors_data_; }
  inline std::string get_anchors_filename() const { return anchors_fn_; }
  void set_anchors_filename(const std::string &fn) { anchors_fn_ = fn; }
  multifit::ProteomicsData *get_proteomics_data() const { return prots_; }
  void add_protein(const multifit::ProteinRecordData &rec) {
    prots_->add_protein(rec);
  }
  IntsList get_paths_for_protein(const std::string &prot_name) const {
    IMP_USAGE_CHECK(paths_map_.find(prot_name) != paths_map_.end(),
                    "Protein:" << prot_name << " is not found");
    return paths_map_.find(prot_name)->second;
  }
  void set_paths_filename_for_protein(const std::string &prot_name,
                                      const std::string &paths_filename) {
    IMP_USAGE_CHECK(paths_filename_.find(prot_name) == paths_filename_.end(),
                    "Protein:" << prot_name << " is already set");
    paths_filename_[prot_name] = paths_filename;
  }
  std::string get_paths_filename_for_protein(const std::string &prot_name)
      const {
    IMP_USAGE_CHECK(paths_filename_.find(prot_name) != paths_filename_.end(),
                    "Protein:" << prot_name << " is not found");
    return paths_filename_.find(prot_name)->second;
  }
  void set_paths_for_protein(const std::string &prot_name, IntsList paths) {
    IMP_USAGE_CHECK(paths_map_.find(prot_name) == paths_map_.end(),
                    "Protein:" << prot_name << " is already set");
    paths_map_[prot_name] = paths;
  }

  void show(std::ostream &s = std::cout) const;

 protected:
  std::map<std::string, IntsList> paths_map_;
  std::map<std::string, std::string> paths_filename_;
  multifit::AnchorsData anchors_data_;
  base::PointerMember<multifit::ProteomicsData> prots_;
  std::string anchors_fn_;
};
IMP_VALUES(ProteinsAnchorsSamplingSpace, ProteinsAnchorsSamplingSpaces);

IMPMULTIFITEXPORT
ProteinsAnchorsSamplingSpace read_protein_anchors_mapping(
    multifit::ProteomicsData *prots, const std::string &anchors_prot_map_fn,
    int max_paths = INT_MAX);
IMPMULTIFITEXPORT
void write_protein_anchors_mapping(
    const std::string &anchors_prot_map_fn, const std::string &anchors_fn,
    const std::vector<std::pair<String, String> > &prot_paths);

// write the protein anchors mapping
IMPMULTIFITEXPORT
void write_protein_anchors_mapping(const std::string &anchors_prot_map_fn,
                                   const ProteinsAnchorsSamplingSpace &pa,
                                   const Strings &prot_names);

//! Get the sampling space of few of the proteins
IMPMULTIFITEXPORT
ProteinsAnchorsSamplingSpace get_part_of_sampling_space(
    const ProteinsAnchorsSamplingSpace &prots_ss, const Strings &prot_names);
//! Get the assembly data for a few of the proteins
IMPMULTIFITEXPORT
multifit::SettingsData *get_partial_assembly_setting_data(
    multifit::SettingsData *prots_sd, const Strings &prot_names);

IMPMULTIFIT_END_NAMESPACE

#endif /* IMPMULTIFIT_PROTEIN_ANCHORS_MAPPING_READER_H */
