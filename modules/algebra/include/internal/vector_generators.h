/**
 *  \file cgal_predicates.h
 *  \brief predicates implemented using CGAL
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
*/

#ifndef IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#define IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H
#include "../config.h"
#include <IMP/base_types.h>

#ifdef IMP_USE_CGAL
#ifndef NDEBUG
//#define NDEBUG
#define IMP_NO_HAD_NDEBUG
#endif


#include <CGAL/Cartesian_d.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Gmpq.h>
#include <CGAL/Optimisation_d_traits_d.h>
#include <CGAL/Min_sphere_d.h>
#include <CGAL/K_neighbor_search.h>
#include <CGAL/Search_traits_2.h>

#ifdef IMP_NO_HAD_NDEBUG
#undef NDEBUG
#undef IMP_NO_HAD_NDEBUG
#endif
#endif

IMPALGEBRA_BEGIN_NAMESPACE
template <unsigned int D>
VectorD<D>
random_vector_on_unit_sphere();
IMPALGEBRA_END_NAMESPACE
IMPALGEBRA_BEGIN_INTERNAL_NAMESPACE

/*If SYMMETRIC is true, then, for each returned vector, both
  it and its negation is part of the cover (and hence the cover
  is symmetric about the center of the sphere. One one of the two
  points is returned. */
template <unsigned int D, bool SYMMETRIC>
std::vector<VectorD<D> >
uniform_cover_sphere(unsigned int n,
                     const VectorD<D> &center,
                     double radius) {
  std::vector<VectorD<D> > ret;
  /*for (unsigned int i=0; i< n; ++i) {
    ret[i]= random_vector_on_unit_sphere<D>();
    }*/
#ifdef IMP_USE_CGAL

  typedef typename ::CGAL::Cartesian_d< ::CGAL::Gmpq> K;
  typedef typename K::Point_d P;
  typedef ::CGAL::Convex_hull_d<K> CH;
  typedef ::CGAL::Optimisation_d_traits_d<K>       Traits;
  typedef ::CGAL::Min_sphere_d<Traits>             Min_sphere;
  std::map<typename CH::Vertex_handle, unsigned int> indexes;
  for (unsigned int i=0; i< D; ++i) {
    ret.push_back(basis_vector<D>(i));
    ret.push_back(-basis_vector<D>(i));
  }
  CH ch(D);
  for (unsigned int j=0; j< ret.size(); ++j) {
    P p(D, ret[j].coordinates_begin(),
        ret[j].coordinates_end());
    typename CH::Vertex_handle vh=ch.insert(p);
    indexes[vh]= j;
    /*std::cout << "added vertex " << j << std::endl;
    std::cout << ret[indexes[vh]]
              << " " << ::CGAL::to_double(vh->point().cartesian(0))
              << " " << ::CGAL::to_double(vh->point().cartesian(1))
              << " " << ::CGAL::to_double(vh->point().cartesian(2))
              << std::endl;*/
  }
  /* ch.number_of_facets()+ */
  while (ch.number_of_vertices()
         <= static_cast<int>(n)) {
    std::vector<VectorD<D> > to_insert;
    for (typename CH::Facet_iterator it= ch.facets_begin();
         it != ch.facets_end(); ++it) {
      VectorD<D> v= zeros<D>();
      for (unsigned int i=0; i< D; ++i) {
        v+= ret[indexes[ch.vertex_of_facet(it, i)]];
      }
      v/=D;
      to_insert.push_back(v.get_unit_vector());
    }
    for (unsigned int i=0; i< to_insert.size(); ++i) {
      P p(D, to_insert[i].coordinates_begin(),
          to_insert[i].coordinates_end());
      typename CH::Vertex_handle vh=ch.insert(p);
      indexes[vh]= ret.size()+i;
      //std::cout << "Added vertex " << ret.size() << std::endl;
      /*std::cout << ret[indexes[vh]]
                << " " << ::CGAL::to_double(vh->point().cartesian(0))
                << " " << ::CGAL::to_double(vh->point().cartesian(1))
                << " " << ::CGAL::to_double(vh->point().cartesian(2))
                << std::endl;*/
    }
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }


#else
  IMP_WARN("cover_sphere produces bad covers if ::CGAL is not found");
  ret.resize(n);
  for (unsigned int i=0; i< n; ++i) {
    ret[i]= random_vector_on_unit_sphere<D>();
  }
#endif
  for (unsigned int i=0; i< ret.size(); ++i) {
    ret[i]= radius*ret[i]+center;
  }

  return ret;
}

