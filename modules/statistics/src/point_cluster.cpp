/**
 *  \file point_clustering.cpp
 *  \brief Holds data points to cluster using k-means
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */
#include <IMP/statistics/point_clustering.h>
#include <IMP/core/XYZ.h>
#include <IMP/statistics/KMData.h>
#include <IMP/statistics/KMTerminationCondition.h>
#include <IMP/statistics/KMLocalSearchLloyd.h>

IMPSTATISTICS_BEGIN_NAMESPACE
Embedding::~Embedding(){}

ConfigurationSetXYZEmbedding
::ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainer *sc):
  cs_(cs), sc_(sc){}

Floats ConfigurationSetXYZEmbedding::get_point(unsigned int a) const {
  cs_->set_configuration(a);
  Floats ret(sc_->get_number_of_particles()*3);
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    ret[3*i]= core::XYZ(sc_->get_particle(i)).get_coordinates()[0];
    ret[3*i+1]= core::XYZ(sc_->get_particle(i)).get_coordinates()[1];
    ret[3*i+2]= core::XYZ(sc_->get_particle(i)).get_coordinates()[2];
  }
  return ret;
}

unsigned int ConfigurationSetXYZEmbedding::get_number_of_points() const {
  return cs_->get_number_of_configurations();
}

void ConfigurationSetXYZEmbedding::show(std::ostream &out) const {
  out << "ConfigurationSetXYZEmbedding with ";
  cs_->show(out);
  sc_->show(out);
}


ParticleEmbedding::ParticleEmbedding(const ParticlesTemp &ps,
                                     const FloatKeys &ks,
                                     bool rescale):
  Embedding("ParticleEmbedding"),
  ps_(ps), ks_(ks), rescale_(rescale){
  if (rescale && !ps.empty()) {
    ranges_.resize(ks.size());
    for (unsigned int i=0; i< ks.size(); ++i) {
      FloatRange r=ps[0]->get_model()->get_range(ks[i]);
      ranges_[i]= FloatRange(r.first, 1.0/(r.second-r.first));
    }
  }
}

Floats ParticleEmbedding::get_point(unsigned int i) const {
  Floats ret(ks_.size());
  for (unsigned int j=0; j< ks_.size(); ++j) {
    ret[j]= ps_[i]->get_value(ks_[j]);
  }
  if (rescale_) {
    for (unsigned int j=0; j< ks_.size(); ++j) {
      ret[j]= (ret[j]-ranges_[j].first)*ranges_[j].second;
    }
  }
  return ret;
}

unsigned int ParticleEmbedding::get_number_of_points() const {
  return ps_.size();
}

void ParticleEmbedding::show(std::ostream &out) const {
  out << "ParticleEmbedding with ";
  for (unsigned int i=0; i< ks_.size(); ++i) {
    out << ks_[i] << " ";
  }
  out << std::endl;
}


HighDensityEmbedding::HighDensityEmbedding(em::DensityMap *dm,
                                           double threshold):
  Embedding("HighDensityEmbedding of "+dm->get_name()) {
  for (unsigned int i=0; i< dm->get_number_of_voxels(); ++i) {
    if (dm->get_value(i) > threshold) {
      algebra::Vector3D v(dm->voxel2loc(i, 0),
                          dm->voxel2loc(i, 1),
                          dm->voxel2loc(i, 2));
      points_.push_back(v);
    }
  }
}

Floats HighDensityEmbedding::get_point(unsigned int i) const {
  return Floats(points_[i].coordinates_begin(),
                points_[i].coordinates_end());
}

unsigned int HighDensityEmbedding::get_number_of_points() const {
  return points_.size();
}

void HighDensityEmbedding::show(std::ostream &out) const {
  out << "HighDensityEmbedding with " << points_.size()
      << " points.";
  out << std::endl;
}




unsigned int KMeansClustering::get_number_of_clusters() const {
  return clusters_.size();
}
const Ints&KMeansClustering::get_cluster(unsigned int i) const {
  return clusters_[i];
}
int KMeansClustering::get_cluster_representative(unsigned int i) const {
  return reps_[i];
}
void KMeansClustering::show(std::ostream &out) const {
  out << "Clustering with " << centers_.size() << " centers." << std::endl;
}


namespace {
  double compute_distance(const Floats &a, const Floats &b) {
    double d=0;
    for (unsigned int i=0; i< a.size(); ++i) {
      d+= square(a[i]-b[i]);
    }
    return std::sqrt(d);
  }
}




KMeansClustering* get_lloyds_kmeans(const Ints &names, Embedding *metric,
                                    unsigned int k, unsigned int iterations) {
  metric->set_was_owned(true);
  IMP_USAGE_CHECK(k < iterations,
                  "You probably switched the k and iterations parameters."
                  << " The former should be (much) smaller than the latter.",
                  ValueException);
  IMP_LOG(VERBOSE,"KMLProxy::run start \n");
  //use the initial centers if provided
  std::auto_ptr<KMPointArray> kmc;
  IMP_LOG(VERBOSE,"KMLProxy::run load initial guess \n");
  //load the initail guess
  KMData data(k, names.size());
  for (unsigned int i=0; i< names.size(); ++i) {
    *(data[i])= metric->get_point(names[i]);
  }
  KMFilterCenters ctrs(k, &data, NULL, 1.0);

  //apply lloyd search
  IMP_LOG(VERBOSE,"KMLProxy::run load lloyd \n");
  KMTerminationCondition term;
  KMLocalSearchLloyd la(&ctrs,&term);
  IMP_LOG(VERBOSE,"KMLProxy::run excute lloyd \n");
  la.execute();
  KMFilterCentersResults best_clusters = la.get_best();
  IMP_INTERNAL_CHECK(k
                     == (unsigned int) best_clusters.get_number_of_centers(),
             "The final number of centers does not match the requested one");
  IMP_LOG(VERBOSE,"KMLProxy::run load best results \n");
  std::vector<Floats> centers(k);
  for (unsigned int i = 0; i < k; i++) {
    KMPoint *kmp = best_clusters[i];
    centers[i]=*kmp;
  }
  //set the assignment of particles to centers
  //array of number of all points
  //TODO - return this
  IMP_LOG(VERBOSE,"KMLProxy::run get assignments \n");
  const std::vector<int> &close_center = *best_clusters.get_assignments();
  IMP_LOG(VERBOSE,"KMLProxy::run get assignments 2\n");
  std::vector<Ints> clusters(k);
  for (unsigned int i=0;i<names.size();i++) {
    //std::cout<<"ps number i: " << i << " close center : "
    //<< (*close_center)[i] << std::endl;
    clusters[close_center[i]].push_back(names[i]);
  }
  Ints reps(k);
  for (unsigned int i=0; i< k; ++i) {
    int c=-1;
    double d= std::numeric_limits<double>::max();
    for (unsigned int j=0; j< clusters[i].size(); ++j) {
      double cd= compute_distance(*(data[clusters[i][j]]), centers[i]);
      if (cd < d) {
        d= cd;
        c= j;
      }
    }
    reps[i]=names[c];
  }

  KMeansClustering *cl= new KMeansClustering(clusters, centers, reps);
  return cl;
}


KMeansClustering* get_lloyds_kmeans(Embedding *metric,
                                    unsigned int k, unsigned int iterations) {
  Ints names(metric->get_number_of_points());
  for (unsigned int i=0; i< names.size(); ++i) {
    names[i]=i;
  }
  return get_lloyds_kmeans(names, metric, k, iterations);
}


IMPSTATISTICS_END_NAMESPACE
