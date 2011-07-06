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

inline bool get_is_wrapped(unsigned int merged, unsigned int k) {
  IMP_USAGE_CHECK(k<=2, "bad k");
  return merged& (1 << k);
}
struct ParticleID {
  typedef Particle* result_type;
  Particle* operator()(Particle *p) const {return p;}
};
struct ParticleCenter {
  algebra::Vector3D operator()(Particle *p) const {
    return XYZ(p).get_coordinates();
  }
};
struct ParticleRadius {
  double operator()(Particle *p) const {
    return XYZR(p).get_radius();
  }
};
struct ParticleClose {
  double d_;
  ParticleClose(double d): d_(d){}
  bool operator()(Particle *a, Particle *b) const {
    return get_interiors_intersect(XYZR(a).get_coordinates()
                                   -XYZR(b).get_coordinates(),
                                   XYZR(b).get_radius()+d_,
                                   XYZR(a).get_radius());
  }
};
struct ParticlePairSink {
  ParticlePairsTemp &out_;
  ParticlePairSink(ParticlePairsTemp &out): out_(out){}
  typedef ParticlePair argument_type;
  bool operator()(const ParticlePair &c) {
    out_.push_back(c);
    return true;
  }
};
struct ParticleSink {
  ParticlesTemp &out_;
  ParticleSink(ParticlesTemp &out): out_(out){}
  typedef Particle* argument_type;
  bool operator()( Particle* c) {
    out_.push_back(c);
    return true;
  }
};
struct PeriodicParticleClose {
  algebra::Vector3D uc_;
  unsigned int merged_;
  double d_;
  PeriodicParticleClose(const algebra::BoundingBox3D &bb,
                        unsigned int merged,
                        double d):
    uc_(bb.get_corner(1)- bb.get_corner(0)),
    merged_(merged), d_(d){}
  bool operator()(Particle *a, Particle *b) const {
    algebra::Vector3D diff
      = XYZR(a).get_coordinates()- XYZR(b).get_coordinates();
    for (unsigned int i=0; i< 3; ++i) {
      if (get_is_wrapped(merged_, i)) {
        if (diff[i] > .5*uc_[i]) diff[i]-=uc_[i];
        else if (diff[i] < -.5*uc_[i]) diff[i]+=uc_[i];
      }
    }
    return get_interiors_intersect(diff, XYZR(a).get_radius(),
                                   XYZR(b).get_radius()+d_);
  }
};
struct BBID {
  algebra::BoundingBox3Ds::const_iterator it_;
  BBID(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
  typedef unsigned int result_type;
  unsigned int operator()(const algebra::BoundingBox3D &bb) const {
    return &bb-&*it_;
  }
};
struct BBRadius {
  algebra::BoundingBox3Ds::const_iterator it_;
  BBRadius(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
  double operator()(unsigned int i) const {
    return algebra::get_maximum_length(*(it_+i));
  }
};
struct BBCenter {
  algebra::BoundingBox3Ds::const_iterator it_;
  BBCenter(algebra::BoundingBox3Ds::const_iterator it): it_(it){}
  algebra::Vector3D operator()(unsigned int i) const {
    return algebra::Vector3D(.5*((it_+i)->get_corner(0)
                                 +(it_+i)->get_corner(1)));
  }
};
struct PeriodicBBClose {
  double d_;
  algebra::Vector3D uc_;
  unsigned int merged_;
  algebra::BoundingBox3Ds::const_iterator it0_, it1_;
  PeriodicBBClose(algebra::BoundingBox3Ds::const_iterator it0,
                  algebra::BoundingBox3Ds::const_iterator it1,
                  const algebra::BoundingBox3D &bb,
                  unsigned int merged, double d): d_(d),
                                                  uc_(bb.get_corner(1)
                                                      - bb.get_corner(0)),
                                                  merged_(merged), it0_(it0),
                                                  it1_(it1){}
  double operator()(unsigned int a, unsigned int b) const {
    for (unsigned int i=0; i< 3; ++i) {
      if (get_is_wrapped(merged_, i)) {
        bool ok=false;
        for (int o=-1; o < 2; ++o) {
          double ub0= uc_[i]*o + (it1_+b)->get_corner(1)[i]+d_;
          if ((it0_+a)->get_corner(0)[i] > ub0) {
            continue;
          }
          double ub1= uc_[i]*o + (it0_+1)->get_corner(1)[i]+d_;
          if ((it1_+b)->get_corner(0)[i] > ub1) {
            continue;
          }
          ok=true;
          break;
        }
        if (!ok) return false;
      } else {
        if ((it0_+a)->get_corner(0)[i]
            > (it1_+b)->get_corner(1)[i]+d_) return false;
        if ((it0_+a)->get_corner(1)[i]+d_
            < (it1_+b)->get_corner(0)[i]) return false;
      }
    }
    return true;
  }
};
struct BBClose {
  double d_;
  algebra::BoundingBox3Ds::const_iterator it0_, it1_;
  BBClose(algebra::BoundingBox3Ds::const_iterator it0,
          algebra::BoundingBox3Ds::const_iterator it1, double d):
    d_(d), it0_(it0), it1_(it1){}
  double operator()(unsigned int a, unsigned int b) const {
    algebra::BoundingBox3D ag= *(it0_+a)+d_;
    return algebra::get_interiors_intersect(ag, *(it1_+b));
  }
};

struct BBPairSink {
  IntPairs &out_;
  BBPairSink(IntPairs &out): out_(out){}
  typedef IntPair argument_type;
  bool operator()(const IntPair &ip) const {
    out_.push_back(ip);
    return true;
  }
};
inline std::string do_show(Particle*p) {
  return p->get_name();
}
inline std::string do_show(const ParticlesTemp&p) {
  std::ostringstream oss;
  for (unsigned int i=0; i< p.size(); ++i) {
    oss << do_show(p[i]) << " ";
  }
  return oss.str();
}
/*algebra::BoundingBox3D do_show(algebra::BoundingBox3D bb) {
  return bb;
  }*/
inline unsigned int do_show(unsigned int i) {
  return i;
}
inline std::string do_show(const std::vector<unsigned int>&p) {
  std::ostringstream oss;
  for (unsigned int i=0; i< p.size(); ++i) {
    oss << do_show(p[i]) << " ";
  }
  return oss.str();
}

template <class IDF, class CenterF,
          class RadiusF, class CloseF, class Out>
struct Helper {
  typedef typename IDF::result_type ID;
  typedef std::vector<ID> IDs;
  typedef typename algebra::SparseGrid3D<IDs> Grid;
  typedef std::vector<Grid> Grids;