#if 0

  while (ret.size() < n) {
    ret.push_back(random_vector_on_unit_sphere<D>());
    P p(D, ret.back().coordinates_begin(),
        ret.back().coordinates_end());
    indexes[ch.insert(p)]= ret.size()-1;
    if (SYMMETRIC) {
      ret.push_back(ret.back());
      P p(D, ret.back().coordinates_begin(),
          ret.back().coordinates_end());
      indexes[ch.insert(p)]= ret.size()-1;
    }
  }
  ch.clear(D);


  typedef ::CGAL::Cartesian_d<double> DK;
  typedef DK::Point_d PD;
  typedef ::CGAL::Search_traits_d<DK> TreeTraits;
  typedef ::CGAL::K_neighbor_search<TreeTraits> Neighbor_search;
  typedef Neighbor_search::Tree Tree;


  for (unsigned int i=0; i< 50; ++i) {
    if (D==3) {
      std::ostringstream oss;
      oss << "temp." << i << ".bild";
      std::ofstream outf(oss.str().c_str());
      outf << ".color " << i << std::endl;
      for (unsigned int i=0; i< n; ++i) {
        outf << ".sphere " << spaces_io(radius*ret[i]+center)
             << " .1" << std::endl;
      }
      /*for (typename CH::Facet_iterator it= ch.facets_begin();
           it != ch.facets_end(); ++it) {
        for (unsigned int j=0; j<D; ++j) {
          unsigned int index_j= indexes.find(ch.vertex_of_facet(it, j))->second;
          int index_k=indexes.find(ch.vertex_of_facet(it, (j+1)%D))->second;
          outf << ".cylinder " << spaces_io(radius*ret[index_j]+center) <<" "
                                 << spaces_io(radius*ret[index_k]+center)
<< " .1" << std::endl;
          outf << ".cylinder "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               j).cartesian(0)) +center[0]<< " "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               j).cartesian(1)) +center[1] << " "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               j).cartesian(2)) +center[2] << " "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               (j+1)%D).cartesian(0)) +center[0] << " "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               (j+1)%D).cartesian(1)) +center[1] << " "
               << radius*::CGAL::to_double(ch.point_of_facet(it,
               (j+1)%D).cartesian(2)) +center[2] << " "
               << " .1" << std::endl;
        }
        }*/
    }

    if (i%5 ==0) {
      tree.clear(D);
      std::vector<PD> pds(ret.size());
      for (unsigned int j=0; j< ret.size(); ++j) {
        P p(D, ret[j].coordinates_begin(),
            ret[j].coordinates_end());
        pds[j]=p;
      }
      tree=Tree(pds.begin(), pds.end());
    }

    // need to average them
    /*double zeros[D]={0};
    std::vector<VectorD<D> > force(n, VectorD<D>(zeros, zeros+D) );
    std::vector<int> degree(n,0);*/
    std::vector<Min_sphere> min_spheres(ret.size());
    for (typename CH::Facet_iterator it= ch.facets_begin();
         it != ch.facets_end(); ++it) {
      for (unsigned int j=0; j<D; ++j) {
        unsigned int index_j= indexes.find(ch.vertex_of_facet(it, j))->second;
        for (unsigned int k=1; k< D; ++k) {
          int index_k=indexes.find(ch.vertex_of_facet(it, (j+k)%D))->second;
          //force[index_j]+= ret[index_k];
          min_spheres[index_j].insert(P(D, ret[index_k].coordinates_begin(),
                                        ret[index_k].coordinates_end()));
          // ++degree[index_j];
        }
      }
    }
    for (unsigned int j=0; j< n; ++j) {
      //typename K::Vector_d vps=ps[j]-::CGAL::ORIGIN;
      //typename K::Vector_d v=(force[j]- (force[j]*vps)*vps)
      //+(ps[j]-::CGAL::ORIGIN);
      //VectorD<D> v= force[j]/degree[j];
      VectorD<D> v;
      for (unsigned int i=0; i< D; ++i) {
        v[i]= ::CGAL::to_double(min_spheres[j].center().cartesian(i));
      }
      ret[j]=v.get_unit_vector();
    }


  }

#endif

IMPALGEBRA_END_INTERNAL_NAMESPACE


#endif  /* IMPALGEBRA_INTERNAL_VECTOR_GENERATORS_H */
