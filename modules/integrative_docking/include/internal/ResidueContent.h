/**
 * \file IMP/integrative_docking/ResidueContent.h \brief
 *
 * Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPINTEGRATIVE_DOCKING_RESIDUE_CONTENT_H
#define IMPINTEGRATIVE_DOCKING_RESIDUE_CONTENT_H

#include <IMP/integrative_docking/integrative_docking_config.h>
#include <IMP/atom/Residue.h>

#include <string>
#include <map>

IMPINTEGRATIVEDOCKING_BEGIN_INTERNAL_NAMESPACE

class IMPINTEGRATIVEDOCKINGEXPORT ResidueContent {
 public:
  /* read residue content file in the following format:
  ALA 3
  TRP 5
  */
  void read_content_file(const std::string& file_name);

  // increase counter for residue
  void add_residue(IMP::atom::ResidueType rt);

  // add residue with counter
  void add_residue(IMP::atom::ResidueType rt, int counter);

  // score for fit
  // note: the score is not symmetric
  // Experimental residue content object should call the modeled:
  // exp_residue_content.score(model_residue_content);
  int score(ResidueContent& other_rc) const;

  // squared score
  int score2(ResidueContent& other_rc) const;

  // get the counter of specific residue
  int get_counter(IMP::atom::ResidueType rt) const;

  // get total number of all residues
  int get_total_counter() const;

  // get sum of squares of residue numbers
  int get_total_counter2() const;

  IMPINTEGRATIVEDOCKINGEXPORT
  friend std::ostream& operator<<(std::ostream& q, const ResidueContent& rc);

 private:
  std::map<IMP::atom::ResidueType, int> residue_content_;
};

IMPINTEGRATIVEDOCKING_END_INTERNAL_NAMESPACE

#endif /* IMPINTEGRATIVE_DOCKING_RESIDUE_CONTENT_H */