  template <class It>
  struct ParticleSet {
    It b_, e_;
    IDF id_;
    CenterF c_;
    RadiusF r_;
    ParticleSet(It b, It e, IDF id,
                CenterF c, RadiusF r): b_(b), e_(e),
                                       id_(id), c_(c), r_(r){}
  };
  template <class It>
  static ParticleSet<It> get_particle_set(It b, It e,
                                          IDF id, CenterF c, RadiusF r) {
    return ParticleSet<It>(b,e,id, c,r);
  }
  template <class It>
  struct IDSet {
    It b_, e_;
    CenterF c_;
    RadiusF r_;
    IDSet(It b, It e, CenterF c, RadiusF r): b_(b), e_(e),
                                             c_(c), r_(r){}
  };
  template <class It>
  static IDSet<It> get_id_set(It b, It e,
                              CenterF c, RadiusF r) {
    return IDSet<It>(b,e,c,r);
  }


  static int get_index(int ii, int which, int lb, int ub) {
    if (which==1) {
      if (ii== lb) ii= ub;
      else if (ii == ub) ii= lb;
    }
    return ii;
  }

  template <class Index>
  static void fill_copies_periodic(const Grid &g,
                                   Index cur,
                                   unsigned int merged,
                                   typename Grid::ExtendedIndex bblb,
                                   typename Grid::ExtendedIndex bbub,
                                   bool half,
                                   std::vector<typename Grid::Index> &out) {
    typename Grid::ExtendedIndex curei(cur[0], cur[1], cur[2]);
    for (int io=-1; io < 2; ++io) {
      if ((!(merged &GridClosePairsFinder::X)) && io != 0) continue;
      int ii=cur[0]+io*(bbub[0]-bblb[0]-1);
      if (ii < 0
          || ii >= static_cast<int>(g.get_number_of_voxels(0))) continue;
      for (int jo=-1; jo < 2; ++jo) {
        if ((!(merged &GridClosePairsFinder::Y)) && jo != 0) continue;
        int ij=cur[1]+jo*(bbub[1]-bblb[1]-1);
        if (ij < 0
            || ij >= static_cast<int>(g.get_number_of_voxels(1))) continue;
        for (int ko=-1; ko < 2; ++ko) {
          if ((!(merged &GridClosePairsFinder::Z)) && ko != 0) continue;
          int ik=cur[2]+ko*(bbub[2]-bblb[2]-1);
          if (ik < 0
              || ik >= static_cast<int>(g.get_number_of_voxels(2))) continue;
          typename Grid::ExtendedIndex cei(ii, ij, ik);
          if (!g.get_has_index(cei)) continue;
          // make sure equivalent voxels are only added once
          if ((half && cei < curei) || (!half && curei != cei)) {
            out.push_back(g.get_index(cei));
          }
        }
      }
    }
  }

