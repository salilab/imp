/**

 */
#include "IMP/core/StructureSet.h"
#include "IMP/core/model_io.h"
#include "IMP/Restraint.h"

IMPCORE_BEGIN_NAMESPACE

StructureSet::StructureSet(Model *m): Object("StructureSet"), m_(m){
  threshold_= std::numeric_limits<double>::max();
}
void StructureSet::add_structure(std::string name) {
  double e= m_->evaluate(false);
  if (e > threshold_) return;
  for (std::map<Restraint*, double>::iterator it= restraint_thresholds_.begin();
       it != restraint_thresholds_.end(); ++it) {
    if (it->first->evaluate(false) > it->second) return;
  }
  structures_.back().energy= m_->evaluate(false);
  structures_.push_back(Data());
  structures_.back().name=name;
  std::ostringstream oss;
  write(m_, oss);
  structures_.back().state= oss.str();
}
void StructureSet::load_structure(StructureIndex i){
  IMP_USAGE_CHECK(i < structures_.size(),
                  "Invalid structure requested: " << i, UsageException);
  std::istringstream iss(structures_[i].state);
  read(iss, m_);
}
Ints StructureSet::get_all_structures() const {
  Ints is(structures_.size());
  for (unsigned int i=0; i< is.size(); ++i) {
    is[i]=i;
  }
  return is;
}

void StructureSet::set_filter_threshold(double v) {
  threshold_=v;
}

void StructureSet::set_restraint_filter(Restraint* r, double v) {
  restraint_thresholds_[r]=v;
}

void StructureSet::show(std::ostream &out) const {
  out << "StructureSet with " << structures_.size() << " structures.\n";
}

IMPCORE_END_NAMESPACE
