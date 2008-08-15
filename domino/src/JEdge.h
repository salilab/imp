/**
 *  \file JEdge.h
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_JEDGE_H
#define __IMP_JEDGE_H

#include "JNode.h"
#include "CombState.h"
#include "domino_config.h"
#include "IMP/Restraint.h"

//todo - can make it more general
namespace IMP
{

class DOMINODLLEXPORT Seperator
{
public:
  Seperator(const std::string &comb_key_) {
    comb_key = comb_key_;
    score = 0.0;
  }
  void update(float new_score) {
    score = new_score;
  }
  void show(std::ostream& out = std::cout) const {
    out << "key: " << comb_key << " score: " << score;
  }
protected:
  std::string comb_key;
  float score;
};

class DOMINODLLEXPORT JEdge
{
public:
  JEdge(JNode *source_, JNode *target_);
  //! Init the separator. Set a table with all of the combinations of states of
  //! the nodes that are found both with source and target
  void init_seperators();
  //! Update the scores of the combination in to_node based on from_node.
  /** /param[in] from_node the distributing node
      /param[in] to_node   the updated node
   */
  void min_marginalize(JNode *from_node, JNode *to_node);
  JNode *get_source() const {
    return source;
  }
  JNode *get_target() const {
    return target;
  }

  //! Get the edge separator that is contained in the other_comb
  /** /param[in] other_comb a combination that contains the particles that
                 build the separator.
      /param[out] a seperator
   */
  CombState *get_seperator(const CombState &other_comb) const;

  const std::map<std::string, float> * get_old_seperators(JNode *n) const;
  const std::map<std::string, float> * get_new_seperators(JNode *n) const;


  void show(std::ostream& out) const;
protected:
  //! Get the combination key for the intersection between the other_comb
  //! and the edge separators
  /** /param[in] other_comb a combination that contains the particles that
                 build the separator.
      /param[out] a seperator key
   */
  const std::string generate_key(const CombState &other_comb) const;

  JNode *source;
  JNode *target;
  std::map<std::string, CombState *> seperators;
  std::map<std::string, float>  source_old_score_seperators;
  std::map<std::string, float>  target_old_score_seperators;
  std::map<std::string, float>  source_new_score_seperators;
  std::map<std::string, float>  target_new_score_seperators;
};

} // namespace IMP

#endif  /* __IMP_JEDGE_H */
