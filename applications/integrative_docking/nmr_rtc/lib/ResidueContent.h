/**
 * \file ResidueContent \brief
 *
 * Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_RESIDUE_CONTENT_H
#define IMP_RESIDUE_CONTENT_H

#include <IMP/atom/Residue.h>

#include <string>
#include <map>

class ResidueContent {
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

  friend std::ostream& operator<<(std::ostream& q, const ResidueContent& rc);
private:
  std::map<IMP::atom::ResidueType, int> residue_content_;
};

#endif /* IMP_RESIDUE_CONTENT_H */
