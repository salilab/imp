/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/Validator.h>
#include <algorithm>
namespace RMF {

Creators& get_validators() {
  static Creators vs;
  return vs;
}

Validator::Validator(RootHandle rh, std::string name): name_(name), rh_(rh){}
NodeValidator::NodeValidator(RootHandle rh, std::string name):
    Validator(rh, name){}

void NodeValidator::write_errors(std::ostream &out) const {
  typedef std::pair<NodeHandles, NodeHandle> QI;
  vector<QI >
      queue(1, QI(NodeHandles(), get_root_handle()));
  do {
    QI c= queue.back();
    queue.pop_back();
    write_errors_node(c.second, c.first, out);
    c.first.push_back(c.second);
    NodeHandles children= c.second.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      queue.push_back(QI(c.first, children[i]));
    }
  } while (!queue.empty());
}
Validator::~Validator() {
}

namespace {
struct NodeIDsLess {
  bool operator()(const NodeHandles &a, const NodeHandles&b) const {
    return std::lexicographical_compare(a.begin(), a.end(),
                                        b.begin(), b.end());
  }
};
struct NodeIDsEqual {
  bool operator()(const NodeHandles &a, const NodeHandles&b) const {
    // super lazy
    return !std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end())
        && !std::lexicographical_compare(b.begin(), b.end(),
                                         a.begin(), a.end());
  }
};
}

class UniquenessValidator: public Validator {
  template <int Arity>
  void validate_one(RootHandle rh,
                    std::ostream &out) const {
    vector<NodeSetHandle<Arity> > all= rh.get_node_sets<Arity>();
    if (all.empty()) return;
    vector<NodeHandles> seen(all.size());
    for (unsigned int i=0; i< all.size(); ++i) {
      NodeHandles cur(Arity);
      for (unsigned int j=0; j< Arity; ++j) {
        cur[j]= all[i].get_node(j);
        if (j >0 && cur[j] <= cur[j-1]) {
          out << "Not all the sets of size " << Arity
              << " are properly sorted: " << all[i]
              << " is not." << std::endl;
        }
      }
      seen[i]=cur;
    }
    std::sort(seen.begin(), seen.end(), NodeIDsLess());
    seen.erase(std::unique(seen.begin(), seen.end(), NodeIDsEqual()),
               seen.end());
    if (all.size() != seen.size()) {
      out << "Not all sets of size " << Arity << " are unique. "
          << " Of " << all.size() << " there were "
          << all.size() - seen.size() << " redundant." << std::endl;
    }
  }
 public:
  UniquenessValidator(RootHandle rh, std::string name):
      Validator(rh, name){}
  void write_errors(std::ostream &out) const {
    validate_one<2>(get_root_handle(), out);
    validate_one<3>(get_root_handle(), out);
    validate_one<4>(get_root_handle(), out);
  }
};

IMP_RMF_VALIDATOR(UniquenessValidator);


struct NonNegativeChecker {
  FloatKey k_;
  std::string catname_;
  std::string keyname_;
  NonNegativeChecker(){}
  NonNegativeChecker(RootHandle rh, Category c, std::string name) {
    if (c != Category()) {
      if (rh.get_has_key<FloatTraits>(c, name)) {
        k_=rh.get_key<FloatTraits>(c, name);
        catname_= rh.get_category_name(c);
        keyname_=name;
      }
    }
  }
  void write_errors(NodeHandle node,
                    std::ostream &out) const {
    if (k_ != FloatKey() && node.get_has_value(k_)) {
      double v= node.get_value(k_);
      if (v <=0) {
        out << node.get_name() << ": Value " << keyname_ << " in category "
            << catname_
            << " must be positive, but it is " << v << std::endl;
      }
    }
  }
};


struct TieChecker {
  FloatKeys ks_;
  std::string catname_;
  std::string keynames_;
  TieChecker(){}
  TieChecker(RootHandle rh, Category c, std::string name, Strings names) {
    if (c != Category()) {
      for (unsigned int i=0; i< names.size(); ++i) {
        if (rh.get_has_key<FloatTraits>(c, names[i])) {
          ks_.push_back(rh.get_key<FloatTraits>(c, names[i]));
        }
      }
      catname_= rh.get_category_name(c);
    }
    keynames_=name;
  }
  void write_errors(NodeHandle node,
                    std::ostream &out) const {
    if (ks_.empty()) return;
    if ( ks_[0] != FloatKey() && node.get_has_value(ks_[0])) {
      for (unsigned int i=1; i< ks_.size(); ++i) {
        if (!node.get_has_value(ks_[i])) {
          out << node.get_name() << "A node must either have none or all of "
              << keynames_ << " in category " << catname_ << std::endl;
        }
      }
    } else {
      for (unsigned int i=1; i< ks_.size(); ++i) {
        if (node.get_has_value(ks_[i])) {
          out << node.get_name() << "A node must either have none or all of "
              << keynames_ << " in category " << catname_ << std::endl;
        }
      }
    }
  }
};


class PhysicsValidator: public NodeValidator {
  NonNegativeChecker m_, r_, D_;
  TieChecker coords_;
 public:
  PhysicsValidator(RootHandle rh, std::string name):
      NodeValidator(rh, name){
    if (rh.get_has_category("physics")) {
      Category c= rh.get_category("physics");
      m_= NonNegativeChecker(rh, c, "mass");
      r_= NonNegativeChecker(rh, c, "radius");
      D_= NonNegativeChecker(rh, c, "diffusion coefficient");
      Strings xyz;
      xyz.push_back("cartesian x");
      xyz.push_back("cartesian y");
      xyz.push_back("cartesian z");
      coords_=TieChecker(rh, c, "cartesian coordinates", xyz);
    }
  }
  void write_errors_node(NodeHandle node,
                         const NodeHandles &,
                         std::ostream &out) const {
    m_.write_errors(node, out);
    r_.write_errors(node, out);
    D_.write_errors(node, out);
    coords_.write_errors(node, out);
  }
};

IMP_RMF_VALIDATOR(PhysicsValidator);

} /* namespace RMF */