  static std::vector<typename Grid::Index> get_nearby(const Grid &g,
                                         typename Grid::ExtendedIndex center,
                                         typename Grid::ExtendedIndex bblb,
                                         typename Grid::ExtendedIndex bbub,
                                                      unsigned int merged,
                                                      bool half) {
    std::vector<typename Grid::Index> out;
    typename Grid::ExtendedIndex lb(center.get_offset(-1, -1, -1)),
      ub(center.get_offset(1, 1, 1));
    if (half) {
      IMP_GRID3D_FOREACH_SMALLER_EXTENDED_INDEX_RANGE(g, center, lb, ub,
                                                      {
                                            fill_copies_periodic(g, voxel_index,
                                                 merged, bblb, bbub, true, out);
                                 typename Grid::ExtendedIndex ei(voxel_index[0],
                                                                 voxel_index[1],
                                                                voxel_index[2]);
                                                     if (g.get_has_index(ei)) {
                                                out.push_back(g.get_index(ei));
                                                        }
                                                      });
      fill_copies_periodic(g, center,
                           merged, bblb, bbub, true, out);
    } else {
      for (typename Grid::ExtendedIndexIterator it
             = g.extended_indexes_begin(lb, ub);
           it != g.extended_indexes_end(lb, ub); ++it) {
        fill_copies_periodic(g, *it,merged, bblb, bbub, false, out);
        if (g.get_has_index(*it)) {
          out.push_back(g.get_index(*it));
        }
      }
      // skipped in there
      out.push_back(g.get_index(center));
    }
    // would be nice to not have duplicates
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
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
  static double get_max_radius(const ParticleSet<It> &ps0) {
    double maxr=0;
    for (It c= ps0.b_; c != ps0.e_; ++c) {
      maxr=std::max(ps0.r_(ps0.id_(*c)), maxr);
    }
    return maxr;
  }

  static algebra::BoundingBox3D get_bb(const IDs& ids, CenterF cc) {
    algebra::BoundingBox3D bb;
    for (typename IDs::const_iterator c= ids.begin(); c != ids.end(); ++c) {
      bb+=cc(*c);
    }
    return bb;
  }


  template <class It>
  static void partition_points(const ParticleSet<It> &ps,
                               double distance,
                               typename std::vector<IDs > &bin_contents,
                               typename std::vector<double> &bin_ubs) {
    bin_contents.push_back(IDs());
    for (It c= ps.b_; c != ps.e_; ++c) {
      double cr= ps.r_(ps.id_(*c))+0;
      while (cr < std::max(.5*bin_ubs.back()-.5*distance, 0.0)) {
        double v=std::max(.5*bin_ubs.back()-.5*distance, 0.1);
        if (v > .1) {
          bin_ubs.push_back(v);
        } else break;
      }
      for ( int i=bin_ubs.size()-1; i >=0; --i) {
        if (cr <= bin_ubs[i] || i==0) {
          while (static_cast<int>(bin_contents.size()) < i+1) {
            bin_contents.push_back(IDs());
          }
          bin_contents[i].push_back(ps.id_(*c));
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
                        CenterF cf,
                        Grid &g) {
    for (typename IDs::const_iterator c= ids.begin(); c != ids.end(); ++c) {
      algebra::Vector3D v= cf(*c);
      typename Grid::ExtendedIndex ind
        =g.get_nearest_extended_index(v);
      if (g.get_has_index(ind)) {
        g[g.get_index(ind)].push_back(*c);
      } else {
        g.add_voxel(ind, IDs(1, *c));
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








  template <class It>
  static bool do_fill_close_pairs_from_list(It b, It e,
                                            CloseF close,
                                            Out out) {
    for (It c= b; c != e; ++c) {
      for (It cp= b; cp != c; ++cp) {
        if (close(*c, *cp)) {
          IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                  << do_show(*cp) << std::endl);
          if (!out( typename Out::argument_type(*c, *cp))) {
            return false;
          }
        }
      }
    }
    return true;
  }






  template <class ItA, class ItB>
  static bool do_fill_close_pairs_from_lists(ItA ab, ItA ae,
                                             ItB bb, ItB be,
                                             CloseF close,
                                             Out out) {
    for (ItA c= ab; c != ae; ++c) {
      for (ItB cp= bb; cp != be; ++cp) {
        if (close(*c, *cp)) {
          IMP_LOG(VERBOSE, "Found pair " << do_show(*c) << " "
                  << do_show(*cp) << std::endl);
          if (!out( typename Out::argument_type(*c, *cp))) {
            return false;
          }
        }
      }
    }
    return true;
  }




  static bool do_fill_close_pairs(const Grid &gg,
                                  typename Grid::Index index,
                                  const IDs &qps,
                                  bool half, CloseF close, Out& out) {
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
                                          close, out)) {
        return false;
      }
    }
    if (half) {
      IMP_LOG(VERBOSE, "Checking pair " << index << " " << index
              << std::endl);
      if (!do_fill_close_pairs_from_list(gg[index].begin(),
                                         gg[index].end(),
                                         close, out)) {
        return false;
      }
    }
    return true;
  }

  static bool do_fill_close_pairs(const Grid &gg,
                                  typename Grid::Index index,
                                  const IDs &qps,
                                  const algebra::BoundingBox3D &bb,
                                  unsigned int merged,
                                  bool half, CloseF close,
                                  Out& out) {
    typename Grid::ExtendedIndex bblb
      = gg.get_extended_index(bb.get_corner(0));
    typename Grid::ExtendedIndex bbub
      = gg.get_extended_index(bb.get_corner(1));
    const std::vector<typename Grid::Index> ids
      = get_nearby(gg, gg.get_extended_index(index),
                   bblb, bbub, merged, half);
    for (unsigned int i=0; i< ids.size(); ++i) {
      IMP_LOG(VERBOSE, "Checking pair " << ids[i] << " " << index
              << ": " << do_show(gg[ids[i]])
              << " and " << do_show(gg[index])
              << " which is " << do_show(qps) << std::endl);
      if (!do_fill_close_pairs_from_lists(gg[ids[i]].begin(),
                                          gg[ids[i]].end(),
                                          qps.begin(), qps.end(),
                                          close, out)) {
        return false;
      }
    }
    if (half) {
      IMP_LOG(VERBOSE, "Checking pair " << index << " " << index
              << std::endl);
      if (!do_fill_close_pairs_from_list(gg[index].begin(),
                                         gg[index].end(),
                                         close, out)) {
        return false;
      }
    }
    return true;
  }


  template <class It>
  static bool fill_close_pairs(const ParticleSet<It> &ps,
                               CloseF close,
                               double distance,
                               const algebra::BoundingBox3D& bb,
                               unsigned int merged,
                               Out out) {
    double maxr=get_max_radius(ps);
    std::vector<IDs > bin_contents_g;
    std::vector<double> bin_ubs;
    bin_ubs.push_back(maxr);

    partition_points(ps, distance, bin_contents_g, bin_ubs);

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
      bbs[i]= get_bb(bin_contents_g[i], ps.c_);
    }
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      if (bin_contents_g[i].empty()) continue;
      if (bin_contents_g[i].size() < 5) {
        if (!do_fill_close_pairs_from_list(bin_contents_g[i].begin(),
                                           bin_contents_g[i].end(),
                                           close, out)) {
          return false;
        }
      } else {
        Grid gg = create_grid(bbs[i], std::max(distance+2*bin_ubs[i],
                                               .01*(bbs[i].get_corner(1)
                                                    -bbs[i].get_corner(0))
                                               .get_magnitude()));
        fill_grid(bin_contents_g[i], ps.c_, gg);
        for (typename Grid::AllConstIterator it
               = gg.all_begin();
             it != gg.all_end(); ++it) {
          if (merged) {
            if (!do_fill_close_pairs(gg, it->first, it->second, bb,
                                     merged, true, close, out)) {
              return false;
            }
          } else {
            if (!do_fill_close_pairs(gg, it->first, it->second,true,
                                     close, out)) {
              return false;
            }
          }
        }
      }
      for (unsigned int j=0; j< i; ++j) {
        if (bin_contents_g[j].empty()) continue;
        algebra::BoundingBox3D bb= bbs[i]+bbs[j];
        IMP_LOG(VERBOSE, "Building grids for " << i << " and " << j
                << " with bb " << bb << " and side "
                << distance+bin_ubs[i]+bin_ubs[j]
                << std::endl);
        Grid ggi, ggj;
        ggi= create_grid(bb, distance+bin_ubs[i]+bin_ubs[j]);
        ggj=ggi;
        fill_grid(bin_contents_g[i], ps.c_, ggi);
        fill_grid(bin_contents_g[j], ps.c_, ggj);
        for (typename Grid::AllConstIterator it
               = ggj.all_begin();
             it != ggj.all_end(); ++it) {
          if (merged) {
            if (!do_fill_close_pairs(ggi, it->first, it->second, bb,
                                     merged, false, close, out)) {
              return false;
            }
          } else {
            if (!do_fill_close_pairs(ggi, it->first, it->second,
                                     false, close, out)) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }






  template <class ItG, class ItQ>
  static bool fill_close_pairs(const ParticleSet<ItG> &psg,
                               const ParticleSet<ItQ> &psq,
                               CloseF close,
                               double distance,
                               const algebra::BoundingBox3D& ,
                               unsigned int merged,
                               Out out) {
    double maxr=std::max(get_max_radius(psg), get_max_radius(psq));
    std::vector<IDs > bin_contents_g, bin_contents_q;
    std::vector<double> bin_ubs;
    bin_ubs.push_back(maxr);

    partition_points(psg, distance, bin_contents_g, bin_ubs);
    partition_points(psq, distance, bin_contents_q, bin_ubs);
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
      bbs_g[i]= get_bb(bin_contents_g[i], psg.c_);
    }
    std::vector<algebra::BoundingBox3D> bbs_q(bin_contents_q.size());
    for (unsigned int i=0; i< bin_contents_q.size(); ++i) {
      bbs_q[i]= get_bb(bin_contents_q[i], psq.c_);
    }
    for (unsigned int i=0; i< bin_contents_g.size(); ++i) {
      if (bin_contents_g[i].empty()) continue;
      for (unsigned int j=0; j< bin_contents_q.size(); ++j) {
        if (bin_contents_q[j].empty()) continue;
        algebra::BoundingBox3D bb= bbs_g[i]+bbs_q[j];
        if (bin_contents_g[i].size() < 5 && bin_contents_q[j].size() < 5) {
          if (!do_fill_close_pairs_from_lists(bin_contents_g[i].begin(),
                                              bin_contents_g[i].end(),
                                              bin_contents_q[j].begin(),
                                              bin_contents_q[j].end(),
                                              close, out)) {
            return false;
          }
        } else {
          IMP_LOG(VERBOSE, "Building grids for " << i << " and " << j
                  << " with bb " << bb << " and side "
                  << distance+bin_ubs[i]+bin_ubs[j]
                  << std::endl);
          Grid gg, gq;
          gg= create_grid(bb, distance+bin_ubs[i]+bin_ubs[j]);
          gq= gg;
          fill_grid(bin_contents_g[i], psg.c_, gg);
          fill_grid(bin_contents_q[j], psq.c_, gq);
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
            if (merged) {
              if (!do_fill_close_pairs(gg, it->first, it->second, bb, merged,
                                       false, close, out)) {
                return false;
              }
            } else {
              if (!do_fill_close_pairs(gg, it->first, it->second,
                                       false, close, out)) {
                return false;
              }
            }
          }
        }
      }
    }
    return true;
  }
};

typedef Helper<ParticleID, ParticleCenter, ParticleRadius,
               ParticleClose, ParticlePairSink> ParticleHelper;
typedef Helper<BBID, BBCenter, BBRadius, BBClose, BBPairSink> BBHelper;
typedef Helper<ParticleID, ParticleCenter, ParticleRadius,
               PeriodicParticleClose, ParticlePairSink> PParticleHelper;
typedef Helper<BBID, BBCenter, BBRadius, PeriodicBBClose,
               BBPairSink> PBBHelper;

IMPCORE_END_INTERNAL_NAMESPACE

#endif /* IMPCORE_INTERNAL_GRID_CLOSE_PAIRS_IMPL_H */
