/**
 *  \file JEdge.h
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPDOMINO_JEDGE_H
#define IMPDOMINO_JEDGE_H

#include "JNode.h"
#include "CombState.h"
#include "domino_exports.h"

#include <IMP/Restraint.h>

//todo - can make it more general
IMPDOMINO_BEGIN_NAMESPACE

class IMPDOMINOEXPORT Separator
{
public:
  Separator(const std::string &comb_key) {
    comb_key_ = comb_key;
    score_ = 0.0;
  }
  void update(float new_score) {
    score_ = new_score;
  }
  void show(std::ostream& out = std::cout) const {
    out << "key: " << comb_key_ << " score: " << score_;
  }
protected:
  std::string comb_key_;
  float score_;
};

class IMPDOMINOEXPORT JEdge
{
public:
  JEdge(JNode *source, JNode *target);
  //! Init the separator. Set a table with all of the combinations of states of
  //! the nodes that are found both with source and target
  void init_separators();
  //! Update the scores of the combination in to_node based on from_node.
  /** \param[in] from_node the distributing node
      \param[in] to_node   the updated node
   */
  void min_marginalize(JNode *from_node, JNode *to_node);
  JNode *get_source() const {
    return source_;
  }
  JNode *get_target() const {
    return target_;
  }

  //! Get the edge separator that is contained in the other_comb
  /** \param[in] other_comb a combination that contains the particles that
                 build the separator.
      \param[out] a separator
   */
  CombState *get_separator(const CombState &other_comb) const;

  const std::map<std::string, float> * get_old_separators(JNode *n) const;
  const std::map<std::string, float> * get_new_separators(JNode *n) const;
  void show(std::ostream& out=std::cout) const;
  void clear();
protected:
  //! Get the combination key for the intersection between the other_comb
  //! and the edge separators
  /** \param[in] other_comb a combination that contains the particles that
                 build the separator.
      \param[out] a separator key
   */
  const std::string generate_key(const CombState &other_comb) const;

  JNode *source_;
  JNode *target_;
  std::map<std::string, CombState *> separators_;
  std::map<std::string, float>  source_old_score_separators_;
  std::map<std::string, float>  target_old_score_separators_;
  std::map<std::string, float>  source_new_score_separators_;
  std::map<std::string, float>  target_new_score_separators_;
};

IMPDOMINO_END_NAMESPACE

#endif  /* IMPDOMINO_JEDGE_H */
