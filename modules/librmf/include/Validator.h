/**
 *  \file RMF/Validator.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_VALIDATOR_H
#define IMPLIBRMF_VALIDATOR_H

#include "RMF_config.h"
#include "RootHandle.h"

namespace RMF {
/** Validators check invariants of the RMF hierarchy. Use the
    IMP_RMF_VALIDATOR() macro to register them. They should write
    information about any errors encountered to the provided
    stream.
*/
class RMFEXPORT Validator {
  std::string name_;
  RootHandle rh_;
public:
  /** This method is called with the root handle when
      validation is required.
  */
  virtual void write_errors(std::ostream &out) const=0;
  virtual ~Validator();
protected:
  RootHandle get_root_handle() const {
    return rh_;
  }
  Validator(RootHandle rh, std::string name);
};

class RMFEXPORT NodeValidator: public Validator {
public:
  /** This method traverses the node and calls
      validate_node() on each.
  */
  void write_errors(std::ostream &out) const;
protected:
  NodeValidator(RootHandle rh, std::string name);
  virtual void write_errors_node(NodeHandle node,
                             const NodeHandles &path,
                             std::ostream &out) const=0;
};


#if !defined(IMP_DOXYGEN) && !defined(SWIG)
struct Creator: public boost::intrusive_ptr_object {
  std::string name_;
  Creator(std::string name): name_(name){}
  virtual Validator *create(RootHandle rh)=0;
};
template <class V>
struct CreatorImpl: public Creator {
  CreatorImpl(std::string name): Creator(name){}
  Validator *create(RootHandle rh) {
    return new V(rh, name_);
  }
};
typedef vector<boost::intrusive_ptr<Creator> > Creators;
RMFEXPORT Creators& get_validators();

template <class V>
struct Registrar {
  Registrar(std::string name) {
    get_validators().push_back(new CreatorImpl<V>(name));
  }
};
#endif

} /* namespace RMF */

#endif /* IMPLIBRMF_VALIDATOR_H */
