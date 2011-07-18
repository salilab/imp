/**
 *  \file PMFTable.h
 *  \brief Functions to read mol2s
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */
#ifndef IMPCORE_INTERNAL_GRID_CLOSE_PAIRS_IMPL_H
#define IMPCORE_INTERNAL_GRID_CLOSE_PAIRS_IMPL_H

#include "../core_config.h"
#include "../GridClosePairsFinder.h"
#include "sinks.h"
#include "IMP/core/QuadraticClosePairsFinder.h"
#include "IMP/core/XYZR.h"
#include <IMP/algebra/Grid3D.h>
#include <IMP/core/utility.h>

IMPCORE_BEGIN_INTERNAL_NAMESPACE

inline bool get_interiors_intersect(const algebra::Vector3D &v,
                                    double ra, double rb) {
  double sr= ra+rb;
  double a0=std::abs(v[0]);
  if (a0 >= sr) return false;
  double a1=std::abs(v[1]);
  if (a1 >= sr) return false;
  double a2=std::abs(v[2]);
  if (a2 >= sr) return false;
  return v*v < square(sr);
}
struct ParticleTraits {
  typedef ParticleIndex ID;
  Model *m_;
  double d_;
  ParticleTraits(Model *m, double d): m_(m), d_(d){}
  ParticleIndex get_id(Particle* p, int) const {return p->get_index();}
  algebra::Vector3D get_center(ParticleIndex p, int) const {
    return m_->get_sphere(p).get_center();
  }
  double get_radius(ParticleIndex p, int) const {
    return m_->get_sphere(p).get_radius();
  }
  bool get_is_close(ParticleIndex a, ParticleIndex b) const {
    return get_interiors_intersect(get_center(a,0)
                                   -get_center(b,0),
                                   get_radius(b,0)+d_,
                                   get_radius(a,0));
  }
  bool check_close(ParticleIndex a, ParticleIndex b) const {
    return get_interiors_intersect(get_center(a,0)
                                   -get_center(b,0),
                                   .95*(get_radius(b, 0)+d_),
                                   .95*(get_radius(a, 0)));
  }
  double get_distance() const {
    return d_;
  }
};


struct BoundingBoxTraits {
  typedef unsigned int ID;
  algebra::BoundingBox3Ds::const_iterator it0_, it1_;
  double d_;
  BoundingBoxTraits(algebra::BoundingBox3Ds::const_iterator it0,
                    algebra::BoundingBox3Ds::const_iterator it1, double d):
    it0_(it0), it1_(it1), d_(d){}
  algebra::BoundingBox3Ds::const_iterator get_it(int which) const {
    switch(which) {
    case 0:
      return it0_;
    default:
      return it1_;
    }
  }
  unsigned int get_id(const algebra::BoundingBox3D &bb, int which) const {
    return &bb-&*get_it(which);
  }
  double get_radius(unsigned int i, int which) const {
    return algebra::get_maximum_length(*(get_it(which)+i))/2.0;
  }
  algebra::Vector3D get_center(unsigned int i, int which) const {
    return algebra::Vector3D(.5*((get_it(which)+i)->get_corner(0)
                                 +(get_it(which)+i)->get_corner(1)));
  }
  bool get_is_close(unsigned int a, unsigned int b) const {
    algebra::BoundingBox3D ag= *(it0_+a)+d_;
    return algebra::get_interiors_intersect(ag, *(it1_+b));
  }
  bool check_close(unsigned int, unsigned int) const {
    return false;
  }
  double get_distance() const {
    return d_;
  }
};

struct BBPairSink {
  IntPairs &out_;
  BBPairSink(IntPairs &out): out_(out){}
  typedef IntPair argument_type;
  bool operator()(unsigned int a,
                  unsigned int b) const {
    out_.push_back(IntPair(a,b));
    return true;
  }
  bool check_contains(unsigned int, unsigned int) const {
    return true;
  }
};


template <class Traits>
struct Helper {
  typedef typename Traits::ID ID;
  struct IDs: public std::vector<ID> {
    int which_;
    IDs(int which): which_(which){}
    IDs(ID id, int which): std::vector<ID>(1,id), which_(which){}
    IDs(): which_(-1){}
  };
  static inline std::string do_show(const IDs&p) {
    std::ostringstream oss;
    for (unsigned int i=0; i< p.size(); ++i) {
      oss << do_show(p[i]) << " ";
    }
    return oss.str();
  }

