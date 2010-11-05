/**
 *  \file point_clustering.cpp
 *  \brief Cluster sets of points.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/statistics/point_clustering.h>
#include <IMP/core/XYZ.h>
#include <IMP/statistics/internal/KMData.h>
#include <IMP/statistics/internal/KMTerminationCondition.h>
#include <IMP/statistics/internal/KMLocalSearchLloyd.h>
#include <IMP/algebra/vector_search.h>
#include <IMP/algebra/geometric_alignment.h>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif
#include <boost/pending/disjoint_sets.hpp>


IMPSTATISTICS_BEGIN_NAMESPACE

Embedding::~Embedding(){}

ConfigurationSetXYZEmbedding
::ConfigurationSetXYZEmbedding(ConfigurationSet *cs,
                               SingletonContainer *sc,
                               bool align):
  Embedding("ConfiguringEmbedding"),
  cs_(cs), sc_(sc), align_(align){}

Floats ConfigurationSetXYZEmbedding::get_point(unsigned int a) const {
  algebra::Transformation3D tr= algebra::get_identity_transformation_3d();
  if (align_) {
    cs_->load_configuration(0);
    algebra::Vector3Ds vs0;
    for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
      vs0.push_back(core::XYZ(sc_->get_particle(i)).get_coordinates());
    }
    cs_->load_configuration(a);
    algebra::Vector3Ds vsc;
    for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
      vsc.push_back(core::XYZ(sc_->get_particle(i)).get_coordinates());
    }
    tr= get_transformation_aligning_first_to_second(vsc, vs0);
  } else {
    cs_->load_configuration(a);
  }
  Floats ret(sc_->get_number_of_particles()*3);
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    algebra::Vector3D v
      = tr.get_transformed(core::XYZ(sc_->get_particle(i)).get_coordinates());
    ret[3*i]= v[0];
    ret[3*i+1]= v[1];
    ret[3*i+2]= v[2];
  }
  return ret;
}

unsigned int ConfigurationSetXYZEmbedding::get_number_of_points() const {
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

void ParticleEmbedding::do_show(std::ostream &out) const {
  for (unsigned int i=0; i< ks_.size(); ++i) {
    out << ks_[i] << " ";
  }
  out << std::endl;
}




unsigned int PartitionalClusteringWithCenter::get_number_of_clusters() const {
  IMP_CHECK_OBJECT(this);
  return clusters_.size();
}
const Ints&PartitionalClusteringWithCenter::get_cluster(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  return clusters_[i];
}
int PartitionalClusteringWithCenter
::get_cluster_representative(unsigned int i) const {
  IMP_CHECK_OBJECT(this);
  IMP_USAGE_CHECK(i < get_number_of_clusters(),
                      "There are only " << get_number_of_clusters()
                      << " clusters. Not " << i);
  return reps_[i];
}
void PartitionalClusteringWithCenter::do_show(std::ostream &out) const {
  out << centers_.size() << " centers." << std::endl;
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




PartitionalClusteringWithCenter*
get_lloyds_kmeans(const Ints &names, Embedding *metric,
                  unsigned int k, unsigned int iterations) {
  metric->set_was_used(true);
  IMP_USAGE_CHECK(k < iterations,
                  "You probably switched the k and iterations parameters."
                  << " The former should be (much) smaller than the latter.");
  if (names.size() < k) {
    IMP_THROW("Too few points to make into " << k << " clusters.",
              ValueException);
  }
  IMP_LOG(VERBOSE,"KMLProxy::run start \n");
  //use the initial centers if provided
  boost::scoped_ptr<internal::KMPointArray> kmc;
  IMP_LOG(VERBOSE,"KMLProxy::run load initial guess \n");
  //load the initail guess
  internal::KMData data(metric->get_point(names[0]).size(), names.size());
  for (unsigned int i=0; i< names.size(); ++i) {
    *(data[i])= metric->get_point(names[i]);
  }
  internal::KMFilterCenters ctrs(k, &data, NULL, 1.0);

  //apply lloyd search
  IMP_LOG(VERBOSE,"KMLProxy::run load lloyd \n");
  internal::KMTerminationCondition term;
  internal::KMLocalSearchLloyd la(&ctrs,&term);
  IMP_LOG(VERBOSE,"KMLProxy::run excute lloyd \n");
  la.execute();
  internal::KMFilterCentersResults best_clusters = la.get_best();
  IMP_INTERNAL_CHECK(k
                     == (unsigned int) best_clusters.get_number_of_centers(),
             "The final number of centers does not match the requested one");
  IMP_LOG(VERBOSE,"KMLProxy::run load best results \n");
  std::vector<Floats> centers(k);
  for (unsigned int i = 0; i < k; i++) {
    internal::KMPoint *kmp = best_clusters[i];
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

  PartitionalClusteringWithCenter *cl
    = new PartitionalClusteringWithCenter(clusters, centers, reps);
  cl->set_was_used(true);
  return cl;
}


PartitionalClusteringWithCenter* get_lloyds_kmeans(Embedding *metric,
                                    unsigned int k, unsigned int iterations) {
  Ints names(metric->get_number_of_points());
  for (unsigned int i=0; i< names.size(); ++i) {
    names[i]=i;
  }
  return get_lloyds_kmeans(names, metric, k, iterations);
}


PartitionalClusteringWithCenter*
get_connectivity_clustering(Embedding *embed,
                            double dist) {
  IMP_USAGE_CHECK(embed->get_number_of_points() >0,
                  "There most be a point to clustering");
  IMP_USAGE_CHECK(embed->get_point(0).size() ==3,
                  "Can currently only use connectivity clustering in 3D.");
  algebra::Vector3Ds vs(embed->get_number_of_points());
  for (unsigned int i=0; i< vs.size(); ++i) {
    Floats fs= embed->get_point(i);
    vs[i]= algebra::Vector3D(fs.begin(), fs.end());
  }
  algebra::NearestNeighborD<3> nn(vs.begin(), vs.end(), .1);
  typedef boost::vector_property_map<unsigned int> Index;
  typedef Index Parent;
  typedef boost::disjoint_sets<Index,Parent> UF;
  Index id;
  Parent pt;
  UF uf(id, pt);
  for (unsigned int i=0; i< vs.size(); ++i) {
    uf.make_set(i);
  }
  for (unsigned int i=0; i< vs.size(); ++i) {
    Ints ns= nn.get_in_ball(i, dist);
    for (unsigned int j=0; j < ns.size(); ++j) {
      //std::cout << "Unioning " << i << " and " << ns[j] << std::endl;
      uf.union_set(static_cast<int>(i), ns[j]);
    }
  }
  std::map<int,int> cluster_map;
  Ints reps;
  std::vector<Ints> clusters;
  algebra::Vector3Ds centers;
  for (unsigned int i=0; i < vs.size(); ++i) {
    int p= uf.find_set(i);
    if (cluster_map.find(p) == cluster_map.end()) {
      cluster_map[p]= clusters.size();
      clusters.push_back(Ints());
      centers.push_back(algebra::get_zero_vector_d<3>());
      reps.push_back(i);
    }
    int ci= cluster_map.find(p)->second;
    clusters[ci].push_back(i);
    centers[ci] += vs[i];
  }
  for (unsigned int i=0; i< clusters.size(); ++i) {
    centers[i]/= clusters[i].size();
    double md=std::numeric_limits<double>::max();
    for (unsigned int j=0; j < clusters[i].size(); ++j) {
      double d= get_distance(centers[i], vs[clusters[i][j]]);
      if (d < md) {
        md=d;
        reps[i]=clusters[i][j];
      }
    }
  }
  return new PartitionalClusteringWithCenter(clusters, centers, reps);
}

IMPSTATISTICS_END_NAMESPACE
