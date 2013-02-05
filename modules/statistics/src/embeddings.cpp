/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/embeddings.h>
#include <IMP/algebra/Transformation3D.h>
#include <IMP/singleton_macros.h>
#include <IMP/algebra/geometric_alignment.h>
IMPSTATISTICS_BEGIN_NAMESPACE

namespace {
  algebra::Vector3D get_coordinates(Particle *p) {
    algebra::Vector3D ret(p->get_value(IMP::kernel::internal::xyzr_keys[0]),
                          p->get_value(IMP::kernel::internal::xyzr_keys[1]),
                          p->get_value(IMP::kernel::internal::xyzr_keys[2]));
    return ret;
  }
}


algebra::VectorKD VectorDEmbedding::get_point(unsigned int i) const {
  return vectors_[i];
}

unsigned int VectorDEmbedding::get_number_of_items() const {
  return vectors_.size();
}

void VectorDEmbedding::do_show(std::ostream &) const {
}

ConfigurationSetXYZEmbedding
::ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainerAdaptor sc,
                               bool align):
  Embedding("ConfiguringEmbedding"),
  cs_(cs), sc_(sc), align_(align){
}

algebra::VectorKD
ConfigurationSetXYZEmbedding::get_point(unsigned int a) const {
  algebra::Transformation3D tr= algebra::get_identity_transformation_3d();
  if (align_) {
    cs_->load_configuration(0);
    algebra::Vector3Ds vs0;
    IMP_FOREACH_SINGLETON(sc_, {
        vs0.push_back(get_coordinates(_1));
      });
    cs_->load_configuration(a);
    algebra::Vector3Ds vsc;
    IMP_FOREACH_SINGLETON(sc_, {
        vsc.push_back(get_coordinates(_1));
      });
    tr= get_transformation_aligning_first_to_second(vsc, vs0);
  } else {
    cs_->load_configuration(a);
  }
  Floats ret;
  IMP_FOREACH_SINGLETON(sc_, {
    algebra::Vector3D v
      = tr.get_transformed(get_coordinates(_1));
    ret.push_back(v[0]);
    ret.push_back(v[1]);
    ret.push_back(v[2]);
    });
  return algebra::VectorKD(ret.begin(), ret.end());
}

unsigned int ConfigurationSetXYZEmbedding::get_number_of_items() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetXYZEmbedding::do_show(std::ostream &out) const {
  cs_->show(out);
  sc_->show(out);
}


ParticleEmbedding::ParticleEmbedding(const ParticlesTemp &ps,
                                     const FloatKeys &ks,
                                     bool rescale):
  Embedding("ParticleEmbedding"),
  ps_(ps.begin(), ps.end()), ks_(ks), rescale_(rescale){
  if (rescale && !ps.empty()) {
    ranges_.resize(ks.size());
    for (unsigned int i=0; i< ks.size(); ++i) {
      FloatRange r=ps[0]->get_model()->get_range(ks[i]);
      ranges_[i]= FloatRange(r.first, 1.0/(r.second-r.first));
    }
  }
}

algebra::VectorKD ParticleEmbedding::get_point(unsigned int i) const {
  Floats ret(ks_.size());
  for (unsigned int j=0; j< ks_.size(); ++j) {
    ret[j]= ps_[i]->get_value(ks_[j]);
  }
  if (rescale_) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      ret[j]= (ret[j]-ranges_[j].first)*ranges_[j].second;
    }
  }
  return algebra::VectorKD(ret.begin(), ret.end());
}

unsigned int ParticleEmbedding::get_number_of_items() const {
  return ps_.size();
}

void ParticleEmbedding::do_show(std::ostream &out) const {
  for (unsigned int i=0; i< ks_.size(); ++i) {
    out << ks_[i] << " ";
  }
  out << std::endl;
}


IMPSTATISTICS_END_NAMESPACE