  typedef typename algebra::SparseGrid3D<IDs> Grid;
  typedef std::vector<Grid> Grids;

  template <class It>
  struct ParticleSet {
    It b_, e_;
    int which_;
    ParticleSet(It b, It e, int which): b_(b), e_(e),
                                        which_(which){}
  };
  template <class It>
  static ParticleSet<It> get_particle_set(It b, It e,
                                          int which) {
    return ParticleSet<It>(b,e,which);
  }
  template <class It>
  struct IDSet {
    It b_, e_;
    int which_;
    IDSet(It b, It e, int which): b_(b), e_(e),
                                  which_(which){}
  };
  template <class It>
  static IDSet<It> get_id_set(It b, It e,
                              int which) {
    return IDSet<It>(b,e, which);
  }


  static int get_index(int ii, int which, int lb, int ub) {
    if (which==1) {
      if (ii== lb) ii= ub;
      else if (ii == ub) ii= lb;
    }
    return ii;
  }


  static std::vector<typename Grid::Index> get_nearby(const Grid &g,
                                  typename Grid::ExtendedIndex center,
                                                      double half) {
    std::vector<typename Grid::Index> out;
    typename Grid::ExtendedIndex lb(center.get_offset(-1, -1, -1)),
      ub(center.get_offset(1, 1, 1));
    if (half) {
      IMP_GRID3D_FOREACH_SMALLER_EXTENDED_INDEX_RANGE(g, center, lb, ub,
                                                      {
                                           typename Grid::ExtendedIndex
                                                          ei(voxel_index[0],
                                                             voxel_index[1],
                                                             voxel_index[2]);
                                             if (g.get_has_index(ei)) {
                                        out.push_back(g.get_index(ei));
                                                        }
                                                      }
                                                      );
    } else {
      for (typename Grid::ExtendedIndexIterator it
             = g.extended_indexes_begin(lb, ub);
           it != g.extended_indexes_end(lb, ub); ++it) {
        if (g.get_has_index(*it)) {
          out.push_back(g.get_index(*it));
        }
      }
    }
    return out;
  }

  template <class It>
  static double get_max_radius(const ParticleSet<It> &ps0, const Traits &tr) {
    double maxr=0;
    for (It c= ps0.b_; c != ps0.e_; ++c) {
      maxr=std::max(tr.get_radius(tr.get_id(*c, ps0.which_), ps0.which_), maxr);
    }
    return maxr;
  }

  static algebra::BoundingBox3D get_bb(const IDs& ids, const Traits& tr) {
    algebra::BoundingBox3D bb;
    for (typename IDs::const_iterator c= ids.begin(); c != ids.end(); ++c) {
      bb+=tr.get_center(*c, ids.which_);
    }
    return bb;
  }


