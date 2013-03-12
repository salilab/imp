/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/Validator.h>
#include <algorithm>

RMF_ENABLE_WARNINGS

namespace RMF {

Creators& get_validators() {
  static Creators vs;
  return vs;
}

Validator::Validator(FileConstHandle rh, std::string name): name_(name),
                                                            rh_(rh) {
}
NodeValidator::NodeValidator(FileConstHandle rh, std::string name):
  Validator(rh, name) {
}

void NodeValidator::write_errors(std::ostream &out) const {
  typedef std::pair<NodeConstHandles, NodeConstHandle> QI;
  std::vector<QI >
  queue(1, QI(NodeConstHandles(), get_file().get_root_node()));
  do {
    QI c = queue.back();
    queue.pop_back();
    write_errors_node(c.second, c.first, out);
    c.first.push_back(c.second);
    NodeConstHandles children = c.second.get_children();
    for (unsigned int i = 0; i < children.size(); ++i) {
      queue.push_back(QI(c.first, children[i]));
    }
  } while (!queue.empty());
}
Validator::~Validator() {
}

struct NonNegativeChecker {
  FloatKey k_;
  std::string catname_;
  std::string keyname_;
  NonNegativeChecker() {
  }
  NonNegativeChecker(FileConstHandle rh, Category c, std::string name) {
    if (c != Category()) {
      catname_ = rh.get_name(c);
      keyname_ = name;
      k_ = rh.get_key<FloatTraits>(c, name);
    }
  }
  void write_errors(NodeConstHandle node,
                    std::ostream    &out) const {
    if (node.get_has_frame_value(k_)) {
      double v = node.get_value(k_);
      if (v <= 0) {
        out << node.get_name() << ": Value " << keyname_ << " in category "
            << catname_
            << " must be positive, but it is " << v << std::endl;
      }
    }
  }
};


struct TieChecker {
  FloatKeys ks_;
  FloatKeys pfks_;
  std::string catname_;
  std::string keynames_;
  TieChecker() {
  }
  TieChecker(FileConstHandle rh, Category c, std::string name, Strings names) {
    if (c != Category()) {
      for (unsigned int i = 0; i < names.size(); ++i) {
        ks_.push_back(rh.get_key<FloatTraits>(c, names[i]));
      }
      catname_ = rh.get_name(c);
    }
    keynames_ = name;
  }
  void write_errors(NodeConstHandle node,
                    std::ostream    &out) const {
    bool found = false;
    for (unsigned int i = 0; i < ks_.size(); ++i) {
      bool cfound = node.get_has_value(ks_[i]);
      if (i > 0 && cfound != found) {
        out << node.get_name() << "A node must either have none or all of "
            << keynames_ << " in category " << catname_ << std::endl;
      }
      found = cfound;
    }
  }
};


class PhysicsValidator: public NodeValidator {
  NonNegativeChecker m_, r_, D_;
  TieChecker coords_;
public:
  PhysicsValidator(FileConstHandle rh, std::string name):
    NodeValidator(rh, name) {
    {
      Category c = rh.get_category("physics");
      m_ = NonNegativeChecker(rh, c, "mass");
      r_ = NonNegativeChecker(rh, c, "radius");
      D_ = NonNegativeChecker(rh, c, "diffusion coefficient");
      Strings xyz;
      xyz.push_back("cartesian x");
      xyz.push_back("cartesian y");
      xyz.push_back("cartesian z");
      coords_ = TieChecker(rh, c, "cartesian coordinates", xyz);
    }
  }
  void write_errors_node(NodeConstHandle        node,
                         const NodeConstHandles &,
                         std::ostream           &out) const {
    m_.write_errors(node, out);
    r_.write_errors(node, out);
    D_.write_errors(node, out);
    coords_.write_errors(node, out);
  }
};

RMF_VALIDATOR(PhysicsValidator);

} /* namespace RMF */

RMF_DISABLE_WARNINGS
