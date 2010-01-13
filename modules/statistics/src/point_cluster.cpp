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

Floats ConfigurationSetXYZEmbedding::get_embedding(unsigned int a) const {
  cs_->set_configuration(a);
  Floats ret(sc_->get_number_of_particles()*3);
  for (unsigned int i=0; i< sc_->get_number_of_particles(); ++i) {
    ret[3*i]= core::XYZ(sc_->get_particle(i)).get_coordinates()[0];
    ret[3*i+1]= core::XYZ(sc_->get_particle(i)).get_coordinates()[1];
    ret[3*i+2]= core::XYZ(sc_->get_particle(i)).get_coordinates()[2];
  }
  return ret;
}

void ConfigurationSetXYZEmbedding::show(std::ostream &out) const {
  out << "ConfigurationSetXYZEmbedding with ";
  cs_->show(out);
  sc_->show(out);
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
    *(data[i])= metric->get_embedding(names[i]);
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
/*
  Pointer<Embedding> pm(metric);
  boost::uninform_int rng(0, data.size());
  std::vector<Floats> data(names.size());
  for (unsigned int i=0; i< names.size(); ++i) {
    data[i]= metric->get_embedding(names[i]);
  }
  Int centers(k);
  for (unsigned int i=0; i< k; ++i) {
    centers[i]= data[rng(random_number_generator)];
  }
  std::vector<Ints> clusters(k);
  Floats changes(k), averages(k);
  assign(names, data, centers, clusters, averages);

  for (unsigned int it=0; i< iterations; ++it) {
    compute_centers(data, clusters, centers, changes);
    assign(data, centers, clusters, averages);
    bool changed_enough=false;
    for (unsigned int i=0;  i< k; ++i) {
      if (changes[i]*100 > averages[i]) {
        changed_enough=true;
        break;
      }
    }
    if (!changed_enough) {
      IMP_LOG(TERSE, "No large movements, ending lloyds iterations."
      << std::endl);
      break;
    }
  }

  Ints reps(k);
  for (unsigned int i=0; i< k; ++i) {
    int c=-1;
    double d= std::numeric_limits<double>::max();
    for (unsigned int j=0; j< clusters[i].size(); ++j) {
      double cd= distance(data[clusters[i][j]], centers[i]);
      if (cd < d) {
        d= cd;
        c= j;
      }
    }
    reps[i]=names[c];
  }
  std::vector<Ints> rclusters(k);
  for (unsigned int i=0; i< k; ++i) {
    rclusters[i].resize(clusters[i].size());
    for (unsigned int j=0; j < clusters[i].size(); ++j) {
      rclusters[i][j]=name[clusters[i][j]];
    }
  }
  KMeansClustering *cl= new KMeansClustering(rclusters, reps, centers);
  return cl;




  void accumulate(Floats &a, const Floats &b) {
    for (unsigned int i=0; i< a.size(); ++i) {
      a[i]+= b[i];
    }
  }
  void assign(std::vector<Floats> &data,
              const std::vector<Floats> &centers,
              std::vector<Ints> &clusters,
              Floats &averages) {
    for (unsigned int i=0; i< clusters.size(); ++i) {
      clusters[i].clear();
      averages[i]=0;
    }
    for (unsigned int i=0; i< data.size(); ++i) {
      double d= std::numeric_limits<double>::max();
      int center=-1;
      for (unsigned int j=0; j< centers.size(); ++j) {
        double dc= distance(data[i], centers[j]);
        if (dc <=d) {
          center=j;
          d= dc;
        }
      }
      clusters[center].push_back(i);
      averages[center]+= d;
    }
    for (unsigned int i=0; i< clusters.size(); ++i) {
      averages[i]/= clusters[i].size();
    }
  }

  void compute_centers(std::vector<Floats>&data,
                         const std::vector<Ints> &clusters,
                         Ints &centers, Floats &moves) {
    for (unsigned int i=0; i< clusters.size(); ++i) {
      Floats c(data[0].size(), 0);
      for (unsigned int j=0; j<  clusters[i].size(); ++j) {
        accumulate(c, data[clusters[i][j]])
      }
      moves[i]= distance(c, centers[i]);
      centers[i]=c;
    }
  }
*/

IMPSTATISTICS_END_NAMESPACE