  template <class It>
  static void partition_points(const ParticleSet<It> &ps,
                               const Traits &tr,
                               typename std::vector<IDs > &bin_contents,
                               typename std::vector<double> &bin_ubs) {
    bin_contents.push_back(IDs(ps.which_));
    for (It c= ps.b_; c != ps.e_; ++c) {
      double cr= tr.get_radius(tr.get_id(*c, ps.which_), ps.which_)+0;
      while (cr < std::max(.5*bin_ubs.back()-.5*tr.get_distance(), 0.0)) {
        double v=std::max(.5*bin_ubs.back()-.5*tr.get_distance(), 0.1);
        if (v > .1) {
          bin_ubs.push_back(v);
        } else break;
      }
      for ( int i=bin_ubs.size()-1; i >=0; --i) {
        if (cr <= bin_ubs[i] || i==0) {
          while (static_cast<int>(bin_contents.size()) < i+1) {
            bin_contents.push_back(IDs(ps.which_));
          }
          bin_contents[i].push_back(tr.get_id(*c, ps.which_));
          break;
        }
      }
    }
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      int total=0;
      for (unsigned int i=0; i< bin_contents.size(); ++i) {
        total+= bin_contents[i].size();
      }
      IMP_INTERNAL_CHECK(total == std::distance(ps.b_, ps.e_),
                         "Lost points " << total << " "
                         << std::distance(ps.b_, ps.e_));
    }
  }

  static Grid create_grid(const algebra::BoundingBox3D &bb,
                          double side) {
    return Grid(side, bb);
  }

  static void fill_grid(const IDs &ids,
                        const Traits &tr,
                        Grid &g) {
    for (typename IDs::const_iterator c= ids.begin(); c != ids.end(); ++c) {
      algebra::Vector3D v= tr.get_center(*c, ids.which_);
      typename Grid::ExtendedIndex ind
        =g.get_nearest_extended_index(v);
      if (g.get_has_index(ind)) {
        g[g.get_index(ind)].push_back(*c);
      } else {
        g.add_voxel(ind, IDs(*c, ids.which_));
      }
    }
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "Grid built" << std::endl);
      for (typename Grid::AllConstIterator it= g.all_begin();
           it != g.all_end(); ++it) {
        IMP_INTERNAL_CHECK(it->second.size() >0,
                           "Empty voxel");
        IMP_LOG(VERBOSE, "Voxel " << it->first << " has ");
        for (unsigned int i=0; i< it->second.size(); ++i) {
          IMP_LOG(VERBOSE, do_show(it->second[i]) << " ");
        }
        IMP_LOG(VERBOSE, std::endl);
      }
    }
  }




  template <class It, class Out>
  static bool do_fill_close_pairs_from_list(It b, It e,
                                            const Traits &tr,
                                            Out out) {
    for (It c= b; c != e; ++c) {
      for (It cp= b; cp != c; ++cp) {
        if (tr.get_is_close(*c, *cp)) {
          IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                  << do_show(*cp) << std::endl);
          if (!out(*c, *cp)) {
            return false;
          }
        }
      }
    }
    return true;
  }






  template <class ItA, class ItB, class Out>
  static bool do_fill_close_pairs_from_lists(ItA ab, ItA ae,
                                             ItB bb, ItB be,
                                             const Traits& tr,
                                             Out out) {
    for (ItA c= ab; c != ae; ++c) {
      for (ItB cp= bb; cp != be; ++cp) {
        if (tr.get_is_close(*c, *cp)) {
          IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                  << do_show(*cp) << std::endl);
          if (!out( *c, *cp)) {
            return false;
          }
        }
      }
    }
    return true;
  }



  template <class Out>
  static bool do_fill_close_pairs(const Grid &gg,
                                  typename Grid::Index index,
                                  const IDs &qps,
                                  bool half, const Traits &tr, Out& out) {
    const std::vector<typename Grid::Index> ids
      = get_nearby(gg, gg.get_extended_index(index), half);
    for (unsigned int i=0; i< ids.size(); ++i) {
      const IDs& ppt= gg[ids[i]];
      IMP_LOG(VERBOSE, "Checking pair " << ids[i] << " " << index
              << ": " << do_show(ppt)
              << " and " << index
              << " which is " << do_show(qps) << std::endl);
      IMP_INTERNAL_CHECK(!half || ids[i] != index,
                         "Index returned by get nearby");
      if (!do_fill_close_pairs_from_lists(ppt.begin(),
                                          ppt.end(),
                                          qps.begin(), qps.end(),
                                          tr, out)) {
        return false;
      }
    }
    if (half) {
      IMP_LOG(VERBOSE, "Checking pair " << index << " " << index
              << std::endl);
      if (!do_fill_close_pairs_from_list(gg[index].begin(),
                                         gg[index].end(),
                                         tr, out)) {
        return false;
      }
    }
    return true;
  }

  template <class Out>
  static bool do_fill_close_pairs(const Grid &gg,
                                  typename Grid::Index index,
                                  const IDs &qps,
                                  const algebra::BoundingBox3D &bb,
                                  bool half, const Traits& tr,
                                  Out& out) {
    typename Grid::ExtendedIndex bblb
      = gg.get_extended_index(bb.get_corner(0));
    typename Grid::ExtendedIndex bbub
      = gg.get_extended_index(bb.get_corner(1));
    const std::vector<typename Grid::Index> ids
      = get_nearby(gg, gg.get_extended_index(index),
                   bblb, bbub, half);
    for (unsigned int i=0; i< ids.size(); ++i) {
      IMP_LOG(VERBOSE, "Checking pair " << ids[i] << " " << index
              << ": " << do_show(gg[ids[i]])
              << " and " << do_show(gg[index])
              << " which is " << do_show(qps) << std::endl);
      if (!do_fill_close_pairs_from_lists(gg[ids[i]].begin(),
                                          gg[ids[i]].end(),
                                          qps.begin(), qps.end(),
                                          tr, out)) {
        return false;
      }
    }
    if (half) {
      IMP_LOG(VERBOSE, "Checking pair " << index << " " << index
              << std::endl);
      if (!do_fill_close_pairs_from_list(gg[index].begin(),
                                         gg[index].end(),
                                         tr, out)) {
        return false;
      }
    }
    return true;
  }


  template <class It, class Out>
  static bool fill_close_pairs(const ParticleSet<It> &ps,
                               const Traits &tr,
                               Out out) {
    double maxr=get_max_radius(ps, tr);
    std::vector<IDs > bin_contents_g;
    std::vector<double> bin_ubs;
    bin_ubs.push_back(maxr);

    partition_points(ps, tr, bin_contents_g, bin_ubs);

    IMP_LOG(VERBOSE, "Divided points into " << bin_contents_g.size()
            << " bins ("
            << bin_ubs.size() << ") " << maxr << std::endl);
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "For G, contents are " << std::endl);
      for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
        IMP_LOG(VERBOSE, i << " " << bin_ubs[i] << " "
                << bin_contents_g[i].size()
                << std::endl);
      }
    }
    std::vector<algebra::BoundingBox3D> bbs(bin_contents_g.size());
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      bbs[i]= get_bb(bin_contents_g[i], tr);
    }
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      if (bin_contents_g[i].empty()) continue;
      if (bin_contents_g[i].size() < 10) {
        if (!do_fill_close_pairs_from_list(bin_contents_g[i].begin(),
                                           bin_contents_g[i].end(),
                                           tr, out)) {
          return false;
        }
      } else {
        Grid gg = create_grid(bbs[i], std::max(tr.get_distance()+2*bin_ubs[i],
                                               .01*(bbs[i].get_corner(1)
                                                    -bbs[i].get_corner(0))
                                               .get_magnitude()));
        fill_grid(bin_contents_g[i], tr, gg);
        for (typename Grid::AllConstIterator it
               = gg.all_begin();
             it != gg.all_end(); ++it) {
          if (!do_fill_close_pairs(gg, it->first, it->second,true,
                                   tr, out)) {
            return false;
          }
        }
      }

      IMP_IF_CHECK(USAGE_AND_INTERNAL) {
        for (unsigned int k=0; k< bin_contents_g[i].size(); ++k) {
          for (unsigned int j=0; j< k; ++j) {
            if (tr.check_close(bin_contents_g[i][k], bin_contents_g[i][j])) {
              out.check_contains(bin_contents_g[i][k],
                                 bin_contents_g[i][j]
                                 );
            }
          }
        }
      }

      for (unsigned int j=0; j< i; ++j) {
        if (bin_contents_g[j].empty()) continue;
        algebra::BoundingBox3D bb= bbs[i]+bbs[j];
        IMP_LOG(VERBOSE, "Building grids for " << i << " and " << j
                << " with bb " << bb << " and side "
                << tr.get_distance()+bin_ubs[i]+bin_ubs[j]
                << std::endl);
        Grid ggi, ggj;
        ggi= create_grid(bb, tr.get_distance()+bin_ubs[i]+bin_ubs[j]);
        ggj=ggi;
        fill_grid(bin_contents_g[i], tr, ggi);
        fill_grid(bin_contents_g[j], tr, ggj);
        for (typename Grid::AllConstIterator it
               = ggj.all_begin();
             it != ggj.all_end(); ++it) {
          if (!do_fill_close_pairs(ggi, it->first, it->second,
                                   false, tr, out)) {
            return false;
          }
        }

        IMP_IF_CHECK(USAGE_AND_INTERNAL) {
          for (unsigned int k=0; k< bin_contents_g[i].size(); ++k) {
            for (unsigned int l=0; l< k; ++l) {
              if (tr.check_close(bin_contents_g[i][k],
                                    bin_contents_g[i][l])) {
                out.check_contains(bin_contents_g[i][k],
                                   bin_contents_g[i][l]);
              }
            }
          }
        }
      }
    }
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      for (It it0= ps.b_; it0 != ps.e_; ++it0) {
        for (It it1= ps.b_; it1 != it0; ++it1) {
          if (tr.check_close(tr.get_id(*it0, 0), tr.get_id(*it1, 1))) {
            out.check_contains(tr.get_id(*it0, 0),
                               tr.get_id(*it1, 1));
          }
        }
      }
    }
    return true;
  }






  template <class ItG, class ItQ, class Out>
  static bool fill_close_pairs(const ParticleSet<ItG> &psg,
                               const ParticleSet<ItQ> &psq,
                               const Traits &tr,
                               Out out) {
    double maxr=std::max(get_max_radius(psg, tr), get_max_radius(psq, tr));
    std::vector<IDs > bin_contents_g, bin_contents_q;
    std::vector<double> bin_ubs;
    bin_ubs.push_back(maxr);

    partition_points(psg, tr, bin_contents_g, bin_ubs);
    partition_points(psq, tr, bin_contents_q, bin_ubs);
    IMP_LOG(VERBOSE, "Divided points into " << bin_contents_g.size()
            << " and " << bin_contents_q.size() << " bins ("
            << bin_ubs.size() << ") " << maxr << std::endl);
    IMP_IF_LOG(VERBOSE) {
      IMP_LOG(VERBOSE, "For G, contents are " << std::endl);
      for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
        IMP_LOG(VERBOSE, i << " " << bin_ubs[i] << " "
                << bin_contents_g[i].size()
                << std::endl);
      }
      IMP_LOG(VERBOSE, "For Q, contents are " << std::endl);
      for (unsigned int i=0; i< bin_contents_q.size(); ++i) {
        IMP_LOG(VERBOSE, i << " " << bin_ubs[i] << " "
                << bin_contents_q[i].size()
                << std::endl);
      }
    }
    std::vector<algebra::BoundingBox3D> bbs_g(bin_contents_g.size());
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      bbs_g[i]= get_bb(bin_contents_g[i], tr);
    }
    std::vector<algebra::BoundingBox3D> bbs_q(bin_contents_q.size());
    for (unsigned int i=0; i< bin_contents_q.size(); ++i) {
      bbs_q[i]= get_bb(bin_contents_q[i], tr);
    }
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      if (bin_contents_g[i].empty()) continue;
      for (unsigned int j=0; j< bin_contents_q.size(); ++j) {
        if (bin_contents_q[j].empty()) continue;
        algebra::BoundingBox3D bb= bbs_g[i]+bbs_q[j];
        if (bin_contents_g[i].size() < 10 && bin_contents_q[j].size() < 10) {
          if (!do_fill_close_pairs_from_lists(bin_contents_g[i].begin(),
                                              bin_contents_g[i].end(),
                                              bin_contents_q[j].begin(),
                                              bin_contents_q[j].end(),
                                              tr, out)) {
            return false;
          }
        } else {
          IMP_LOG(VERBOSE, "Building grids for " << i << " and " << j
                  << " with bb " << bb << " and side "
                  << tr.get_distance()+bin_ubs[i]+bin_ubs[j]
                  << std::endl);
          Grid gg, gq;
          gg= create_grid(bb, tr.get_distance()+bin_ubs[i]+bin_ubs[j]);
          gq= gg;
          fill_grid(bin_contents_g[i], tr, gg);
          fill_grid(bin_contents_q[j], tr, gq);
          IMP_IF_CHECK(USAGE) {
            for (unsigned int i=0; i< 3; ++i) {
              IMP_USAGE_CHECK(gg.get_number_of_voxels(i)
                              == gq.get_number_of_voxels(i),
                              "Do not match on dimension i "
                              << gg.get_number_of_voxels(i) << " vs "
                              << gq.get_number_of_voxels(i));
            }
          }
          for (typename Grid::AllConstIterator it
                 = gq.all_begin();
               it != gq.all_end(); ++it) {
            if (!do_fill_close_pairs(gg, it->first, it->second,
                                     false, tr, out)) {
              return false;
            }
          }
        }

        IMP_IF_CHECK(USAGE_AND_INTERNAL) {
          for (unsigned int k=0; k< bin_contents_g[i].size(); ++k) {
            for (unsigned int l=0; l< bin_contents_q[j].size(); ++l) {
              if (tr.check_close(bin_contents_g[i][k],
                                    bin_contents_q[j][l])) {
                out.check_contains(bin_contents_g[i][k],
                                   bin_contents_q[j][l]);
              }
            }
          }
        }
      }
    }
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
       for (ItG it0= psg.b_; it0 != psg.e_; ++it0) {
         for (ItQ it1= psq.b_; it1 != psq.e_; ++it1) {
           if (tr.check_close(tr.get_id(*it0, 0), tr.get_id(*it1, 1))) {
             out.check_contains(tr.get_id(*it0, 0),
                                tr.get_id(*it1, 1));
          }
        }
      }
    }
    return true;
  }
};

typedef Helper<ParticleTraits> ParticleHelper;
typedef Helper<BoundingBoxTraits> BBHelper;

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_GRID_CLOSE_PAIRS_IMPL_H */
