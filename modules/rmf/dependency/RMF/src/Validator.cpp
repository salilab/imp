/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/Validator.h>
#include <algorithm>
namespace RMF {

Creators& get_validators() {
  static Creators vs;
  return vs;
}

Validator::Validator(FileConstHandle rh, std::string name): name_(name),
                                                            rh_(rh){}
NodeValidator::NodeValidator(FileConstHandle rh, std::string name):
    Validator(rh, name){}

void NodeValidator::write_errors(std::ostream &out) const {
  typedef std::pair<NodeConstHandles, NodeConstHandle> QI;
  vector<QI >
      queue(1, QI(NodeConstHandles(), get_file().get_root_node()));
  do {
    QI c= queue.back();
    queue.pop_back();
    write_errors_node(c.second, c.first, out);
    c.first.push_back(c.second);
    NodeConstHandles children= c.second.get_children();
    for (unsigned int i=0; i< children.size(); ++i) {
      queue.push_back(QI(c.first, children[i]));
    }
  } while (!queue.empty());
}
Validator::~Validator() {
}

namespace {
struct NodeIDsLess {
  bool operator()(const NodeConstHandles &a, const NodeConstHandles&b) const {
    return std::lexicographical_compare(a.begin(), a.end(),
                                        b.begin(), b.end());
  }
};
struct NodeIDsEqual {
  bool operator()(const NodeConstHandles &a, const NodeConstHandles&b) const {
    // super lazy
    return !std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end())
        && !std::lexicographical_compare(b.begin(), b.end(),
                                         a.begin(), a.end());
  }
};
}

struct NonNegativeChecker {
  FloatKey k_, pfk_;
  std::string catname_;
  std::string keyname_;
  NonNegativeChecker(){}
  NonNegativeChecker(FileConstHandle rh, Category c, std::string name) {
    if (c != Category()) {
      catname_= rh.get_name(c);
      keyname_=name;
      if (rh.get_has_key<FloatTraits>(c, name, false)) {
        k_=rh.get_key<FloatTraits>(c, name, false);
      }
      if (rh.get_has_key<FloatTraits>(c, name, true)) {
        pfk_=rh.get_key<FloatTraits>(c, name, true);
      }
    }
  }
  void write_errors(NodeConstHandle node,
                    std::ostream &out) const {
    if (k_ != FloatKey() && node.get_has_value(k_)) {
      double v= node.get_value(k_);
      if (v <=0) {
        out << node.get_name() << ": Value " << keyname_ << " in category "
            << catname_
            << " must be positive, but it is " << v << std::endl;
      }
    }
    if (pfk_ != FloatKey()) {
      for (unsigned int i=0; i< node.get_file().get_number_of_frames();
           ++i) {
        node.get_file().set_current_frame(i);
        if (node.get_has_value(pfk_)) {
          double v= node.get_value(pfk_);
          if (v <=0) {
            out << node.get_name() << ": Value " << keyname_ << " in category "
                << catname_
                << " must be positive, but it is " << v << std::endl;
          }
        }
      }
    }
  }
};


struct TieChecker {
  FloatKeys ks_;
  FloatKeys pfks_;
  std::string catname_;
  std::string keynames_;
  TieChecker(){}
  TieChecker(FileConstHandle rh, Category c, std::string name, Strings names) {
    if (c != Category()) {
      for (unsigned int i=0; i< names.size(); ++i) {
        ks_.push_back(rh.get_key<FloatTraits>(c, names[i], false));
        pfks_.push_back(rh.get_key<FloatTraits>(c, names[i], true));
      }
      catname_= rh.get_name(c);
    }
    keynames_=name;
  }
  void write_errors(NodeConstHandle node,
                    std::ostream &out) const {
    bool found=false;
    unsigned int nf=node.get_file().get_number_of_frames();
    for (unsigned int f=0; f< nf; ++f) {
      node.get_file().set_current_frame(f);
      for (unsigned int i=0; i< ks_.size(); ++i) {
        bool cfound= node.get_has_value(ks_[i])
          || node.get_has_value(pfks_[i]);
        if (i > 0 && cfound != found) {
          out << node.get_name() << "A node must either have none or all of "
              << keynames_ << " in category " << catname_ << std::endl;
        }
        found=cfound;
      }
    }
  }
};


class PhysicsValidator: public NodeValidator {
  NonNegativeChecker m_, r_, D_;
  TieChecker coords_;
 public:
  PhysicsValidator(FileConstHandle rh, std::string name):
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
  void write_errors_node(NodeConstHandle node,
                         const NodeConstHandles &,
                         std::ostream &out) const {
    m_.write_errors(node, out);
    r_.write_errors(node, out);
    D_.write_errors(node, out);
    coords_.write_errors(node, out);
  }
};

RMF_VALIDATOR(PhysicsValidator);


#define RMF_DECLARE_KEYS(lcname, UCName, PassValue, ReturnValue, \
                         PassValues, ReturnValues)               \
  vector<std::pair<Key<UCName##Traits>, Key<UCName##Traits> > >  \
  lcname##_pairs_;

#define RMF_INIT_KEYS(lcname, UCName, PassValue, ReturnValue,           \
                      PassValues, ReturnValues)                         \
  init(rh, categories[i], lcname##_pairs_)

#define RMF_CHECK_KEYS(lcname, UCName, PassValue, ReturnValue,       \
                       PassValues, ReturnValues)                     \
  check(node, out, lcname##_pairs_)

class KeyValidator: public NodeValidator {
  RMF_FOREACH_TYPE(RMF_DECLARE_KEYS);

  template <class Traits>
  void check(NodeConstHandle node,
             std::ostream &out,
             const vector<std::pair<Key<Traits>,
                                    Key<Traits> > >  &keys) const {
    for (unsigned int i=0; i< keys.size(); ++i) {
      if (!node.get_has_value(keys[i].second)) continue;
      unsigned int fn
        =node.get_file().get_number_of_frames();
      for (unsigned int j=0; j< fn; ++j) {
        node.get_file().set_current_frame(j);
        if (node.get_has_value(keys[i].first)) {
          out << "Node " << node
              << " has non-per frame and per frame for "
              << node.get_file().get_name(keys[i].second)
              << " at frame " << j << std::endl;
        }
      }
    }
  }

  template <class Traits>
  void init(FileConstHandle rh,
            Category cat,
            vector<std::pair<Key<Traits>,
                             Key<Traits> > >  &keys) {
    vector<Key<Traits> > allkeys
      = rh.get_keys<Traits>(cat);
    for (unsigned int j=0; j< allkeys.size(); ++j) {
      for (unsigned int k=0; k < j; ++k) {
        if (rh.get_name(allkeys[j]) == rh.get_name(allkeys[k])) {
          Key<Traits> ka= allkeys[j];
          Key<Traits> kb= allkeys[k];
          if (rh.get_is_per_frame(kb)) std::swap(ka, kb);
          RMF_INTERNAL_CHECK(rh.get_is_per_frame(ka),
                                 "Not");
          RMF_INTERNAL_CHECK(!rh.get_is_per_frame(kb),
                                 "Is");
          keys.push_back(std::make_pair(ka, kb));
          break;
        }
      }
    }
  }

 public:
  KeyValidator(FileConstHandle rh, std::string name):
      NodeValidator(rh, name){
    Categories categories= rh.get_categories();
    for (unsigned int i=0; i< categories.size(); ++i) {
      RMF_FOREACH_TYPE(RMF_INIT_KEYS);
    }
  }
  void write_errors_node(NodeConstHandle node,
                         const NodeConstHandles &,
                         std::ostream &out) const {
    RMF_FOREACH_TYPE(RMF_CHECK_KEYS);
  }
};
RMF_VALIDATOR(KeyValidator);


} /* namespace RMF */
